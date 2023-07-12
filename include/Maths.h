#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>

#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)

template <class T>
struct Vector2;

using Vector2F = Vector2<float>;
using Vector2I = Vector2<int>;

struct Matrix_2_3;

namespace MathUtility
{
    constexpr double epsilon = 0.000001;

    // the ratio of the circumference to the radius of a circle, which is equal to 2π
    constexpr float TAU = 2.0f * M_PI;

    inline float DegreesToRadians(float angle)
    {
        return angle * M_PI / 180;
    }
};

template <class T>
struct Vector2
{
    T X, Y;

    constexpr Vector2() = default;

    template <class X, class Y>
    constexpr Vector2(X x, Y y)
    {
        this->X = x;
        this->Y = y;
    }

    template <class K>
    constexpr Vector2(const Vector2<K> &vector)
        : X(static_cast<T>(vector.X)), Y(static_cast<T>(vector.Y)) {}

    static const Vector2 Zero;
    static const Vector2 One;
    static const Vector2 Up;
    static const Vector2 Right;
    static const Vector2 Left;
    static const Vector2 Down;

    [[nodiscard]] float Length() const
    {
        return sqrt(X * X + Y * Y);
    }

    Vector2<T> Normalized() const
    {
        Vector2<T> normalized;

        normalized = (*this) / this->Length();

        return normalized;
    }

#pragma region Operator Overloads

    constexpr Vector2<T> operator+(const Vector2<T> &v) const
    {
        return Vector2<T>(this->X + v.X, this->Y + v.Y);
    }

    constexpr Vector2<T> operator-(const Vector2<T> &v) const
    {
        return Vector2<T>(this->X - v.X, this->Y - v.Y);
    }

    constexpr Vector2<T> operator*(float scale) const
    {
        return Vector2<T>(this->X * scale, this->Y * scale);
    }

    constexpr Vector2<T> operator*(Vector2<T> v) const
    {
        return Vector2<T>(this->X * v.X, this->Y * v.Y);
    }

    constexpr Vector2<T> operator/(float scale) const
    {
        return Vector2<T>(this->X / scale, this->Y / scale);
    }

    constexpr Vector2<T> &operator+=(const Vector2<T> &v)
    {
        this->X += v.X;
        this->Y += v.Y;
        return *this;
    }

    constexpr Vector2<T> &operator-=(const Vector2<T> &v)
    {
        this->X -= v.X;
        this->Y -= v.Y;
        return *this;
    }

    constexpr Vector2<T> &operator*=(const Vector2<T> &v)
    {
        this->X *= v.X;
        this->Y *= v.Y;
        return *this;
    }

    constexpr bool operator==(const Vector2<T> &v) const
    {
        return abs(X - v.X) < MathUtility::epsilon && abs(Y - v.Y) < MathUtility::epsilon;
    }

    constexpr bool operator!=(const Vector2<T> &v) const
    {
        return *this != v;
    }

#pragma endregion Operator Overloads
};

template <class T>
const Vector2<T> Vector2<T>::Zero = Vector2<T>(0, 0);
template <class T>
const Vector2<T> Vector2<T>::One = Vector2<T>(1, 1);
template <class T>
const Vector2<T> Vector2<T>::Up = Vector2<T>(0, 1);
template <class T>
const Vector2<T> Vector2<T>::Right = Vector2<T>(1, 0);
template <class T>
const Vector2<T> Vector2<T>::Left = Vector2<T>(-1, 0);
template <class T>
const Vector2<T> Vector2<T>::Down = Vector2<T>(0, -1);

struct Matrix_2_3
{
    float values[2][3];

    static Matrix_2_3 IdentityMatrix()
    {
        Matrix_2_3 mat =
            {
                .values =
                    {
                        1.f, 0.f, 0.f,
                        0.f, 1.f, 0.f}};

        return mat;
    }

    static Matrix_2_3 TranslationMatrix(Vector2F translation)
    {
        Matrix_2_3 mat =
            {
                .values =
                    {
                        1.f, 0.f, translation.X,
                        0.f, 1.f, translation.Y}};

        return mat;
    }

    static Matrix_2_3 RotationMatrix(float angle, bool clockWise = true)
    {
        Matrix_2_3 mat =
            {
                .values =
                    {
                        {cosf(MathUtility::DegreesToRadians(angle)), clockWise ? sinf(MathUtility::DegreesToRadians(angle)) : -sinf(MathUtility::DegreesToRadians(angle)), 0},
                        {clockWise ? -sinf(MathUtility::DegreesToRadians(angle)) : sinf(MathUtility::DegreesToRadians(angle)), cosf(MathUtility::DegreesToRadians(angle)), 0}}};

        return mat;
    }

    static Matrix_2_3 ScaleMatrix(Vector2F scaleRatio)
    {
        Matrix_2_3 mat =
            {
                .values =
                    {
                        scaleRatio.X, 0.f, 0.f,
                        0.f, scaleRatio.Y, 0.f}};

        return mat;
    }

    template <class T>
    static Vector2<T> Multiply(Matrix_2_3 mat, Vector2<T> vec)
    {
        Vector2F result =
            {
                vec.X * mat.values[0][0] + vec.Y * mat.values[0][1] + mat.values[0][2],
                vec.X * mat.values[1][0] + vec.Y * mat.values[1][1] + mat.values[1][2]};

        return result;
    }

    static Matrix_2_3 Multiply(Matrix_2_3 matA, Matrix_2_3 matB)
    {
        Matrix_2_3 result =
            {
                .values =
                    {
                        {matA.values[0][0] * matB.values[0][0] + matA.values[0][1] * matB.values[1][0] + matA.values[0][2] * 0,
                         matA.values[0][0] * matB.values[0][1] + matA.values[0][1] * matB.values[1][1] + matA.values[0][2] * 0,
                         matA.values[0][0] * matB.values[0][2] + matA.values[0][1] * matB.values[1][2] + matA.values[0][2] * 1},

                        {matA.values[1][0] * matB.values[0][0] + matA.values[1][1] * matB.values[1][0] + matA.values[1][2] * 0,
                         matA.values[1][0] * matB.values[0][1] + matA.values[1][1] * matB.values[1][1] + matA.values[1][2] * 0,
                         matA.values[1][0] * matB.values[0][2] + matA.values[1][1] * matB.values[1][2] + matA.values[1][2] * 1}}};

        return result;
    }
};