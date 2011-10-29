/*
 * vector2.h
 *
 *  Created on: Oct 11, 2011#pragma once
 *      Author: adi.hodos
 */

#ifndef GFX_VECTOR2_H_
#define GFX_VECTOR2_H_


#include <cassert>
#include <cmath>

#if defined(D2D_SUPPORT__)
#include <d2d1.h>
#endif

#include "gfx_misc.h"

namespace gfx {

class vector2 {
public:
    float x_;
    float y_;

    static const vector2 null;

    static const vector2 unit;

    static const vector2 unit_x;

    static const vector2 unit_y;

    vector2() {}

    vector2(float x, float y) : x_(x), y_(y) {}

#if defined(D2D_SUPPORT__)
    vector2(const D2D1_POINT_2F& d2p) : x_(d2p.x), y_(d2p.y) {}

    vector2(const D2D1_POINT_2U& d2p) : x_(d2p.x), y_(d2p.y) {}

    operator D2D1_POINT_2F() const {
        return D2D1::Point2F(x_, y_);
    }

    operator D2D1_SIZE_F() const {
        return D2D1::SizeF(x_, y_);
    }
#endif

    vector2& operator+=(const vector2& rhs) {
        x_ += rhs.x_;
        y_ += rhs.y_;
        return *this;
    }

    vector2& operator-=(const vector2& rhs) {
        x_ -= rhs.x_;
        y_ -= rhs.y_;
        return *this;
    }

    vector2& operator*=(float k) {
        x_ *= k;
        y_ *= k;
        return *this;
    }

    vector2& operator/=(float k) {
        assert(!is_zero(k));
        x_ /= k;
        y_ /= k;
        return *this;
    }

    float sum_components_squared() const {
        return x_ * x_ + y_ * y_;
    }

    float magnitude() const {
        return std::sqrt(sum_components_squared());
    }

    vector2& normalize() {
        float magn(magnitude());
        if (is_zero(magn)) {
            x_ = y_ = 0.0f;
        } else {
            x_ /= magn; y_ /= magn;
        }
        return *this;
    }
};

inline
bool
operator==(const vector2& lhs, const vector2& rhs) {
    return is_zero(lhs.x_ - rhs.x_) && is_zero(lhs.y_ - rhs.y_);
}

inline
bool
operator!=(const vector2& lhs, const vector2& rhs) {
    return !(lhs == rhs);
}

inline
vector2
operator+(const vector2& lhs, const vector2& rhs) {
    vector2 res(lhs);
    res += rhs;
    return res;
}

inline
vector2
operator-(const vector2& lhs, const vector2& rhs) {
    vector2 res(lhs);
    res -= rhs;
    return res;
}

inline
vector2
operator-(const vector2& vec) {
    return vector2(-vec.x_, -vec.y_);
}

inline
vector2
operator*(const vector2& vec, float k) {
    vector2 result(vec);
    result *= k;
    return result;
}

inline
vector2
operator*(float k, const vector2& vec) {
    return vec * k;
}

inline
vector2
operator/(const vector2& vec, float k) {
    vector2 result(vec);
    result /= k;
    return result;
}

inline
float
dot_product(const vector2& lhs, const vector2& rhs) {
    return lhs.x_ * rhs.x_ + lhs.y_ * rhs.y_;
}

inline
bool
ortho_test(const vector2& lhs, const vector2& rhs) {
    return is_zero(dot_product(lhs, rhs));
}

inline
float
angle_of(const vector2& lhs, const vector2& rhs) {
    return std::acos(dot_product(lhs, rhs) / (lhs.magnitude() * rhs.magnitude()));
}

inline
vector2
projection_of(const vector2& lhs, const vector2& rhs) {
    return (dot_product(lhs, rhs) / rhs.sum_components_squared()) * rhs;
}

inline
vector2
normal_of(const vector2& vec) {
    vector2 res(vec);
    res.normalize();
    return res;
}

inline
vector2
ortho_from(const vector2& vec, bool counter_clockwise = true) {
    vector2 result;
    if (counter_clockwise) {
        result.x_ = -vec.y_;
        result.y_ = vec.x_;
    } else {
        result.x_ = vec.y_;
        result.y_ = -vec.x_;
    }
    return result;
}

} /* namespace gfx */
#endif /* VECTOR2_H_ */
