/*
	some functionality in this library requires the use of libVector(32/64).o
*/

#ifndef _MATRIX_H_
#define _MATRIX_H_

#include "vectorMath.h"
#include <iostream>

struct mat3;

struct mat4
{
	static const mat4 identity;
	float data[16];
	mat4(const vector4&, const vector4&, const vector4&, const vector4&);
	mat4(const mat3&, const vector3&);
	mat4(	const float& = 0.0f, const float& = 0.0f, const float& = 0.0f, const float& = 0.0f,
		const float& = 0.0f, const float& = 0.0f, const float& = 0.0f, const float& = 0.0f,
		const float& = 0.0f, const float& = 0.0f, const float& = 0.0f, const float& = 0.0f,
		const float& = 0.0f, const float& = 0.0f, const float& = 0.0f, const float& = 0.0f);
	mat4(const float* data);
	void operator+=(const mat4&);
	mat4 transpose() const;
	mat3 rotmat() const;
	mat4 inverse() const;
};

struct mat3
{
	static const mat3 identity;
	float data[9];
	mat3(const vector3&, const vector3&,  const vector3&);
	mat3(	const float& = 0.0f, const float& = 0.0f, const float& = 0.0f,
		const float& = 0.0f, const float& = 0.0f, const float& = 0.0f,
		const float& = 0.0f, const float& = 0.0f, const float& = 0.0f);
	mat3(const float* data);
	void operator+=(const mat3&);
	mat3 transpose() const;
	mat4 transform(const vector3& translate = vector3(0.0f, 0.0f, 0.0f));
	mat3 inverse() const;
};

bool operator ==(const mat3&, const mat3&);
bool operator ==(const mat4&, const mat4&);

std::ostream &operator <<(std::ostream&, const mat4&);
std::ostream &operator <<(std::ostream&, const mat3&);

mat4 operator*(const mat4&, const float&);
mat4 operator*(const float&, const mat4&);
mat3 operator*(const mat3&, const float&);
mat3 operator*(const float&, const mat3&);

mat4 operator*(const mat4&, const mat4&);
mat3 operator*(const mat3&, const mat3&);

mat4 operator+(const mat4&, const mat4&);
mat3 operator+(const mat3&, const mat3&);

vector4 operator*(const mat4&, const vector4&);
vector3 operator*(const mat3&, const vector3&);

#endif
