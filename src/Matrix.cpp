#include "Matrix.hpp"

Mat4x4f32 Mat4x4f32::Zeroes = Mat4x4f32{ std::array<float, 16u>{
    0.f, 0.f, 0.f, 0.f,
    0.f, 0.f, 0.f, 0.f,
    0.f, 0.f, 0.f, 0.f,
    0.f, 0.f, 0.f, 0.f,
} };

Mat4x4f32 Mat4x4f32::Identity = Mat4x4f32{ std::array<float, 16u>{
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f,
} };