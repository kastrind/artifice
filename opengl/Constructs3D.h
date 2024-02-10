#pragma once

#include <glm/glm.hpp>
#include <cmath>
#include <vector>
#include <iostream>

typedef enum {
	NORTH,
	EAST,
	SOUTH,
	WEST,
	TOP,
	BOTTOM
} Orientation;

struct mat4x4
{
	float m[4][4] = { 0 };

	inline mat4x4 operator*(mat4x4& in)
	{
		mat4x4 matrix;
		for (int c = 0; c < 4; c++) {
			for (int r = 0; r < 4; r++) {
				matrix.m[r][c] = m[r][0] * in.m[0][c] + m[r][1] * in.m[1][c] + m[r][2] * in.m[2][c] + m[r][3] * in.m[3][c];
			}
		}
		return matrix;
	}

	inline static mat4x4 getIdMatrix()
	{
		mat4x4 matId;
		matId.m[0][0] = 1.0f;
		matId.m[1][1] = 1.0f;
		matId.m[2][2] = 1.0f;
		matId.m[3][3] = 1.0f;
		return matId;
	}

	inline static mat4x4 getTranslMatrix(float x, float y, float z)
	{
		mat4x4 matTransl;
		matTransl.m[0][0] = 1.0f;
		matTransl.m[1][1] = 1.0f;
		matTransl.m[2][2] = 1.0f;
		matTransl.m[3][3] = 1.0f;
		matTransl.m[3][0] = x;
		matTransl.m[3][1] = y;
		matTransl.m[3][2] = z;
		return matTransl;
	}

	inline static mat4x4 getRotMatrixX(float theta)
	{
		mat4x4 matRotX;
		matRotX.m[0][0] = 1;
		matRotX.m[1][1] = cosf(theta * 0.5f);
		matRotX.m[1][2] = sinf(theta * 0.5f);
		matRotX.m[2][1] = -sinf(theta * 0.5f);
		matRotX.m[2][2] = cosf(theta * 0.5f);;
		matRotX.m[3][3] = 1;
		return matRotX;
	}

	inline static mat4x4 getRotMatrixY(float theta)
	{
		mat4x4 matRotY;
		matRotY.m[0][0] = cosf(theta);
		matRotY.m[0][2] = sinf(theta);
		matRotY.m[2][0] = -sinf(theta);
		matRotY.m[1][1] = 1.0f;
		matRotY.m[2][2] = cosf(theta);
		matRotY.m[3][3] = 1.0f;
		return matRotY;
	}

	inline static mat4x4 getRotMatrixZ(float theta)
	{
		mat4x4 matRotZ;
		matRotZ.m[0][0] = cosf(theta);
		matRotZ.m[0][1] = sinf(theta);
		matRotZ.m[1][0] = -sinf(theta);
		matRotZ.m[1][1] = cosf(theta);
		matRotZ.m[2][2] = 1;
		matRotZ.m[3][3] = 1;
		return matRotZ;
	}

	inline mat4x4 invertRotationOrTranslationMatrix() {
		mat4x4 matrix;
		matrix.m[0][0] = m[0][0]; matrix.m[0][1] = m[1][0]; matrix.m[0][2] = m[2][0]; matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = m[0][1]; matrix.m[1][1] = m[1][1]; matrix.m[1][2] = m[2][1]; matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = m[0][2]; matrix.m[2][1] = m[1][2]; matrix.m[2][2] = m[2][2]; matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = -(m[3][0] * matrix.m[0][0] + m[3][1] * matrix.m[1][0] + m[3][2] * matrix.m[2][0]);
		matrix.m[3][1] = -(m[3][0] * matrix.m[0][1] + m[3][1] * matrix.m[1][1] + m[3][2] * matrix.m[2][1]);
		matrix.m[3][2] = -(m[3][0] * matrix.m[0][2] + m[3][1] * matrix.m[1][2] + m[3][2] * matrix.m[2][2]);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

};
struct vec2d
{
	float u = 0;
	float v = 0;
	float w = 1;
};
struct vec3d
{
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1;

	inline vec3d getNormal(const vec3d& v) {
		vec3d normal;
		normal.x = y * v.z - z * v.y;
		normal.y = z * v.x - x * v.z;
		normal.z = x * v.y - y * v.x;
		return normal;
	}

	inline float getLength() {
		return sqrt(x * x + y * y + z * z);
	}

	inline void normalize() {
		float length = this->getLength();
		x /= length; y /= length; z /= length; w /= length;
	}

	inline float getDotProduct(const vec3d& v) {
		return (x * v.x + y * v.y + z * v.z);
	}

	inline vec3d& operator=(const glm::vec4& in) {
		x = in.x; y = in.y; z = in.z; w = in.w;
		return *(this);
	}

	inline vec3d& operator=(const glm::vec3& in) {
		x = in.x; y = in.y; z = in.z;
		return *(this);
	}

	inline vec3d operator+(const vec3d& in) {
		vec3d out;
		out.x = x + in.x; out.y = y + in.y; out.z = z + in.z; out.w = w + in.w;
		return out;
	}

	inline vec3d operator+(const float& in) {
		vec3d out;
		out.x = x + in; out.y = y + in; out.z = z + in; out.w = w + in;
		return out;
	}

	inline vec3d operator-(const vec3d& in) {
		vec3d out;
		out.x = x - in.x; out.y = y - in.y; out.z = z - in.z; out.w = w - in.w;
		return out;
	}

	inline vec3d operator-(const float& in) {
		vec3d out;
		out.x = x - in; out.y = y - in; out.z = z - in; out.w = w - in;
		return out;
	}

	inline vec3d operator*(const vec3d& in) {
		vec3d out;
		out.x = x * in.x; out.y = y * in.y; out.z = z * in.z; out.w = w * in.w;
		return out;
	}

	inline vec3d operator*(const float& in) {
		vec3d out;
		out.x = x * in; out.y = y * in; out.z = z * in; out.w = w * in;
		return out;
	}

	inline vec3d operator*(const mat4x4& in) {
		vec3d out;
		out.x = x * in.m[0][0] + y * in.m[1][0] + z * in.m[2][0] + w * in.m[3][0];
		out.y = x * in.m[0][1] + y * in.m[1][1] + z * in.m[2][1] + w * in.m[3][1];
		out.z = x * in.m[0][2] + y * in.m[1][2] + z * in.m[2][2] + w * in.m[3][2];
		out.w = x * in.m[0][3] + y * in.m[1][3] + z * in.m[2][3] + w * in.m[3][3];

		return out;
	}

	inline vec3d operator/(const vec3d& in) {
		vec3d out;
		out.x = x / in.x; out.y = y / in.y; out.z = z / in.z; out.w = w / in.w;
		return out;
	}

	inline vec3d operator/(const float& in) {
		vec3d out;
		out.x = x / in; out.y = y / in; out.z = z / in; out.w = w / in;
		return out;
	}

	inline vec3d divByW(const float& w) {
		vec3d out;
		out.x = x / w; out.y = y / w; out.z = z / w;
		return out;
	}
};

struct texturePoint
{
	vec2d t;
	vec2d p;
};

struct triangle
{
	vec3d p[3] = { 0, 0, 0 }; //points
	vec2d t[3] = { 0, 0, 0 }; //texture points

	unsigned char R = 255; unsigned char G = 255; unsigned char B = 255;

	float luminance = 0.0f;

	inline triangle operator+(const vec3d& in) {
		triangle out;
		out.luminance = luminance; out.R = R; out.G = G; out.B = B;
		out.t[0] = t[0]; out.t[1] = t[1]; out.t[2] = t[2];
		out.p[0] = p[0] + in; out.p[1] = p[1] + in; out.p[2] = p[2] + in;
		return out;
	}

	inline triangle operator-(const vec3d& in) {
		triangle out;
		out.luminance = luminance; out.R = R; out.G = G; out.B = B;
		out.t[0] = t[0]; out.t[1] = t[1]; out.t[2] = t[2];
		out.p[0] = p[0] - in; out.p[1] = p[1] - in; out.p[2] = p[2] - in;
		return out;
	}

	inline triangle operator*(const vec3d& in) {
		triangle out;
		out.luminance = luminance; out.R = R; out.G = G; out.B = B;
		out.t[0] = t[0]; out.t[1] = t[1]; out.t[2] = t[2];
		out.p[0] = p[0] * in; out.p[1] = p[1] * in; out.p[2] = p[2] * in;
		return out;
	}

	inline triangle operator*(const mat4x4& in) {
		triangle out;
		out.luminance = luminance; out.R = R; out.G = G; out.B = B;
		out.t[0] = t[0]; out.t[1] = t[1]; out.t[2] = t[2];
		out.p[0] = p[0] * in; out.p[1] = p[1] * in; out.p[2] = p[2] * in;
		return out;
	}

	inline triangle operator/(const vec3d& in) {
		triangle out;
		out.luminance = luminance; out.R = R; out.G = G; out.B = B;
		out.t[0] = t[0]; out.t[1] = t[1]; out.t[2] = t[2];
		out.p[0] = p[0] / in; out.p[1] = p[1] / in; out.p[2] = p[2] / in;
		return out;
	}

	inline triangle divByW() {
		triangle out;
		out.t[0] = t[0]; out.t[1] = t[1]; out.t[2] = t[2];
		out.p[0] = p[0].divByW(p[0].w);
		out.p[1] = p[1].divByW(p[1].w);
		out.p[2] = p[2].divByW(p[2].w);
		return out;
	}

	inline float area()
	{
		//area = [ x1(y2 – y3) + x2(y3 – y1) + x3(y1-y2)]/2
		return std::abs( ( p[0].x * ( p[1].y - p[2].y ) + p[1].x * ( p[2].y - p[0].y ) + p[2].x * ( p[0].y - p[1].y ) ) / 2.0f );
	}

	bool contains(vec3d point)
	{
		//calculate area of triangle ABC
		float ABC_area = area();

		triangle PBC{ point, p[1], p[2] };

		//calculate area of triangle PBC 
		float PBC_area = PBC.area();

		triangle PAC{ p[0], point, p[2] };
		
		//calculate area of triangle PAC
		float PAC_area = PAC.area();

		triangle PAB{ p[0], p[1], point };
		
		//calculate area of triangle PAB
		float PAB_area = PAB.area();

		//check if sum of A1, A2 and A3 is same as A
		//return (ABC_area == PBC_area + PAC_area + PAB_area);
		return std::abs(ABC_area - PBC_area - PAC_area - PAB_area) < 0.01f;
	}
};

struct rectangle
{
	vec3d p;
	float w; float h; //width along x, height along y
	float thetaRotX = 0.0f;
	float thetaRotY = 0.0f;
	float thetaRotZ = 0.0f;

	inline void toTriangles(std::vector<triangle>& triangles) {
		mat4x4 matRotX = mat4x4::getRotMatrixX(thetaRotX);
		mat4x4 matRotY = mat4x4::getRotMatrixY(thetaRotY);
		mat4x4 matRotZ = mat4x4::getRotMatrixZ(thetaRotZ);
		triangle tri1{ p.x, p.y, p.z, 1.0f,      p.x, p.y + h, p.z, 1.0f,          p.x + w, p.y + h, p.z, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f, 1.0f };
		tri1 = tri1 * matRotX;
		tri1 = tri1 * matRotY;
		tri1 = tri1 * matRotZ;
		triangle tri2{ p.x, p.y, p.z, 1.0f,      p.x + w, p.y + h, p.z, 1.0f,      p.x + w, p.y, p.z, 1.0f,        0.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f };
		tri2 = tri2 * matRotX;
		tri2 = tri2 * matRotY;
		tri2 = tri2 * matRotZ;
		triangles.push_back(tri1);
		triangles.push_back(tri2);
	}

};

struct cuboid
{
	vec3d p;
	float w; float h; float d;  //width along x, height along y, depth along z
	float thetaRotX = 0.0f;
	float thetaRotY = 0.0f;
	float thetaRotZ = 0.0f;

	inline void toTriangles(std::vector<triangle>& triangles) {
		mat4x4 matRotX = mat4x4::getRotMatrixX(thetaRotX);
		mat4x4 matRotY = mat4x4::getRotMatrixY(thetaRotY);
		mat4x4 matRotZ = mat4x4::getRotMatrixZ(thetaRotZ);
		//SOUTH
		triangle south1{ p.x, p.y, p.z, 1.0f,      p.x, p.y + h, p.z, 1.0f,          p.x + w, p.y + h, p.z, 1.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f };
		triangle south2{ p.x, p.y, p.z, 1.0f,      p.x + w, p.y + h, p.z, 1.0f,      p.x + w, p.y, p.z, 1.0f,        1.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f };
		south1 = south1 * matRotX; south1 = south1 * matRotY; south1 = south1 * matRotZ;
		south2 = south2 * matRotX; south2 = south2 * matRotY; south2 = south2 * matRotZ;
		triangles.push_back(south1);
		triangles.push_back(south2);
		//EAST
		triangle east1{ p.x + w, p.y, p.z, 1.0f,      p.x + w, p.y + h, p.z, 1.0f,          p.x + w, p.y + h, p.z + d, 1.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f };
		triangle east2{ p.x + w, p.y, p.z, 1.0f,      p.x + w, p.y + h, p.z + d, 1.0f,      p.x + w, p.y, p.z + d,    1.0f,     1.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f };
		east1 = east1 * matRotX; east1 = east1 * matRotY; east1 = east1 * matRotZ;
		east2 = east2 * matRotX; east2 = east2 * matRotY; east2 = east2 * matRotZ;
		triangles.push_back(east1);
		triangles.push_back(east2);
		//NORTH
		triangle north1{ p.x + w, p.y, p.z + d, 1.0f,      p.x + w, p.y + h, p.z + d, 1.0f,          p.x, p.y + h, p.z + d, 1.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f };
		triangle north2{ p.x + w, p.y, p.z + d, 1.0f,      p.x, p.y + h, p.z + d, 1.0f,              p.x, p.y, p.z + d, 1.0f,        1.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f };
		north1 = north1 * matRotX; north1 = north1 * matRotY; north1 = north1 * matRotZ;
		north2 = north2 * matRotX; north2 = north2 * matRotY; north2 = north2 * matRotZ;
		triangles.push_back(north1);
		triangles.push_back(north2);
		//WEST
		triangle west1{ p.x, p.y, p.z + d, 1.0f,      p.x, p.y + h, p.z + d, 1.0f,          p.x, p.y + h, p.z, 1.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f };
		triangle west2{ p.x, p.y, p.z + d, 1.0f,      p.x, p.y + h, p.z, 1.0f,              p.x, p.y, p.z, 1.0f,        1.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f };
		west1 = west1 * matRotX; west1 = west1 * matRotY; west1 = west1 * matRotZ;
		west2 = west2 * matRotX; west2 = west2 * matRotY; west2 = west2 * matRotZ;
		triangles.push_back(west1);
		triangles.push_back(west2);
		//TOP
		triangle top1{ p.x, p.y + h, p.z, 1.0f,      p.x, p.y + h, p.z + d, 1.0f,          p.x + w, p.y + h, p.z + d, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f };
		triangle top2{ p.x, p.y + h, p.z, 1.0f,      p.x + w, p.y + h, p.z + d, 1.0f,      p.x + w, p.y + h, p.z, 1.0f,        0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f };
		top1 = top1 * matRotX; top1 = top1 * matRotY; top1 = top1 * matRotZ;
		top2 = top2 * matRotX; top2 = top2 * matRotY; top2 = top2 * matRotZ;
		triangles.push_back(top1);
		triangles.push_back(top2);
		//BOTTOM
		triangle bottom1{ p.x + w, p.y, p.z + d, 1.0f,      p.x, p.y, p.z + d, 1.0f,          p.x, p.y, p.z, 1.0f,        1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f };
		triangle bottom2{ p.x + w, p.y, p.z + d, 1.0f,      p.x, p.y, p.z, 1.0f,              p.x + w, p.y, p.z, 1.0f,    1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f };
		bottom1 = bottom1 * matRotX; bottom1 = bottom1 * matRotY; bottom1 = bottom1 * matRotZ;
		bottom2 = bottom2 * matRotX; bottom2 = bottom2 * matRotY; bottom2 = bottom2 * matRotZ;
		triangles.push_back(bottom1);
		triangles.push_back(bottom2);
	}

};

struct mesh
{
	std::vector<triangle> tris;
};

struct model {
	int id;
	mesh modelMesh;
	glm::vec3 position;
	glm::mat4 modelMatrix;
	bool inFocus = false;
};