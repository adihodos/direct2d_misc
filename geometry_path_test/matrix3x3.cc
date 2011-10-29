/*
 * matrix3x3.cc
 *
 *  Created on: Oct 11, 2011
 *      Author: adi.hodos
 */
#include "pch_hdr.h"
#include "matrix3x3.h"

const gfx::matrix3X3 gfx::matrix3X3::null(
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f
		);

const gfx::matrix3X3 gfx::matrix3X3::identity(
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
		);
