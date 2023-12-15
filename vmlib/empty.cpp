// This file is intentially left empty. 
//
// Visual Studio does not like static libraries without any sources. This
// file ensures that the is at least one source file in the Visual Studio
// project. This keeps VS happy.
#include <iostream>
#include "../vmlib/mat44.hpp"  // Make sure the correct path is included


void testMatrixVectorMultiplication()
{
    // Test matrix
    Mat44f matrix = { {
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    } };

    // Test vector
    Vec4f vector = { 2.0f, 1.0f, 3.0f, 4.0f };

    // Multiply matrix and vector
    Vec4f result = matrix * vector;

    // Print the result vector
    std::cout << "Result Vector:" << std::endl;
    for (std::size_t i = 0; i < 4; ++i)
    {
        std::cout << result[i] << "\t";
    }
    std::cout << std::endl;
}

void TestMatrixMultiplication() {
    // Test matrix A
    Mat44f matrixA = { {
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    } };

    // Test matrix B
    Mat44f matrixB = { {
        2.0f, 0.0f, 1.0f, 3.0f,
        1.0f, 2.0f, 0.0f, 4.0f,
        3.0f, 1.0f, 2.0f, 5.0f,
        0.0f, 4.0f, 3.0f, 1.0f
    } };

    // Multiply matrices A and B
    Mat44f result = matrixA * matrixB;

    // Print the result matrix
    std::cout << "Result Matrix:" << std::endl;
    for (std::size_t i = 0; i < 4; ++i)
    {
        for (std::size_t j = 0; j < 4; ++j)
        {
            std::cout << result(i, j) << "\t";
        }
        std::cout << std::endl;
    }
}

void testRotateX() {
    Mat44f matrixA = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, -1.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
    };

    // Rotation angle in radians
    float angle = 0.5f;

    Mat44f rotatedMatrix = make_rotation_x(angle) * matrixA;

    // Print the rotated matrix
    std::cout << "Original Matrix A:" << std::endl;
    for (std::size_t i = 0; i < 4; ++i)
    {
        for (std::size_t j = 0; j < 4; ++j)
        {
            std::cout << matrixA(i, j) << "\t";
        }
        std::cout << std::endl;
    }

    std::cout << "\nRotated Matrix A:" << std::endl;
    for (std::size_t i = 0; i < 4; ++i)
    {
        for (std::size_t j = 0; j < 4; ++j)
        {
            std::cout << rotatedMatrix(i, j) << "\t";
        }
        std::cout << std::endl;
    }
}

void test_translation() {
    Vec3f translation = { 1.0f, 2.0f, 3.0f };

    // Create a translation matrix
    Mat44f translationMatrix = make_translation(translation);

    // Test point
    Vec4f point = { 1.0f, 1.0f, 1.0f, 1.0f };

    // Apply translation to the point
    Vec4f translatedPoint = translationMatrix * point;

    // Print the result
    std::cout << "Original Point: " << point.x << ", " << point.y << ", " << point.z << ", " << point.w << "\n";
    std::cout << "Translated Point: " << translatedPoint.x << ", " << translatedPoint.y << ", " << translatedPoint.z << ", " << translatedPoint.w << "\n";
}

void test_projection() {
    Mat44f projectionMatrix = make_perspective_projection(45.0f * (3.14159f / 180.0f), 16.0f / 9.0f, 0.1f, 100.0f);

    // Print the result
    //std::cout << "Perspective Projection Matrix:\n" << projectionMatrix << "\n";
}

int main()
{
    test_translation();

    return 0;
}

