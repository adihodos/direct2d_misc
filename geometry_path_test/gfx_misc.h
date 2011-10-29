/*
 * gfx_misc.h
 *
 *  Created on: Oct 5, 2011
 *      Author: adi.hodos
 */

#ifndef GFX_MISC_H_
#define GFX_MISC_H_

#include <cassert>
#include <cmath>

namespace gfx {

const float EPSILON = 0.000001f;

const float PI = 3.14159265f;

template<typename T>
inline T clamp(const T& val, const T& min, const T& max) {
	return val <= min ? min : (val >= max ? max : val);
}

inline
float
deg2rads(
		float degs
		)
{
	return (PI * degs) / 180.0f;
}

inline
float
rads2degs(
		float rads
		)
{
	return (rads * 180.0f) / PI;
}

inline
bool
is_zero(
		float val
		)
{
	return std::fabs(val) <= EPSILON;
}

} // ns gfx


#endif /* GFX_MISC_H_ */
