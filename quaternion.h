#ifndef _QUATERNION_H_
#define _QUATERNION_H_

#include "vectorMath.h"
#include "matrices.h"

class Quaternion
{
public:
	static Quaternion identity;
	static Quaternion fromAxisAngle(const float&, const float&, const float&, const float&);
	static Quaternion inverse(const Quaternion&);
	Quaternion(const Quaternion& = identity);
	Quaternion(const float&, const float&, const float&, const float&);

	void operator =(const Quaternion&);
	void operator *=(const Quaternion&);

	Quaternion inverse() const;
	mat3 matrixRotation() const;
	mat4 matrixTransformation() const;

	~Quaternion();

	float w, x, y, z;
};

Quaternion operator *(const Quaternion&, const Quaternion&);

#endif
