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

template <class T>
struct Matrix2x3;

using Matrix2x3F = Matrix2x3<float>;
using Matrix2x3I = Matrix2x3<int>;

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

    [[nodiscard]] Vector2<T> Normalized() const
    {
        Vector2<T> normalized = Vector2<T>(0, 0);
        float length = this->Length();

        if (length > MathUtility::epsilon)
        {
            normalized = (*this) / this->Length();
        }

        return normalized;
    }

	float GetDistance(Vector2<T> other)
	{
		return sqrt(pow(other.X - X, 2) + pow(other.Y - Y, 2));
	}

	[[nodiscard]] float Dot(Vector2<T> other) const
	{
		return X * other.X + Y * other.Y;
	}

	[[nodiscard]] float Angle(Vector2<T> other) const
	{
		return acos(this->Dot(other) / (this->Length() * other.Length()));
	}

	[[nodiscard]] Vector2<T> Rotate(float angle) const
	{
		float rad = MathUtility::DegreesToRadians(angle);
		float cos = std::cos(rad);
		float sin = std::sin(rad);

		return Vector2<T>(X * cos - Y * sin, X * sin + Y * cos);
	}

	[[nodiscard]] Vector2<T> RotateAround(Vector2<T> point, float angle) const
	{
		Vector2<T> rotated = (*this) - point;
		rotated = rotated.Rotate(angle);
		rotated += point;

		return rotated;
	}

	[[nodiscard]] Vector2<T> Lerp(Vector2<T> other, float t) const
	{
		return Vector2<T>(X + (other.X - X) * t, Y + (other.Y - Y) * t);
	}

	[[nodiscard]] Vector2<T> Project(Vector2<T> other) const
	{
		return other * (this->Dot(other) / other.Dot(other));
	}

	[[nodiscard]] Vector2<T> Reflect(Vector2<T> normal) const
	{
		return (*this) - normal * 2 * this->Dot(normal);
	}

	[[nodiscard]] Vector2<T> Abs() const
	{
		return Vector2<T>(abs(X), abs(Y));
	}

	[[nodiscard]] Vector2<T> Clamp(Vector2<T> min, Vector2<T> max) const
	{
		return Vector2<T>(MIN(MAX(X, min.X), max.X), MIN(MAX(Y, min.Y), max.Y));
	}

	[[nodiscard]] Vector2<T> Clamp(T min, T max) const
	{
		return Vector2<T>(MIN(MAX(X, min), max), MIN(MAX(Y, min), max));
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

template <class T>
struct Matrix2x3
{
    T values[2][3];

    static Matrix2x3<T> IdentityMatrix()
    {
        Matrix2x3<T> mat =
            {
                .values =
                    {
                        1.f, 0.f, 0.f,
                        0.f, 1.f, 0.f}};

        return mat;
    }

    static Matrix2x3<T> TranslationMatrix(Vector2F translation)
    {
        Matrix2x3<T> mat =
            {
                .values =
                    {
                        1.f, 0.f, translation.X,
                        0.f, 1.f, translation.Y}};

        return mat;
    }

    static Matrix2x3<T> RotationMatrix(float angle)
    {
        Matrix2x3<T> mat =
            {
                .values =
                    {
                        {cosf(MathUtility::DegreesToRadians(angle)), -sinf(MathUtility::DegreesToRadians(angle)), 0},
                        {sinf(MathUtility::DegreesToRadians(angle)), cosf(MathUtility::DegreesToRadians(angle)), 0}}};

        return mat;
    }

    static Matrix2x3<T> ScaleMatrix(Vector2F scaleRatio)
    {
        Matrix2x3<T> mat =
            {
                .values =
                    {
                        scaleRatio.X, 0.f, 0.f,
                        0.f, scaleRatio.Y, 0.f}};

        return mat;
    }

    template <class K>
    static Vector2<K> Multiply(Matrix2x3<T> mat, Vector2<K> vec)
    {
        Vector2<K> result =
            {
                vec.X * mat.values[0][0] + vec.Y * mat.values[0][1] + mat.values[0][2],
                vec.X * mat.values[1][0] + vec.Y * mat.values[1][1] + mat.values[1][2]};

        return result;
    }

    static Matrix2x3<T> Multiply(Matrix2x3<T> matA, Matrix2x3<T> matB)
    {
        Matrix2x3<T> result =
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
    static float GetDet2x3(Matrix2x3<T> mat)
    {
        float aei = mat.values[0][0] * mat.values[1][1] * 1;
        float bfg = mat.values[0][1] * mat.values[1][2] * 0;
        float cdh = mat.values[0][2] * mat.values[1][0] * 0;

        float sum1 = aei + bfg + cdh;

        float afh = mat.values[0][0] * mat.values[1][2] * 0;
        float bdi = mat.values[0][1] * mat.values[1][0] * 1;
        float ceg = mat.values[0][2] * mat.values[1][1] * 0;

        float sum2 = afh + bdi + ceg;

        return sum1 - sum2;
    }
    static float GetDet2x2(T n1, T n2,
                           T n3, T n4)
    {
        return (n1 * n4) - (n3 * n2);
    }

    static Matrix2x3<T> Invert(Matrix2x3<T> mat)
    {
        float det = GetDet2x3(mat);
        if (det == 0)
        {
            return IdentityMatrix();
        }
        T transMat[3][3] =
            {
                mat.values[0][0],
                mat.values[1][0],
                0.f,
                mat.values[0][1],
                mat.values[1][1],
                0.f,
                mat.values[0][2],
                mat.values[1][2],
                1.f,
            };

        T t1 = GetDet2x2(transMat[1][1], transMat[1][2], transMat[2][1], transMat[2][2]);
        T t2 = GetDet2x2(transMat[1][0], transMat[1][2], transMat[2][0], transMat[2][2]);
        T t3 = GetDet2x2(transMat[1][0], transMat[1][1], transMat[2][0], transMat[2][1]);
        T t4 = GetDet2x2(transMat[0][1], transMat[0][2], transMat[2][1], transMat[2][2]);
        T t5 = GetDet2x2(transMat[0][0], transMat[0][2], transMat[2][0], transMat[2][2]);
        T t6 = GetDet2x2(transMat[0][0], transMat[0][1], transMat[2][0], transMat[2][1]);
        T t7 = GetDet2x2(transMat[0][1], transMat[0][2], transMat[1][1], transMat[1][2]);
        T t8 = GetDet2x2(transMat[0][0], transMat[0][2], transMat[1][0], transMat[1][2]);
        T t9 = GetDet2x2(transMat[0][0], transMat[0][1], transMat[1][0], transMat[1][1]);

        T adjMat[3][3] =
            {
                t1,
                t2 * -1,
                t3,
                t4 * -1,
                t5,
                t6 * -1,
                t7,
                t8 * -1,
                t9};

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                adjMat[i][j] *= 1.f / det;
            }
        }

        Matrix2x3<T> inverted =
            {
                .values =
                    {
                        adjMat[0][0], adjMat[0][1], adjMat[0][2],
                        adjMat[1][0], adjMat[1][1], adjMat[1][2]}};

        return inverted;
    }

    static Matrix2x3<T> TransformMatrix(Vector2F scaleRatio, float angle, Vector2F translation, Vector2F pivot)
    {
        // Step 1: Move the pivot to the origin (0, 0)
        Matrix2x3<T> toOriginMatrix = TranslationMatrix(Vector2F(-pivot.X, -pivot.Y));

        // Step 2: Create the transformation matrix (scale, rotate, translate)
        Matrix2x3<T> transformationMatrix = Matrix2x3<T>::IdentityMatrix();
        transformationMatrix = Multiply(transformationMatrix, ScaleMatrix(scaleRatio));
        transformationMatrix = Multiply(transformationMatrix, RotationMatrix(angle));
        transformationMatrix = Multiply(transformationMatrix, TranslationMatrix(translation));

        // Step 3: Move back the pivot to its original position
        Matrix2x3<T> fromOriginMatrix = TranslationMatrix(pivot);

        // Step 4: Combine the matrices
        return Multiply(fromOriginMatrix, Multiply(transformationMatrix, toOriginMatrix));
    }

    // template <class T>
    // static Matrix2x3<T> TransformMatrix(Vector2F pivot, Vector2F scaleRatio, float angle, Vector2F translation)
    // {
    //     // Step 1: Move the pivot to the origin (0, 0)
    //     Matrix2x3<T> toOriginMatrix = Matrix2x3<T>::TranslationMatrix(-pivot);

    //     // Step 2: Create the transformation matrix (scale, rotate, translate)
    //     Matrix2x3<T> transformationMatrix = Matrix2x3<T>::IdentityMatrix();
    //     transformationMatrix = Matrix2x3<T>::Multiply(transformationMatrix, Matrix2x3<T>::ScaleMatrix(scaleRatio));
    //     transformationMatrix = Matrix2x3<T>::Multiply(transformationMatrix, Matrix2x3<T>::RotationMatrix(angle));
    //     transformationMatrix = Matrix2x3<T>::Multiply(transformationMatrix, Matrix2x3<T>::TranslationMatrix(translation));

    //     // Step 3: Move back the pivot to its original position
    //     Matrix2x3<T> fromOriginMatrix = Matrix2x3<T>::TranslationMatrix(pivot);

    //     // Step 4: Combine the matrices
    //     return Matrix2x3<T>::Multiply(fromOriginMatrix, Matrix2x3<T>::Multiply(transformationMatrix, toOriginMatrix));
    // }

};