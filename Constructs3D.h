#pragma once

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include "ArtificeShaderProgram.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <vector>
#include <iostream>
#include <memory>


typedef enum axis {
	X,
	Y,
	Z
} axis;


typedef enum orientation {
	NORTH,
	EAST,
	SOUTH,
	WEST,
	TOP,
	BOTTOM
} orientation;


typedef enum shapetype {
	RECTANGLE,
	CUBOID,
	CUBE
} shapetype;

typedef struct matrix
{

	inline static glm::mat4 getIdMatrix()
	{
		glm::mat4 matId = { {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} };
		matId[0][0] = 1.0f;
		matId[1][1] = 1.0f;
		matId[2][2] = 1.0f;
		matId[3][3] = 1.0f;
		return matId;
	}

	inline static glm::mat4 getTranslMatrix(float x, float y, float z)
	{
		glm::mat4 matTransl = { {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} };
		matTransl[0][0] = 1.0f;
		matTransl[1][1] = 1.0f;
		matTransl[2][2] = 1.0f;
		matTransl[3][3] = 1.0f;
		matTransl[3][0] = x;
		matTransl[3][1] = y;
		matTransl[3][2] = z;
		return matTransl;
	}

	inline static glm::mat4 getRotMatrixX(float theta)
	{
		glm::mat4 matRotX = { {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} };
		matRotX[0][0] = 1;
		matRotX[1][1] = cosf(theta * 0.5f);
		matRotX[1][2] = sinf(theta * 0.5f);
		matRotX[2][1] = -sinf(theta * 0.5f);
		matRotX[2][2] = cosf(theta * 0.5f);;
		matRotX[3][3] = 1;
		return matRotX;
	}

	inline static glm::mat4 getRotMatrixY(float theta)
	{
		glm::mat4 matRotY = { {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} };
		matRotY[0][0] = cosf(theta);
		matRotY[0][2] = sinf(theta);
		matRotY[2][0] = -sinf(theta);
		matRotY[1][1] = 1.0f;
		matRotY[2][2] = cosf(theta);
		matRotY[3][3] = 1.0f;
		return matRotY;
	}

	inline static glm::mat4 getRotMatrixZ(float theta)
	{
		glm::mat4 matRotZ = { {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} };
		matRotZ[0][0] = cosf(theta);
		matRotZ[0][1] = sinf(theta);
		matRotZ[1][0] = -sinf(theta);
		matRotZ[1][1] = cosf(theta);
		matRotZ[2][2] = 1;
		matRotZ[3][3] = 1;
		return matRotZ;
	}

} matrix;


typedef struct triangle
{
	glm::vec4 p[3] = { {0.0f, 0.0f, 0.0f, 0.0f},  {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} }; //points
	glm::vec3 t[3] = { {0.0f, 0.0f, 0.0f},  {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }; //texture points

	unsigned char R = 255; unsigned char G = 255; unsigned char B = 255;

	float luminance = 0.0f;

	glm::vec3 tang;

	inline triangle operator+(const glm::vec4& in) {
		triangle out;
		out.luminance = luminance; out.R = R; out.G = G; out.B = B;
		out.t[0] = t[0]; out.t[1] = t[1]; out.t[2] = t[2];
		out.p[0] = p[0] + in; out.p[1] = p[1] + in; out.p[2] = p[2] + in;
		return out;
	}

	inline triangle operator-(const glm::vec4& in) {
		triangle out;
		out.luminance = luminance; out.R = R; out.G = G; out.B = B;
		out.t[0] = t[0]; out.t[1] = t[1]; out.t[2] = t[2];
		out.p[0] = p[0] - in; out.p[1] = p[1] - in; out.p[2] = p[2] - in;
		return out;
	}

	inline triangle operator*(const glm::vec4& in) {
		triangle out;
		out.luminance = luminance; out.R = R; out.G = G; out.B = B;
		out.t[0] = t[0]; out.t[1] = t[1]; out.t[2] = t[2];
		out.p[0] = p[0] * in; out.p[1] = p[1] * in; out.p[2] = p[2] * in;
		return out;
	}

	inline triangle operator*(const glm::mat4& in) {
		triangle out;
		out.luminance = luminance; out.R = R; out.G = G; out.B = B;
		out.t[0] = t[0]; out.t[1] = t[1]; out.t[2] = t[2];
		out.p[0] = p[0] * in; out.p[1] = p[1] * in; out.p[2] = p[2] * in;
		return out;
	}

	inline triangle operator/(const glm::vec4& in) {
		triangle out;
		out.luminance = luminance; out.R = R; out.G = G; out.B = B;
		out.t[0] = t[0]; out.t[1] = t[1]; out.t[2] = t[2];
		out.p[0] = p[0] / in; out.p[1] = p[1] / in; out.p[2] = p[2] / in;
		return out;
	}

	inline float area()
	{
		//area = [ x1(y2 – y3) + x2(y3 – y1) + x3(y1-y2)]/2
		return std::abs( ( p[0].x * ( p[1].y - p[2].y ) + p[1].x * ( p[2].y - p[0].y ) + p[2].x * ( p[0].y - p[1].y ) ) / 2.0f );
	}

	bool contains(glm::vec4 point)
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
		return std::abs(ABC_area - PBC_area - PAC_area - PAB_area) < 0.0001f;
	}

	glm::vec3 calcTangent() {
		glm::vec3 edge1 = p[1] - p[0];
		glm::vec3 edge2 = p[2] - p[0];
		glm::vec2 deltaUV1 = t[1] - t[0];
		glm::vec2 deltaUV2 = t[2] - t[0];
		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		tang.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tang.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tang.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		return tang;
	}

} triangle;

typedef struct shape
{
	public:

		glm::vec4 p = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		float thetaRotX = 0.0f;
		float thetaRotY = 0.0f;
		float thetaRotZ = 0.0f;
		std::vector<triangle> triangles;
		shapetype type;
		glm::mat4 rotationMatrix = glm::mat4(1.0f);

		shape(float thetaRotationX = 0.0f, float thetaRotationY = 0.0f,float thetaRotationZ = 0.0f)
		: thetaRotX(thetaRotationX), thetaRotY(thetaRotationY), thetaRotZ(thetaRotationZ) {
			rotationMatrix = glm::rotate(rotationMatrix, thetaRotX, glm::vec3(1.0f, 0.0f, 0.0f));
			rotationMatrix = glm::rotate(rotationMatrix, thetaRotY, glm::vec3(0.0f, 1.0f, 0.0f));
			rotationMatrix = glm::rotate(rotationMatrix, thetaRotZ, glm::vec3(0.0f, 0.0f, 1.0f));
		}

		shape(shape& s) : thetaRotX(s.thetaRotX), thetaRotY(s.thetaRotY), thetaRotZ(s.thetaRotZ), triangles(s.triangles), rotationMatrix(s.rotationMatrix) {}

		void rotate(float thetaRotationX, float thetaRotationY,float thetaRotationZ) {
			thetaRotX = thetaRotationX; thetaRotY = thetaRotationY; thetaRotZ = thetaRotationZ;
			rotationMatrix = glm::mat4(1.0f);
			rotationMatrix = glm::rotate(rotationMatrix, thetaRotX, glm::vec3(1.0f, 0.0f, 0.0f));
			rotationMatrix = glm::rotate(rotationMatrix, thetaRotY, glm::vec3(0.0f, 1.0f, 0.0f));
			rotationMatrix = glm::rotate(rotationMatrix, thetaRotZ, glm::vec3(0.0f, 0.0f, 1.0f));
		}

	private:

		virtual inline void toTriangles() = 0;

} shape;


typedef struct rectangle : public shape
{
	public:

		float w; float h; //width along x, height along y

		rectangle(float width, float height, float thetaRotationX = 0.0f, float thetaRotationY = 0.0f,float thetaRotationZ = 0.0f)
		: w(width), h(height), shape(thetaRotationX, thetaRotationY, thetaRotationZ)
		{
			toTriangles();
			type = shapetype::RECTANGLE;
		}

	private:

		inline void toTriangles() override {
			//CCW WINDING ORDER
			// 1: (0,1) ┌─────┐ 2: (1,1)
			//          │   / │
			//          │  /  │
			//          │ /   │
			// 3: (0,0) └─────┘ (1,0)
			triangle tri1{ { {p.x, p.y + h, p.z, 1.0f}, {p.x + w, p.y + h, p.z, 1.0f}, {p.x + w, p.y, p.z, 1.0f} }, { {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f} } };
			triangle tri2{ { {p.x, p.y + h, p.z, 1.0f}, {p.x + w, p.y, p.z, 1.0f},     {p.x, p.y, p.z, 1.0f} },     { {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f} } };
			tri1.calcTangent(); tri2.calcTangent();
			triangles.push_back(tri1);
			triangles.push_back(tri2);
		}

} rectangle;


typedef struct cuboid : public shape
{
	public:

		float w; float h; float d;  //width along x, height along y, depth along z

		cuboid(float width, float height, float depth, float thetaRotationX = 0.0f, float thetaRotationY = 0.0f,float thetaRotationZ = 0.0f)
		: w(width), h(height), d(depth), shape(thetaRotationX, thetaRotationY, thetaRotationZ)
		{
			toTriangles();
			type = shapetype::CUBOID;
		}

	private:

		inline void toTriangles() override {
			//ALWAYS CCW WINDING ORDER - SEE COMMENTS ABOVE FOR RECTANGLE
			//SOUTH
			triangle south1{ { {p.x + w, p.y + h, p.z + d, 1.0f}, {p.x, p.y + h, p.z + d, 1.0f}, {p.x + w, p.y, p.z + d, 1.0f} },    { {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f} } };
			triangle south2{ { {p.x, p.y + h, p.z + d, 1.0f},     {p.x, p.y, p.z + d, 1.0f},     {p.x + w, p.y, p.z + d, 1.0f} },    { {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f} } };
			south1.calcTangent(); south2.calcTangent();
			triangles.push_back(south1);
			triangles.push_back(south2);
			//EAST
			triangle east1{ { {p.x + w, p.y + h, p.z, 1.0f}, {p.x + w, p.y + h, p.z + d, 1.0f }, {p.x + w, p.y, p.z + d, 1.0f} },    { {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f} } };
			triangle east2{ { {p.x + w, p.y + h, p.z, 1.0f}, {p.x + w, p.y, p.z + d, 1.0f },     {p.x + w, p.y, p.z, 1.0f} },        { {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f} } };
			east1.calcTangent(); east2.calcTangent();
			triangles.push_back(east1);
			triangles.push_back(east2);
			//NORTH
			triangle north1{ { {p.x, p.y + h, p.z, 1.0f}, {p.x + w, p.y + h, p.z, 1.0f}, {p.x + w, p.y, p.z, 1.0f} }, { {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f} } };
			triangle north2{ { {p.x, p.y + h, p.z, 1.0f}, {p.x + w, p.y, p.z, 1.0f},     {p.x, p.y, p.z, 1.0f} },     { {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f} } };
			north1.calcTangent(); north2.calcTangent();
			triangles.push_back(north1);
			triangles.push_back(north2);
			//WEST
			triangle west1{ { {p.x, p.y + h, p.z + d, 1.0f}, {p.x, p.y + h, p.z, 1.0f}, {p.x, p.y, p.z + d, 1.0f} }, { {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f} } };
			triangle west2{ { {p.x, p.y, p.z + d, 1.0f},     {p.x, p.y + h, p.z, 1.0f}, {p.x, p.y, p.z, 1.0f} },     { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f} } };
			west1.calcTangent(); west2.calcTangent();
			triangles.push_back(west1);
			triangles.push_back(west2);
			//TOP
			triangle top1{ { {p.x, p.y + h, p.z + d, 1.0f},     {p.x + w, p.y + h, p.z + d, 1.0f}, {p.x, p.y + h, p.z, 1.0f} },    { {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} } };
			triangle top2{ { {p.x + w, p.y + h, p.z + d, 1.0f}, {p.x + w, p.y + h, p.z, 1.0f},     {p.x, p.y + h, p.z, 1.0f} },    { {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f} } };
			top1.calcTangent(); top2.calcTangent();
			triangles.push_back(top1);
			triangles.push_back(top2);
			//BOTTOM
			triangle bottom1{ { {p.x, p.y, p.z, 1.0f}, {p.x + w, p.y, p.z + d, 1.0f}, {p.x, p.y, p.z + d, 1.0f} },     { {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f} } };
			triangle bottom2{ { {p.x, p.y, p.z, 1.0f}, {p.x + w, p.y, p.z, 1.0f},     {p.x + w, p.y, p.z + d, 1.0f} }, { {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f} } };
			bottom1.calcTangent(); bottom2.calcTangent();
			triangles.push_back(bottom1);
			triangles.push_back(bottom2);
		}

} cuboid;


typedef struct cube : public shape
{
	public:

		float size; //width along x, height along y, depth along z
		
		cube(float size, float thetaRotationX = 0.0f, float thetaRotationY = 0.0f,float thetaRotationZ = 0.0f)
		: size(size), shape(thetaRotationX, thetaRotationY, thetaRotationZ)
		{
			toTriangles();
			type = shapetype::CUBE;
		}

	private:

		inline void toTriangles() override {
			float s = size / 2.0f;
			//ALWAYS CCW WINDING ORDER - SEE COMMENTS ABOVE FOR RECTANGLE
			//RIGHT - EAST
			triangle east1{ { {p.x + s, p.y - s, p.z + s, 1.0f}, {p.x + s, p.y + s, p.z - s, 1.0f}, {p.x + s, p.y + s, p.z + s, 1.0f} },    { {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f} } };
			triangle east2{ { {p.x + s, p.y - s, p.z + s, 1.0f}, {p.x + s, p.y - s, p.z - s, 1.0f}, {p.x + s, p.y + s, p.z - s, 1.0f} },    { {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f} } };
			east1.calcTangent(); east2.calcTangent();
			triangles.push_back(east1);
			triangles.push_back(east2);
			//LEFT - WEST
			triangle west1{ { {p.x - s, p.y - s, p.z - s, 1.0f}, {p.x - s, p.y + s, p.z + s, 1.0f}, {p.x - s, p.y + s, p.z - s, 1.0f}, },    { {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f} } };
			triangle west2{ { {p.x - s, p.y - s, p.z - s, 1.0f}, {p.x - s, p.y - s, p.z + s, 1.0f}, {p.x - s, p.y + s, p.z + s, 1.0f}, },    { {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f} } };
			west1.calcTangent(); west2.calcTangent();
			triangles.push_back(west1);
			triangles.push_back(west2);
			//TOP
			triangle top1{ { {p.x - s, p.y + s, p.z - s, 1.0f}, {p.x - s, p.y + s, p.z + s, 1.0f}, {p.x + s, p.y + s, p.z - s, 1.0f} },    { {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f} } };
			triangle top2{ { {p.x - s, p.y + s, p.z + s, 1.0f}, {p.x + s, p.y + s, p.z + s, 1.0f}, {p.x + s, p.y + s, p.z - s, 1.0f} },    { {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f} } };
			top1.calcTangent(); top2.calcTangent();
			triangles.push_back(top1);
			triangles.push_back(top2);
			//BOTTOM
			triangle bottom1{ { {p.x - s, p.y - s, p.z - s, 1.0f}, {p.x + s, p.y - s, p.z - s, 1.0f}, {p.x + s, p.y - s, p.z + s, 1.0f} },    { {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f} } };
			triangle bottom2{ { {p.x - s, p.y - s, p.z - s, 1.0f}, {p.x + s, p.y - s, p.z + s, 1.0f}, {p.x - s, p.y - s, p.z + s, 1.0f} },    { {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f} } };
			bottom1.calcTangent(); bottom2.calcTangent();
			triangles.push_back(bottom1);
			triangles.push_back(bottom2);
			//BACK
			triangle back1{ { {p.x - s, p.y - s, p.z - s, 1.0f}, {p.x - s, p.y + s, p.z - s, 1.0f}, {p.x + s, p.y + s, p.z - s, 1.0f} },    { {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f} } };
			triangle back2{ { {p.x - s, p.y - s, p.z - s, 1.0f}, {p.x + s, p.y + s, p.z - s, 1.0f}, {p.x + s, p.y - s, p.z - s, 1.0f} },    { {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f} } };
			back1.calcTangent(); back2.calcTangent();
			triangles.push_back(back1);
			triangles.push_back(back2);
			//FRONT
			triangle front1{ { {p.x - s, p.y + s, p.z + s, 1.0f}, {p.x - s, p.y - s, p.z + s, 1.0f}, {p.x + s, p.y + s, p.z + s, 1.0f} },    { {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f} } };
			triangle front2{ { {p.x - s, p.y - s, p.z + s, 1.0f}, {p.x + s, p.y - s, p.z + s, 1.0f}, {p.x + s, p.y + s, p.z + s, 1.0f} },    { {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f} } };
			front1.calcTangent(); front2.calcTangent();
			triangles.push_back(front1);
			triangles.push_back(front2);
		}

} cube;


typedef struct mesh
{
	shapetype shape;
	std::vector<triangle> tris;

} mesh;

typedef struct boundingbox
{
	float minX = 0.0f; float maxX = 0.0f;
	float minY = 0.0f; float maxY = 0.0f;
	float minZ = 0.0f; float maxZ = 0.0f;
} boundingbox;


typedef struct model {

	public:

		unsigned long id;
		unsigned long compoundModelId = 0;
		unsigned long sn;
		std::string texture;
		glm::vec3 position;
		bool isSolid = true;
		mesh modelMesh;

		bool inFocus = false;
		float distance;
		bool isInDOF = true;
		bool isInFOV = true;
		bool removeFlag = false;
		bool isCovered = false;
		boundingbox bbox;
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		glm::mat4 rotationMatrix = glm::mat4(1.0f);

		unsigned short frameIndex = 0;
		unsigned short frameRows = 1;
		unsigned short frameCols = 1;

		float speed = 0.0f;
		glm::vec3 front = glm::vec3(0.0, 1.0, 0.0);
		float highestY = 0.0f;
		float lowestY = 0.0f;
		unsigned long ignoreForCycles = 0;

		model() {}

		model(unsigned long id, unsigned long sn, std::string texture, glm::vec3 position, mesh modelMesh, bool isSolid = true)
		: id(id), sn(sn), texture(texture), position(position), modelMesh(modelMesh), isSolid(isSolid) {}

		model(unsigned long id, unsigned long sn, std::string texture, glm::vec3 position, shape& shape, bool isSolid = true)
		: id(id), sn(sn), texture(texture), position(position), isSolid(isSolid)
		{
			modelMesh.tris = shape.triangles;
			modelMesh.shape = shape.type;
			rotationMatrix = shape.rotationMatrix;
		}

		virtual void render(ArtificeShaderProgram* geometryShader, GLuint gVAO, GLuint gIBO, GLuint textureId, GLuint lightmapId, GLuint normalmapId, GLuint displacementmapId)
		{
			if (modelMesh.shape == shapetype::RECTANGLE && glIsEnabled(GL_CULL_FACE)) {
				glDisable(GL_CULL_FACE);
			}
			// std::cout << "rendering model" << std::endl;
			geometryShader->bind();
			geometryShader->setInt("material.diffuseTexture", 0);
			geometryShader->setInt("material.lightmap", 1);
			geometryShader->setInt("material.normalmap", 2);
			geometryShader->setInt("material.displacementmap", 3);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureId);
			
			geometryShader->setBool("material.existsLightmap", lightmapId > 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, lightmapId);

			geometryShader->setBool("material.existsNormalmap", normalmapId > 0);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, normalmapId);

			geometryShader->setBool("material.existsDisplacementmap", displacementmapId > 0);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, displacementmapId);

			geometryShader->setMat4("model", this->modelMatrix);
			geometryShader->setInt("frameIndex", this->frameIndex);
			geometryShader->setInt("frameRows", this->frameRows);
			geometryShader->setInt("frameCols", this->frameCols);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
			glBindVertexArray(gVAO);
			glDrawElements(GL_TRIANGLES, this->modelMesh.tris.size() * 3, GL_UNSIGNED_INT, (void*)(( this->sn ) * sizeof(GL_UNSIGNED_INT)));
			glBindVertexArray(0);
			if (modelMesh.shape == shapetype::RECTANGLE && !glIsEnabled(GL_CULL_FACE)) {
				glEnable(GL_CULL_FACE);
			}
		}

		void rotate(float thetaRotationX, float thetaRotationY, float thetaRotationZ) {
			rotationMatrix = glm::mat4(1.0f);
			rotationMatrix = glm::rotate(rotationMatrix, thetaRotationX, glm::vec3(1.0f, 0.0f, 0.0f));
			rotationMatrix = glm::rotate(rotationMatrix, thetaRotationY, glm::vec3(0.0f, 1.0f, 0.0f));
			rotationMatrix = glm::rotate(rotationMatrix, thetaRotationZ, glm::vec3(0.0f, 0.0f, 1.0f));
		}

		virtual void scale(float width, float height, float depth) {
			if (modelMesh.shape == shapetype::RECTANGLE) {
				rectangle rectangle(width, height, 0.0f, 0.0f, 0.0f);
				modelMesh.tris = rectangle.triangles;
			}else if (modelMesh.shape == shapetype::CUBOID) {
				cuboid cuboid(width, height, depth, 0.0f, 0.0f, 0.0f);
				modelMesh.tris = cuboid.triangles;
			}
		}

		float getWidth() {
			glm::vec4 pt[3];
			if (modelMesh.shape == shapetype::RECTANGLE || modelMesh.shape == shapetype::CUBOID) {
				pt[0] = modelMatrix * modelMesh.tris[0].p[0];
				pt[1] = modelMatrix * modelMesh.tris[0].p[1];
				return pt[0].x - pt[1].x;
			}else if (modelMesh.shape == shapetype::CUBE) {
				pt[0] = modelMatrix * modelMesh.tris[4].p[0];
				pt[1] = modelMatrix * modelMesh.tris[4].p[1];
				return pt[1].x - pt[0].x;
			}
			return 0;
		}

		float getHeight() {
			glm::vec4 pt[3];
			if (modelMesh.shape == shapetype::RECTANGLE || modelMesh.shape == shapetype::CUBOID) {
				pt[0] = modelMatrix * modelMesh.tris[0].p[0];
				pt[2] = modelMatrix * modelMesh.tris[0].p[2];
				return pt[0].y - pt[2].y;
			}else if (modelMesh.shape == shapetype::CUBE) {
				pt[0] = modelMatrix * modelMesh.tris[0].p[0];
				pt[1] = modelMatrix * modelMesh.tris[0].p[1];
				return pt[0].y - pt[1].y;
			}
			return 0;
		}

		float getDepth() {
			glm::vec4 pt[3];
			if (modelMesh.shape == shapetype::CUBE || modelMesh.shape == shapetype::CUBOID) {
				pt[0] = modelMatrix * modelMesh.tris[2].p[0];
				pt[1] = modelMatrix * modelMesh.tris[2].p[1];
				return pt[1].z - pt[0].z;
			}else if (modelMesh.shape == shapetype::RECTANGLE) {
				pt[0] = modelMatrix * modelMesh.tris[0].p[0];
				pt[1] = modelMatrix * modelMesh.tris[0].p[2];
				return pt[1].z - pt[0].z;
			}
			return 0;
		}

		void snapTo(glm::vec3 cameraFront, std::shared_ptr<model> m) {
			position = m->position;
			float dpX = glm::dot(cameraFront, glm::vec3(1, 0, 0));
			float dpY = glm::dot(cameraFront, glm::vec3(0, 1, 0));
			float dpZ = glm::dot(cameraFront, glm::vec3(0, 0, 1));
			float absDpX = std::abs(dpX);
			float absDpY = std::abs(dpY);
			float absDpZ = std::abs(dpZ);
			//std::cout << "dpX: " << dpX << ", dpY: " << dpY << ", dpZ: " << dpZ << std::endl;
			//std::cout << "snapping to model with w h d: " << m->getWidth() << ", " << m->getHeight() << ", " << m->getDepth() << std::endl;
			if (absDpX > absDpY && absDpX > absDpZ) { position.x += (dpX / absDpX) * m->getWidth(); }
			if (absDpY > absDpX && absDpY > absDpZ) { position.y += (dpY / absDpY) * m->getHeight(); }
			if (absDpZ > absDpX && absDpZ > absDpY) { position.z += (dpZ / absDpZ) * m->getDepth(); }
		}

		virtual ~model() {}

} model;

typedef struct cubeModel : public model {

	public:

		bool isSkyBox = false;
		bool isActiveSkyBox = false;

		cubeModel(unsigned long id, unsigned long sn, std::string texture, glm::vec3 position, shape& shape, bool isSolid = true)
		: model(id, sn, texture, position, shape, isSolid) {}

		cubeModel(model& m) : model(m) {}

		void render(ArtificeShaderProgram* geometryShader, GLuint gCubeVAO, GLuint gCubeIBO, GLuint textureId, GLuint lightmapId, GLuint normalmapId, GLuint displacementmapId) override
		{
			//std::cout << "rendering cubeModel" << std::endl;
			geometryShader->setInt("material.diffuseTexture", 0);
			geometryShader->setInt("material.lightmap", 1);
			geometryShader->setInt("material.normalmap", 2);
			geometryShader->setInt("material.displacementmap", 3);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

			geometryShader->setBool("material.existsLightmap", lightmapId > 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_CUBE_MAP, lightmapId);

			geometryShader->setBool("material.existsNormalmap", normalmapId > 0);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_CUBE_MAP, normalmapId);

			geometryShader->setBool("material.existsDisplacementmap", displacementmapId > 0);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_CUBE_MAP, displacementmapId);

			if (!isSkyBox) { geometryShader->setMat4("model", this->modelMatrix); }
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gCubeIBO);
			glBindVertexArray(gCubeVAO);
			glDrawElements(GL_TRIANGLES, this->modelMesh.tris.size() * 3, GL_UNSIGNED_INT, (void*)((this->sn) * sizeof(GL_UNSIGNED_INT)));
		}

		virtual void scale(float width, float height, float depth) {
			cube cube(std::max(width, std::max(height, depth)), 0.0f, 0.0f, 0.0f);
			modelMesh.tris = cube.triangles;
		}

		virtual ~cubeModel() {}

} cubeModel;
