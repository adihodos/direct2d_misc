/*
 * matrix3x3.h
 *
 *  Created on: Oct 11, 2011
 *      Author: adi.hodos
 */

#ifndef MATRIX3X3_H_
#define MATRIX3X3_H_

#include <cassert>
#include "gfx_misc.h"
#include "vector2.h"

namespace gfx {

class matrix3X3;

inline
matrix3X3
transpose_of(const matrix3X3&);

class matrix3X3 {
public:
	union {
		struct {
			float a11_, a12_, a13_;
			float a21_, a22_, a23_;
			float a31_, a32_, a33_;
		};
		float elements_[9];
	};

	static const matrix3X3 null;

	static const matrix3X3 identity;

	static matrix3X3 translation(float x0, float y0) {
		return matrix3X3(
				1.0f, 0.0f, x0,
				0.0f, 1.0f, y0,
				0.0f, 0.0f, 1.0f
				);
	}

	static matrix3X3 translation(const vector2& org) {
		return matrix3X3::translation(org.x_, org.y_);
	}

	static matrix3X3 rotation(float theta, float x_org = 0.0f, float y_org = 0.0f) {
		float sin_theta = std::sin(deg2rads(theta));
		float cos_theta = std::cos(deg2rads(theta));
		/*
		 * x = x0 + (x - x0) cost - (y - y0) sint
		 * y = y0 + (x - x0) sint + (y - y0) cost
		 */
		return matrix3X3(
				cos_theta, -sin_theta, 	x_org * (1.0f - cos_theta) + y_org * sin_theta,
				sin_theta,  cos_theta, 	y_org * (1.0f - cos_theta) + x_org * sin_theta,
				0.0f, 		0.0f, 		1.0f
				);
	}

	static matrix3X3 rotation(float theta, const vector2& org) {
		return matrix3X3::rotation(theta, org.x_, org.y_);
	}

	static matrix3X3 scale(float sx, float sy, float x_org = 0.0f, float y_org = 0.0f) {
		/*
		 * x = x0 + (x - x0) sx = x0 + xsx - x0sx = sxx + x0(1-sx)
		 */
		return matrix3X3(
				sx, 0.0f, (1.0f - sx) * x_org,
				0.0f, sy, (1.0f - sy) * y_org,
				0.0f, 0.0f, 1.0f
				);
	}

	static matrix3X3 scale(float sx, float sy, const vector2& org) {
		return matrix3X3::scale(sx, sy, org.x_, org.y_);
	}

	matrix3X3() {}

	matrix3X3(
			float a11, float a12, float a13,
			float a21, float a22, float a23,
			float a31, float a32, float a33
			)
	{
		a11_ = a11; a12_ = a12; a13_ = a13;
		a21_ = a21; a22_ = a22; a23_ = a23;
		a31_ = a31; a32_ = a32; a33_ = a33;
	}

	matrix3X3& operator+=(const matrix3X3& rhs) {
		a11_ += rhs.a11_; a12_ += rhs.a12_; a13_ += rhs.a13_;
		a21_ += rhs.a21_; a22_ += rhs.a22_; a23_ += rhs.a23_;
		a31_ += rhs.a13_; a32_ += rhs.a32_; a33_ += rhs.a33_;
		return *this;
	}

	matrix3X3& operator-=(const matrix3X3& rhs) {
		a11_ -= rhs.a11_; a12_ -= rhs.a12_; a13_ -= rhs.a13_;
		a21_ -= rhs.a21_; a22_ -= rhs.a22_; a23_ -= rhs.a23_;
		a31_ -= rhs.a13_; a32_ -= rhs.a32_; a33_ -= rhs.a33_;
		return *this;
	}

	matrix3X3& operator*=(float k) {
		a11_ *= k; a12_ *= k; a13_ *= k;
		a21_ *= k; a22_ *= k; a23_ *= k;
		a31_ *= k; a32_ *= k; a33_ *= k;
		return *this;
	}

	matrix3X3& operator/=(float k) {
		a11_ /= k; a12_ /= k; a13_ /= k;
		a21_ /= k; a22_ /= k; a23_ /= k;
		a31_ /= k; a32_ /= k; a33_ /= k;
		return *this;
	}

	float determinant() const {
		float A11 = a22_ * a33_ - a23_ * a32_;
		float A12 = a23_ * a31_ + a21_ * a33_;
		float A13 = a21_ * a32_ - a22_ * a31_;

		return a11_ * A11 + a12_ * A12 + a13_ * A13;
	}

	bool is_invertible() const {
		return !is_zero(determinant());
	}

	matrix3X3 adjoint() const {
		float A11 = a22_ * a33_ - a23_ * a32_;
		float A12 = a23_ * a31_ + a21_ * a33_;
		float A13 = a21_ * a32_ - a22_ * a31_;
		float A21 = a13_ * a32_ - a12_ * a33_;
		float A22 = a11_ * a33_ - a13_ * a31_;
		float A23 = a12_ * a31_ - a11_ * a32_;
		float A31 = a12_ * a23_ - a13_ * a22_;
		float A32 = a13_ * a21_ - a11_ * a23_;
		float A33 = a11_ * a12_ - a12_ * a21_;

		return matrix3X3(A11, A21, A31, A12, A22, A32, A13, A23, A33);
	}

	matrix3X3& invert() {
		float det(determinant());
		assert(!is_zero(det));
		matrix3X3 adjoint_mtx(adjoint());
		adjoint_mtx /= det;
		*this = adjoint_mtx;
		return *this;
	}

	matrix3X3& transpose() {
		*this = transpose_of(*this);
		return *this;
	}
};

inline
matrix3X3
operator+(const matrix3X3& lhs, const matrix3X3& rhs) {
	matrix3X3 result(lhs);
	result += rhs;
	return result;
}

inline
matrix3X3
operator-(const matrix3X3& lhs, const matrix3X3& rhs) {
	matrix3X3 result(lhs);
	result -= rhs;
	return result;
}

inline
matrix3X3
operator-(const matrix3X3& mtx) {
	matrix3X3 result(mtx);
	result *= -1.0f;
	return result;
}

inline
matrix3X3
operator*(const matrix3X3& lhs, const matrix3X3& rhs) {
	matrix3X3 res;
	res.a11_ = lhs.a11_ * rhs.a11_ + lhs.a12_ * rhs.a21_ + lhs.a13_ * rhs.a31_;
	res.a12_ = lhs.a11_ * rhs.a12_ + lhs.a12_ * rhs.a22_ + lhs.a13_ * rhs.a32_;
	res.a13_ = lhs.a11_ * rhs.a13_ + lhs.a12_ * rhs.a23_ + lhs.a13_ * rhs.a33_;

	res.a21_ = lhs.a21_ * rhs.a11_ + lhs.a22_ * rhs.a21_ + lhs.a23_ * rhs.a31_;
	res.a22_ = lhs.a21_ * rhs.a12_ + lhs.a22_ * rhs.a22_ + lhs.a23_ * rhs.a32_;
	res.a23_ = lhs.a21_ * rhs.a13_ + lhs.a22_ * rhs.a23_ + lhs.a23_ * rhs.a33_;

	res.a31_ = lhs.a31_ * rhs.a11_ + lhs.a32_ * rhs.a21_ + lhs.a33_ * rhs.a31_;
	res.a32_ = lhs.a31_ * rhs.a12_ + lhs.a32_ * rhs.a22_ + lhs.a33_ * rhs.a32_;
	res.a33_ = lhs.a31_ * rhs.a13_ + lhs.a32_ * rhs.a23_ + lhs.a33_ * rhs.a33_;

	return res;
}

inline
matrix3X3
operator*(float k, const matrix3X3& mtx) {
	matrix3X3 result(mtx);
	result *= k;
	return result;
}

inline
matrix3X3
operator*(const matrix3X3& mtx, float k) {
	return k * mtx;
}

inline
vector2
operator*(const matrix3X3& mtx, const vector2& vec) {
	return vector2(
			mtx.a11_ * vec.x_ + mtx.a12_ * vec.y_ + mtx.a13_,
			mtx.a21_ * vec.x_ + mtx.a22_ * vec.y_ + mtx.a23_
			);
}

inline
matrix3X3
inverse_of(const matrix3X3& mtx) {
	matrix3X3 result(mtx);
	result.invert();
	return result;
}

inline
matrix3X3
transpose_of(const matrix3X3& mtx) {
	return matrix3X3(
			mtx.a11_, mtx.a21_, mtx.a31_,
			mtx.a12_, mtx.a22_, mtx.a32_,
			mtx.a13_, mtx.a23_, mtx.a33_
			);
}

} /* namespace gfx */
#endif /* MATRIX3X3_H_ */
