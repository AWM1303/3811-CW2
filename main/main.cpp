#include <glad.h>
#include <GLFW/glfw3.h>

#include <typeinfo>
#include <stdexcept>

#include <cstdio>
#include <cstdlib>

#include "../support/error.hpp"
#include "../support/program.hpp"
#include "../support/checkpoint.hpp"
#include "../support/debug_output.hpp"

#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"

#include "defaults.hpp"
#include "loadobj.hpp"

#define TO_RADIANS 3.14/180.0



namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - CW2";

	constexpr float kPi_ = 3.1415926f;

	constexpr float kMovementPerSecond_ = 5.f; // units per second
	constexpr float kMouseSensitivity_ = 0.01f; // radians per pixel

	struct State_
	{
		ShaderProgram* prog;

		struct CamCtrl_
		{
			bool cameraActive;
			bool actionZoomIn, actionZoomOut;

			float phi, theta;
			float radius;

			float lastX, lastY;

			// Camera position
			float camX, camY, camZ;

			struct Motion
			{
				bool Forward, Backward, Left, Right;
			} motion;

			// Constructor
			CamCtrl_() : cameraActive(false), actionZoomIn(false), actionZoomOut(false),
				phi(0.0f), theta(0.0f), radius(10.0f), lastX(0.0f), lastY(0.0f),
				camX(0.0f), camY(0.0f), camZ(0.0f) {}

			// Function to handle camera movement
			void moveCamera()
			{
				if (actionZoomIn)
					radius -= 0.1f;
				if (actionZoomOut)
					radius += 0.1f;

				// Limit the radius to avoid going too close or too far
				radius = std::max(0.1f, std::min(radius, 50.0f));

				// Update camera position based on keyboard input
				if (motion.Forward)
				{
					camX += cos((theta + 90) * TO_RADIANS) / 5.0;
					camZ -= sin((theta + 90) * TO_RADIANS) / 5.0;
				}
				if (motion.Backward)
				{
					camX += cos((theta + 90 + 180) * TO_RADIANS) / 5.0;
					camZ -= sin((theta + 90 + 180) * TO_RADIANS) / 5.0;
				}
				if (motion.Left)
				{
					camX += cos((theta + 90 + 90) * TO_RADIANS) / 5.0;
					camZ -= sin((theta + 90 + 90) * TO_RADIANS) / 5.0;
				}
				if (motion.Right)
				{
					camX += cos((theta + 90 - 90) * TO_RADIANS) / 5.0;
					camZ -= sin((theta + 90 - 90) * TO_RADIANS) / 5.0;
				}

				// Limit the values of pitch between -60 and 70
				if (phi >= 70)
					phi = 70;
				if (phi <= -60)
					phi = -60;
			}

			void keyboard_up(unsigned char key, int x, int y)
			{
				switch (key)
				{
				case 'W':
				case 'w':
					motion.Forward = false;
					break;
				case 'A':
				case 'a':
					motion.Left = false;
					break;
				case 'S':
				case 's':
					motion.Backward = false;
					break;
				case 'D':
				case 'd':
					motion.Right = false;
					break;
				case 'E':
				case 'e':
					// Handle upward movement (E key)
					camY -= 0.1f;
					break;
				case 'Q':
				case 'q':
					// Handle downward movement (Q key)
					camY += 0.1f;
					break;
				}
			}


			void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				auto* camCtrl = static_cast<CamCtrl_*>(glfwGetWindowUserPointer(window));

				if (action == GLFW_PRESS)
				{
					switch (key)
					{
					case GLFW_KEY_ESCAPE:
						glfwSetWindowShouldClose(window, GLFW_TRUE);
						break;
					case GLFW_KEY_LEFT_SHIFT:
						camCtrl->actionZoomIn = true;
						break;
					case GLFW_KEY_LEFT_CONTROL:
						camCtrl->actionZoomOut = true;
						break;
						// Add more cases for other keys if needed
					}
				}
				else if (action == GLFW_RELEASE)
				{
					switch (key)
					{
					case GLFW_KEY_LEFT_SHIFT:
						camCtrl->actionZoomIn = false;
						break;
					case GLFW_KEY_LEFT_CONTROL:
						camCtrl->actionZoomOut = false;
						break;
						// Add more cases for other keys if needed
					}
				}
			}
		} camControl;

	};
	
	void glfw_callback_error_( int, char const* );

	void glfw_callback_key_( GLFWwindow*, int, int, int, int );

	struct GLFWCleanupHelper
	{
		~GLFWCleanupHelper();
	};
	struct GLFWWindowDeleter
	{
		~GLFWWindowDeleter();
		GLFWwindow* window;
	};
}

int main() try
{
	// Initialize GLFW
	if( GLFW_TRUE != glfwInit() )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwInit() failed with '%s' (%d)", msg, ecode );
	}

	// Ensure that we call glfwTerminate() at the end of the program.
	GLFWCleanupHelper cleanupHelper;

	// Configure GLFW and create window
	glfwSetErrorCallback( &glfw_callback_error_ );

	glfwWindowHint( GLFW_SRGB_CAPABLE, GLFW_TRUE );
	glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );

	//glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	glfwWindowHint( GLFW_DEPTH_BITS, 24 );

#	if !defined(NDEBUG)
	// When building in debug mode, request an OpenGL debug context. This
	// enables additional debugging features. However, this can carry extra
	// overheads. We therefore do not do this for release builds.
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
#	endif // ~ !NDEBUG

	GLFWwindow* window = glfwCreateWindow(
		1280,
		720,
		kWindowTitle,
		nullptr, nullptr
	);

	if( !window )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwCreateWindow() failed with '%s' (%d)", msg, ecode );
	}

	GLFWWindowDeleter windowDeleter{ window };

	// Set up event handling
	// TODO: Additional event handling setup
	State_ state{};
	glfwSetWindowUserPointer(window, &state);

	// Capture camControl by reference in the lambda capture list
	glfwSetKeyCallback(window, &glfw_callback_key_);




	// Set up drawing stuff
	glfwMakeContextCurrent( window );
	glfwSwapInterval( 1 ); // V-Sync is on.

	// Initialize GLAD
	// This will load the OpenGL API. We mustn't make any OpenGL calls before this!
	if( !gladLoadGLLoader( (GLADloadproc)&glfwGetProcAddress ) )
		throw Error( "gladLoaDGLLoader() failed - cannot load GL API!" );

	std::printf( "RENDERER %s\n", glGetString( GL_RENDERER ) );
	std::printf( "VENDOR %s\n", glGetString( GL_VENDOR ) );
	std::printf( "VERSION %s\n", glGetString( GL_VERSION ) );
	std::printf( "SHADING_LANGUAGE_VERSION %s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );

	// Ddebug output
#	if !defined(NDEBUG)
	setup_gl_debug_output();
#	endif // ~ !NDEBUG

	// Global GL state
	OGL_CHECKPOINT_ALWAYS();

	// TODO: global GL setup goes here

	OGL_CHECKPOINT_ALWAYS();

	// Get actual framebuffer size.
	// This can be different from the window size, as standard window
	// decorations (title bar, borders, ...) may be included in the window size
	// but not be part of the drawable surface area.
	int iwidth, iheight;
	glfwGetFramebufferSize( window, &iwidth, &iheight );

	glViewport( 0, 0, iwidth, iheight );

	// Load shader program
	ShaderProgram prog({
		{ GL_VERTEX_SHADER, "assets/default.vert" },
		{ GL_FRAGMENT_SHADER, "assets/default.frag" }
		});

	state.prog = &prog;
	state.camControl.radius = 10.f;

	// Animation state
	auto last = Clock::now();

	float angle = 0.f;

	// Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();
	
	// TODO: global GL setup goes here

	OGL_CHECKPOINT_ALWAYS();
	SimpleMeshData parlahtiMesh = load_wavefront_obj("assets/parlahti.obj");
	GLuint vaoparlahti = create_vao(parlahtiMesh);
	std::size_t parlahtiVertexCount = parlahtiMesh.positions.size();

	// Main loop
	while( !glfwWindowShouldClose( window ) )
	{
		// Let GLFW process events
		glfwPollEvents();
		
		// Check if window was resized.
		float fbwidth, fbheight;
		{
			int nwidth, nheight;
			glfwGetFramebufferSize( window, &nwidth, &nheight );

			fbwidth = float(nwidth);
			fbheight = float(nheight);

			if( 0 == nwidth || 0 == nheight )
			{
				// Window minimized? Pause until it is unminimized.
				// This is a bit of a hack.
				do
				{
					glfwWaitEvents();
					glfwGetFramebufferSize( window, &nwidth, &nheight );
				} while( 0 == nwidth || 0 == nheight );
			}

			glViewport( 0, 0, nwidth, nheight );
		}

		// Update state
		//TODO: update state
		auto const now = Clock::now();
		float dt = std::chrono::duration_cast<Secondsf>(now - last).count();
		last = now;

		//To rotate or not
		/*angle += dt * kPi_ * 0.3f;
		if (angle >= 2.f * kPi_)
			angle -= 2.f * kPi_;*/

		// Update camera state
		if (state.camControl.actionZoomIn)
			state.camControl.radius -= kMovementPerSecond_ * dt;
		else if (state.camControl.actionZoomOut)
			state.camControl.radius += kMovementPerSecond_ * dt;

		if (state.camControl.radius <= 0.1f)
			state.camControl.radius = 0.1f;

		//camControl.moveCamera();
		// Update: compute matrices
		//TODO: define and compute projCameraWorld matrix

		// Draw scene
		OGL_CHECKPOINT_DEBUG();
		Mat44f model2world = make_rotation_y(angle);
		//Mat44f world2camera = make_translation({ 0.f, 0.f, -10.f });
		Mat44f projection = make_perspective_projection(
			60.f * 3.1415926f / 180.f,
			fbwidth / float(fbheight),
			0.1f, 100.0f
		);
		//Mat44f projCameraWorld = projection * world2camera * model2world;

		Mat44f model2world_second = make_rotation_y(angle) * make_translation({ 3.f, 0.f, 0.f });
		//Mat44f projCameraWorld_second = projection * world2camera * model2world_second;

		Mat44f Rx = make_rotation_x(state.camControl.theta);
		Mat44f Ry = make_rotation_y(state.camControl.phi);
		Mat44f T = make_translation({ 0.f, 0.f, -state.camControl.radius });

		Mat44f world2camera = Rx * Ry * T;
		Mat44f projCameraWorld = projection * world2camera * model2world;
		Mat44f projCameraWorld_second = projection * world2camera * model2world_second;

		// Draw scene
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// We want to draw with our program..
		glUseProgram(prog.programId());
		glBindVertexArray(vaoparlahti);
		glUniformMatrix4fv(0, 1, GL_TRUE, projCameraWorld.v);
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(parlahtiVertexCount));

		//TODO: draw frame

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers( window );
	}

	// Cleanup.
	state.prog = nullptr;
	//TODO: additional cleanup
	
	return 0;
}
catch( std::exception const& eErr )
{
	std::fprintf( stderr, "Top-level Exception (%s):\n", typeid(eErr).name() );
	std::fprintf( stderr, "%s\n", eErr.what() );
	std::fprintf( stderr, "Bye.\n" );
	return 1;
}


namespace
{
	void glfw_callback_error_(int aErrNum, char const* aErrDesc)
	{
		std::fprintf(stderr, "GLFW error: %s (%d)\n", aErrDesc, aErrNum);
	}

	void glfw_callback_key_(GLFWwindow* aWindow, int aKey, int, int aAction, int)
	{
		if (GLFW_KEY_ESCAPE == aKey && GLFW_PRESS == aAction)
		{
			glfwSetWindowShouldClose(aWindow, GLFW_TRUE);
			return;
		}

		if (auto* state = static_cast<State_*>(glfwGetWindowUserPointer(aWindow)))
		{
			// R-key reloads shaders.
			if (GLFW_KEY_R == aKey && GLFW_PRESS == aAction)
			{
				if (state->prog)
				{
					try
					{
						state->prog->reload();
						std::fprintf(stderr, "Shaders reloaded and recompiled.\n");
					}
					catch (std::exception const& eErr)
					{
						std::fprintf(stderr, "Error when reloading shader:\n");
						std::fprintf(stderr, "%s\n", eErr.what());
						std::fprintf(stderr, "Keeping old shader.\n");
					}
				}
			}

			// Space toggles camera
			if (GLFW_KEY_SPACE == aKey && GLFW_PRESS == aAction)
			{
				state->camControl.cameraActive = !state->camControl.cameraActive;

				if (state->camControl.cameraActive)
					glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				else
					glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}

			// Camera controls if camera is active
			if (state->camControl.cameraActive)
			{
				// Separate handling for the space key
				if (GLFW_KEY_W == aKey && GLFW_PRESS == aAction)
				{
					state->camControl.actionZoomIn = true;
				}
				else if (GLFW_KEY_W == aKey && GLFW_RELEASE == aAction)
				{
					state->camControl.actionZoomIn = false;
				}
				else if (GLFW_KEY_S == aKey && GLFW_PRESS == aAction)
				{
					state->camControl.actionZoomOut = true;
				}
				else if (GLFW_KEY_S == aKey && GLFW_RELEASE == aAction)
				{
					state->camControl.actionZoomOut = false;
				}
			}
		}
	}

	void glfw_callback_motion_(GLFWwindow* aWindow, double aX, double aY)
	{
		if (auto* state = static_cast<State_*>(glfwGetWindowUserPointer(aWindow)))
		{
			if (state->camControl.cameraActive)
			{
				auto const dx = float(aX - state->camControl.lastX);
				auto const dy = float(aY - state->camControl.lastY);

				state->camControl.phi += dx * kMouseSensitivity_;

				state->camControl.theta += dy * kMouseSensitivity_;
				if (state->camControl.theta > kPi_ / 2.f)
					state->camControl.theta = kPi_ / 2.f;
				else if (state->camControl.theta < -kPi_ / 2.f)
					state->camControl.theta = -kPi_ / 2.f;
			}

			state->camControl.lastX = float(aX);
			state->camControl.lastY = float(aY);
		}
	}
}

namespace
{
	GLFWCleanupHelper::~GLFWCleanupHelper()
	{
		glfwTerminate();
	}

	GLFWWindowDeleter::~GLFWWindowDeleter()
	{
		if( window )
			glfwDestroyWindow( window );
	}
}

