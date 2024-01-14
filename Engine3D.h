#pragma once


#include "Configuration.h"
#include "EventController.h"
#include <SDL.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <list>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>

typedef enum {
	NORTH,
	EAST,
	SOUTH,
	WEST
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

	inline mat4x4 pointAt(vec3d& target, vec3d& up)
	{
		vec3d pos = { x, y, z, w };

		// calc. new forward direction
		vec3d newForward = target - pos;
		newForward.normalize();

		// calc. new up direction
		vec3d a = newForward * up.getDotProduct(newForward);
		vec3d newUp = up - a;
		newUp.normalize();

		// new right direction is the cross product
		vec3d newRight = newUp.getNormal(newForward);

		// construct dimensioning and translation Matrix	
		mat4x4 matrix;
		matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
		return matrix;
	}

	inline vec3d intersectPlane(vec3d& planeNormal, vec3d& lineStart, vec3d& lineEnd, float& t)
	{
		planeNormal.normalize();
		float planeNormalPointDP = -planeNormal.getDotProduct(*this);
		float lineStartNormalDP = lineStart.getDotProduct(planeNormal);
		float lineEndNormalDP = lineEnd.getDotProduct(planeNormal);
		t = (-planeNormalPointDP - lineStartNormalDP) / (lineEndNormalDP - lineStartNormalDP);
		vec3d lineStartToEnd = lineEnd - lineStart;
		vec3d lineToIntersect = lineStartToEnd * t;
		return lineStart + lineToIntersect;
	}
};
struct triangle
{
	vec3d p[3] = { 0, 0, 0 }; // points

	unsigned char R = 255; unsigned char G = 255; unsigned char B = 255;

	float luminance = 0.0f;

	inline triangle operator+(const vec3d& in) {
		triangle out;
		out.luminance = luminance; out.R = R; out.G = G; out.B = B;
		out.p[0] = p[0] + in; out.p[1] = p[1] + in; out.p[2] = p[2] + in;
		return out;
	}

	inline triangle operator-(const vec3d& in) {
		triangle out;
		out.luminance = luminance; out.R = R; out.G = G; out.B = B;
		out.p[0] = p[0] - in; out.p[1] = p[1] - in; out.p[2] = p[2] - in;
		return out;
	}

	inline triangle operator*(const vec3d& in) {
		triangle out;
		out.luminance = luminance; out.R = R; out.G = G; out.B = B;
		out.p[0] = p[0] * in; out.p[1] = p[1] * in; out.p[2] = p[2] * in;
		return out;
	}

	inline triangle operator*(const mat4x4& in) {
		triangle out;
		out.luminance = luminance; out.R = R; out.G = G; out.B = B;
		out.p[0] = p[0] * in; out.p[1] = p[1] * in; out.p[2] = p[2] * in;
		return out;
	}

	inline triangle operator/(const vec3d& in) {
		triangle out;
		out.luminance = luminance; out.R = R; out.G = G; out.B = B;
		out.p[0] = p[0] / in; out.p[1] = p[1] / in; out.p[2] = p[2] / in;
		return out;
	}

	inline triangle divByW() {
		triangle out;
		out.p[0] = p[0].w > 0 ? p[0].divByW(p[0].w) : p[0];
		out.p[1] = p[1].w > 0 ? p[1].divByW(p[1].w) : p[1];
		out.p[2] = p[2].w > 0 ? p[2].divByW(p[2].w) : p[2];
		return out;
	}
};

struct rectangle
{
	vec3d p;
	float w; float h; //width along x, height along y
	float zOff = 0;

	inline void toTriangles(std::vector<triangle>& triangles) {
		triangles.push_back({ p.x, p.y, p.z, 1.0f,      p.x, p.y + h, p.z, 1.0f,          p.x + w, p.y + h, p.z + zOff, 1.0f });
		triangles.push_back({ p.x, p.y, p.z, 1.0f,      p.x + w, p.y + h, p.z + zOff, 1.0f,      p.x + w, p.y, p.z + zOff, 1.0f });
	}
};

struct cuboid
{
	vec3d p;
	float w; float h; float d;  //width along x, height along y, depth along z

	inline void toTriangles(std::vector<triangle>& triangles) {
		//SOUTH
		triangles.push_back({ p.x, p.y, p.z, 1.0f,      p.x, p.y + h, p.z, 1.0f,          p.x + w, p.y + h, p.z, 1.0f });
		triangles.push_back({ p.x, p.y, p.z, 1.0f,      p.x + w, p.y + h, p.z, 1.0f,      p.x + w, p.y, p.z, 1.0f });

		//EAST
		triangles.push_back({ p.x + w, p.y, p.z, 1.0f,      p.x + w, p.y + h, p.z, 1.0f,          p.x + w, p.y + h, p.z + d, 1.0f });
		triangles.push_back({ p.x + w, p.y, p.z, 1.0f,      p.x + w, p.y + h, p.z + d, 1.0f,      p.x + w, p.y, p.z + d, 1.0f });

		//NORTH
		triangles.push_back({ p.x + w, p.y, p.z + d, 1.0f,      p.x + w, p.y + h, p.z + d, 1.0f,          p.x, p.y + h, p.z + d, 1.0f });
		triangles.push_back({ p.x + w, p.y, p.z + d, 1.0f,      p.x, p.y + h, p.z + d, 1.0f,              p.x, p.y, p.z + d, 1.0f });

		//WEST
		triangles.push_back({ p.x, p.y, p.z + d, 1.0f,      p.x, p.y + h, p.z + d, 1.0f,          p.x, p.y + h, p.z, 1.0f });
		triangles.push_back({ p.x, p.y, p.z + d, 1.0f,      p.x, p.y + h, p.z, 1.0f,              p.x, p.y, p.z, 1.0f });

		//BOTTOM
		triangles.push_back({ p.x, p.y + h, p.z, 1.0f,      p.x, p.y + h, p.z + d, 1.0f,          p.x + w, p.y + h, p.z + d, 1.0f });
		triangles.push_back({ p.x, p.y + h, p.z, 1.0f,      p.x + w, p.y + h, p.z + d, 1.0f,      p.x + w, p.y + h, p.z, 1.0f });

		//TOP
		triangles.push_back({ p.x + w, p.y, p.z + d, 1.0f,      p.x, p.y, p.z + d, 1.0f,          p.x, p.y, p.z, 1.0f });
		triangles.push_back({ p.x + w, p.y, p.z + d, 1.0f,      p.x, p.y, p.z, 1.0f,              p.x + w, p.y, p.z, 1.0f });
	}

};

struct mesh
{
	std::vector<triangle> tris;
};

struct model {
	mesh modelMesh;
};

class Engine3D
{
	public:

		Engine3D(std::string name, int width=320, int height=240, float near = 0.1f, float far = 1000.0f, float fov = 90.0f, EventController* eventController = nullptr);

		std::thread startEngine();

		bool onUserCreate();

		bool onUserUpdate(float elapsedTime);

		virtual bool onUserDestroy();

		void fillProjMatrix();

		mat4x4 getProjMatrix();

		mat4x4 getIdMatrix();

		mat4x4 getTranslMatrix(float x, float y, float z);

		mat4x4 getRotMatrixX(float theta);

		mat4x4 getRotMatrixY(float theta);

		mat4x4 getRotMatrixZ(float theta);

		void clearDepthBuffer();

		std::atomic<bool> isActive;

		std::atomic<bool> blockRaster;

		float elapsedTime;

		std::string name;

		std::mutex mtx;

		std::vector<triangle> trianglesToRaster;

		model mdl;

	private:

		int width;
		int height;
		float near;
		float far;
		float fov;
		float aspectRatio;
		float fovRad;

		mat4x4 matProj;

		float* depthBuffer = nullptr;

		float theta = 0;

		float yaw = 0;

		float pitch = 0;

		mat4x4 matCameraRotY90CW;

		mat4x4 matCameraRotY90CCW;

		vec3d lookDir;

		vec3d up;

		vec3d camera;

		vec3d target;

		vec3d forward;

		vec3d right;

		vec3d left;

		vec3d light;

		EventController* eventController;

		void engineThread();

};