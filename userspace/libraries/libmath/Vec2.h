#pragma once

#include <assert.h>
#include <libio/Format.h>
#include <libmath/MinMax.h>
#include <math.h>

namespace Math
{

template <typename Scalar>
class Vec2
{
private:
    Scalar _x;
    Scalar _y;

public:
    static ALWAYS_INLINE Vec2 one() { return Vec2(1, 1); }
    static ALWAYS_INLINE Vec2 oneX() { return Vec2(1, 0); }
    static ALWAYS_INLINE Vec2 oneY() { return Vec2(0, 1); }
    static ALWAYS_INLINE Vec2 zero() { return Vec2(0, 0); }

    ALWAYS_INLINE const Scalar &x() const { return _x; }
    ALWAYS_INLINE const Scalar &y() const { return _y; }

    ALWAYS_INLINE Scalar &x() { return _x; }
    ALWAYS_INLINE Scalar &y() { return _y; }

    ALWAYS_INLINE double length() const
    {
        return sqrt(_x * _x + _y * _y);
    }

    ALWAYS_INLINE Vec2 extract_x() const { return Vec2(_x, 0); }
    ALWAYS_INLINE Vec2 extract_y() const { return Vec2(0, _y); }

    ALWAYS_INLINE Vec2() : _x(0), _y(0) {}
    ALWAYS_INLINE Vec2(Scalar v) : _x(v), _y(v) {}
    ALWAYS_INLINE Vec2(Scalar x, Scalar y) : _x(x), _y(y) {}

    template <typename OtherScalar>
    ALWAYS_INLINE Vec2(Vec2<OtherScalar> other)
        : _x{static_cast<Scalar>(other.x())},
          _y{static_cast<Scalar>(other.y())}
    {
    }

    ALWAYS_INLINE Vec2 operator+(const Vec2 &rhs) const
    {
        return Vec2(x() + rhs.x(), y() + rhs.y());
    }

    ALWAYS_INLINE Vec2 &operator+=(const Vec2 &rhs)
    {
        *this = *this + rhs;
        return *this;
    }

    ALWAYS_INLINE Vec2 operator-() const
    {
        return Vec2(-x(), -y());
    }

    ALWAYS_INLINE Vec2 operator-(const Vec2 &rhs) const
    {
        return Vec2(x() - rhs.x(), y() - rhs.y());
    }

    ALWAYS_INLINE Vec2 &operator-=(const Vec2 &rhs)
    {
        *this = *this - rhs;
        return *this;
    }

    ALWAYS_INLINE Vec2 operator*(const Vec2 &rhs) const
    {
        return Vec2(x() * rhs.x(), y() * rhs.y());
    }

    template <typename U>
    ALWAYS_INLINE Vec2 operator*(const Vec2<U> &rhs) const
    {
        return Vec2(x() * rhs.x(), y() * rhs.y());
    }

    template <typename U>
    ALWAYS_INLINE Vec2 operator*(U rhs) const
    {
        return Vec2(x() * rhs, y() * rhs);
    }

    template <typename U>
    ALWAYS_INLINE Vec2 operator/(U rhs) const
    {
        return Vec2(x() / rhs, y() / rhs);
    }

    ALWAYS_INLINE Vec2 &operator*=(const Vec2 &rhs)
    {
        *this = *this * rhs;
        return *this;
    }

    ALWAYS_INLINE bool operator==(const Vec2 &rhs) const
    {
        return x() == rhs.x() && y() == rhs.y();
    }

    ALWAYS_INLINE bool operator!=(const Vec2 &rhs) const
    {
        return x() != rhs.x() || y() != rhs.y();
    }

    ALWAYS_INLINE double distance_to(const Vec2 &destination) const
    {
        return (destination - *this).length();
    }

    ALWAYS_INLINE Vec2 clamped(const Vec2 &min, const Vec2 &max) const
    {
        Scalar xx = clamp(x(), min.x(), max.x());
        Scalar yy = clamp(y(), min.y(), max.y());

        return {xx, yy};
    }

    ALWAYS_INLINE Vec2 normalized() const
    {
        Scalar magn = length();

        if (magn != 0)
        {
            return *this / length();
        }
        else
        {
            return {0, 0};
        }
    }

    ALWAYS_INLINE Vec2 vector_to(const Vec2 &destination) const
    {
        return (destination - *this).normalized();
    }

    ALWAYS_INLINE Scalar dot(const Vec2 &other) const
    {
        return x() * other.x() + y() * other.y();
    }

    ALWAYS_INLINE Scalar angle_with(const Vec2 &other) const
    {
        // Normalize the dot product
        auto r = this->normalized().dot(other.normalized());
        auto sign = (x() * other.y() < y() * other.x()) ? -1.0f : 1.0f;

        return sign * acosf(r);
    }
};

template <typename T>
ResultOr<size_t> format(IO::Writer &writer, const IO::Formating &, const Math::Vec2<T> &vec)
{
    return IO::format(writer, "({}, {})", vec.x(), vec.y());
}

using Vec2i = Vec2<int>;
using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;

} // namespace Math
