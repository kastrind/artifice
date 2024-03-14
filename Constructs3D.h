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


typedef enum {
	TRIANGLE,
	RECTANGLE,
	CUBOID,
	CUBE
} Shape;


struct matrix
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

};


struct triangle
{
	glm::vec4 p[3] = { {0.0f, 0.0f, 0.0f, 0.0f},  {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} }; //points
	glm::vec3 t[3] = { {0.0f, 0.0f, 0.0f},  {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }; //texture points

	unsigned char R = 255; unsigned char G = 255; unsigned char B = 255;

	float luminance = 0.0f;

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
		return std::abs(ABC_area - PBC_area - PAC_area - PAB_area) < 0.01f;
	}
};


struct rectangle
{
	float w; float h; //width along x, height along y
	glm::vec4 p;
	float thetaRotX = 0.0f;
	float thetaRotY = 0.0f;
	float thetaRotZ = 0.0f;

	inline void toTriangles(std::vector<triangle>& triangles) {
		glm::mat4 matRotX = matrix::getRotMatrixX(thetaRotX);
		glm::mat4 matRotY = matrix::getRotMatrixY(thetaRotY);
		glm::mat4 matRotZ = matrix::getRotMatrixZ(thetaRotZ);
		triangle tri1{ { {p.x, p.y, p.z, 1.0f}, {p.x, p.y + h, p.z, 1.0f}, {p.x + w, p.y + h, p.z, 1.0f} },    { {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f} } };
		tri1 = tri1 * matRotX;
		tri1 = tri1 * matRotY;
		tri1 = tri1 * matRotZ;
		triangle tri2{ { {p.x, p.y, p.z, 1.0f}, {p.x + w, p.y + h, p.z, 1.0f}, {p.x + w, p.y, p.z, 1.0f} },    { {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f} } };
		tri2 = tri2 * matRotX;
		tri2 = tri2 * matRotY;
		tri2 = tri2 * matRotZ;
		triangles.push_back(tri1);
		triangles.push_back(tri2);
	}

};


struct cuboid
{
	float w; float h; float d;  //width along x, height along y, depth along z
	glm::vec4 p;
	float thetaRotX = 0.0f;
	float thetaRotY = 0.0f;
	float thetaRotZ = 0.0f;

	inline void toTriangles(std::vector<triangle>& triangles) {
		glm::mat4 matRotX = matrix::getRotMatrixX(thetaRotX);
		glm::mat4 matRotY = matrix::getRotMatrixY(thetaRotY);
		glm::mat4 matRotZ = matrix::getRotMatrixZ(thetaRotZ);
		//SOUTH
		triangle south1{ { {p.x + w, p.y + h, p.z + d, 1.0f}, {p.x + w, p.y, p.z + d, 1.0f}, {p.x, p.y + h, p.z + d, 1.0f} },    { {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} } };
		triangle south2{ { {p.x, p.y + h, p.z + d, 1.0f},     {p.x + w, p.y, p.z + d, 1.0f}, {p.x, p.y, p.z + d, 1.0f} },        { {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f} } };
		south1 = south1 * matRotX; south1 = south1 * matRotY; south1 = south1 * matRotZ;
		south2 = south2 * matRotX; south2 = south2 * matRotY; south2 = south2 * matRotZ;
		triangles.push_back(south1);
		triangles.push_back(south2);
		//EAST
		triangle east1{ { {p.x + w, p.y + h, p.z, 1.0f},     {p.x + w, p.y, p.z, 1.0f}, {p.x + w, p.y + h, p.z + d, 1.0f } },    { {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} } };
		triangle east2{ { {p.x + w, p.y + h, p.z + d, 1.0f}, {p.x + w, p.y, p.z, 1.0f}, {p.x + w, p.y, p.z + d,    1.0f } },     { {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f} } };
		east1 = east1 * matRotX; east1 = east1 * matRotY; east1 = east1 * matRotZ;
		east2 = east2 * matRotX; east2 = east2 * matRotY; east2 = east2 * matRotZ;
		triangles.push_back(east1);
		triangles.push_back(east2);
		//NORTH
		triangle north1{ { {p.x, p.y + h, p.z, 1.0f},     {p.x, p.y, p.z, 1.0f}, {p.x + w, p.y + h, p.z, 1.0f} },    { {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} } };
		triangle north2{ { {p.x + w, p.y + h, p.z, 1.0f}, {p.x, p.y, p.z, 1.0f}, {p.x + w, p.y, p.z, 1.0f} },        { {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f} } };
		north1 = north1 * matRotX; north1 = north1 * matRotY; north1 = north1 * matRotZ;
		north2 = north2 * matRotX; north2 = north2 * matRotY; north2 = north2 * matRotZ;
		triangles.push_back(north1);
		triangles.push_back(north2);
		//WEST
		triangle west1{ { {p.x, p.y + h, p.z + d, 1.0f}, {p.x, p.y, p.z + d, 1.0f}, {p.x, p.y + h, p.z, 1.0f} },    { {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} } };
		triangle west2{ { {p.x, p.y + h, p.z, 1.0f},     {p.x, p.y, p.z + d, 1.0f}, {p.x, p.y, p.z, 1.0f} },        { {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f} } };
		west1 = west1 * matRotX; west1 = west1 * matRotY; west1 = west1 * matRotZ;
		west2 = west2 * matRotX; west2 = west2 * matRotY; west2 = west2 * matRotZ;
		triangles.push_back(west1);
		triangles.push_back(west2);
		//TOP
		triangle top1{ { {p.x, p.y + h, p.z + d, 1.0f},     {p.x, p.y + h, p.z, 1.0f}, {p.x + w, p.y + h, p.z + d, 1.0f} },    { {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f} } };
		triangle top2{ { {p.x + w, p.y + h, p.z + d, 1.0f}, {p.x, p.y + h, p.z, 1.0f}, {p.x + w, p.y + h, p.z, 1.0f} },        { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f} } };
		top1 = top1 * matRotX; top1 = top1 * matRotY; top1 = top1 * matRotZ;
		top2 = top2 * matRotX; top2 = top2 * matRotY; top2 = top2 * matRotZ;
		triangles.push_back(top1);
		triangles.push_back(top2);
		//BOTTOM
		triangle bottom1{ { {p.x, p.y, p.z + d, 1.0f}, {p.x + w, p.y, p.z + d, 1.0f}, {p.x, p.y, p.z, 1.0f} },    { {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f} } };
		triangle bottom2{ { {p.x, p.y, p.z, 1.0f}, {p.x + w, p.y, p.z + d, 1.0f}, {p.x + w, p.y, p.z, 1.0f} },    { {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f} } };
		bottom1 = bottom1 * matRotX; bottom1 = bottom1 * matRotY; bottom1 = bottom1 * matRotZ;
		bottom2 = bottom2 * matRotX; bottom2 = bottom2 * matRotY; bottom2 = bottom2 * matRotZ;
		triangles.push_back(bottom1);
		triangles.push_back(bottom2);
	}

};


struct cube
{
	float size; //width along x, height along y, depth along z
	glm::vec4 p;
	float thetaRotX = 0.0f;
	float thetaRotY = 0.0f;
	float thetaRotZ = 0.0f;

	inline void toTriangles(std::vector<triangle>& triangles) {
		float s = size / 2.0f;
		glm::mat4 matRotX = matrix::getRotMatrixX(thetaRotX);
		glm::mat4 matRotY = matrix::getRotMatrixY(thetaRotY);
		glm::mat4 matRotZ = matrix::getRotMatrixZ(thetaRotZ);
		//RIGHT
		triangle east1{ { {p.x + s, p.y + s, p.z + s, 1.0f}, {p.x + s, p.y + s, p.z - s, 1.0f}, {p.x + s, p.y - s, p.z - s, 1.0f} } };
		triangle east2{ { {p.x + s, p.y - s, p.z + s, 1.0f}, {p.x + s, p.y + s, p.z + s, 1.0f}, {p.x + s, p.y - s, p.z - s, 1.0f} } };
		east1 = east1 * matRotX; east1 = east1 * matRotY; east1 = east1 * matRotZ;
		east2 = east2 * matRotX; east2 = east2 * matRotY; east2 = east2 * matRotZ;
		triangles.push_back(east1);
		triangles.push_back(east2);
		//LEFT
		triangle west1{ { {p.x - s, p.y - s, p.z - s, 1.0f}, {p.x - s, p.y + s, p.z - s, 1.0f}, {p.x - s, p.y + s, p.z + s, 1.0f} } };
		triangle west2{ { {p.x - s, p.y - s, p.z - s, 1.0f}, {p.x - s, p.y + s, p.z + s, 1.0f}, {p.x - s, p.y - s, p.z + s, 1.0f} } };
		west1 = west1 * matRotX; west1 = west1 * matRotY; west1 = west1 * matRotZ;
		west2 = west2 * matRotX; west2 = west2 * matRotY; west2 = west2 * matRotZ;
		triangles.push_back(west1);
		triangles.push_back(west2);
		//TOP
		triangle top1{ { {p.x - s, p.y + s, p.z + s, 1.0f}, {p.x - s, p.y + s, p.z - s, 1.0f}, {p.x + s, p.y + s, p.z + s, 1.0f} } };
		triangle top2{ { {p.x + s, p.y + s, p.z + s, 1.0f}, {p.x - s, p.y + s, p.z - s, 1.0f}, {p.x + s, p.y + s, p.z - s, 1.0f} } };
		top1 = top1 * matRotX; top1 = top1 * matRotY; top1 = top1 * matRotZ;
		top2 = top2 * matRotX; top2 = top2 * matRotY; top2 = top2 * matRotZ;
		triangles.push_back(top1);
		triangles.push_back(top2);
		//BOTTOM
		triangle bottom1{ { {p.x - s, p.y - s, p.z - s, 1.0f}, {p.x - s, p.y - s, p.z + s, 1.0f}, {p.x + s, p.y - s, p.z + s, 1.0f} } };
		triangle bottom2{ { {p.x - s, p.y - s, p.z - s, 1.0f}, {p.x + s, p.y - s, p.z + s, 1.0f}, {p.x + s, p.y - s, p.z - s, 1.0f} } };
		bottom1 = bottom1 * matRotX; bottom1 = bottom1 * matRotY; bottom1 = bottom1 * matRotZ;
		bottom2 = bottom2 * matRotX; bottom2 = bottom2 * matRotY; bottom2 = bottom2 * matRotZ;
		triangles.push_back(bottom1);
		triangles.push_back(bottom2);
		//BACK
		triangle back1{ { {p.x + s, p.y - s, p.z - s, 1.0f}, {p.x + s, p.y + s, p.z - s, 1.0f}, {p.x - s, p.y + s, p.z - s, 1.0f} } };
		triangle back2{ { {p.x + s, p.y - s, p.z - s, 1.0f}, {p.x - s, p.y + s, p.z - s, 1.0f}, {p.x - s, p.y - s, p.z - s, 1.0f} } };
		back1 = back1 * matRotX; back1 = back1 * matRotY; back1 = back1 * matRotZ;
		back2 = back2 * matRotX; back2 = back2 * matRotY; back2 = back2 * matRotZ;
		triangles.push_back(back1);
		triangles.push_back(back2);
		//FRONT
		triangle front1{ { {p.x - s, p.y + s, p.z + s, 1.0f}, {p.x + s, p.y + s, p.z + s, 1.0f}, {p.x + s, p.y - s, p.z + s, 1.0f} } };
		triangle front2{ { {p.x - s, p.y - s, p.z + s, 1.0f}, {p.x - s, p.y + s, p.z + s, 1.0f}, {p.x + s, p.y - s, p.z + s, 1.0f} } };
		front1 = front1 * matRotX; front1 = front1 * matRotY; front1 = front1 * matRotZ;
		front2 = front2 * matRotX; front2 = front2 * matRotY; front2 = front2 * matRotZ;
		triangles.push_back(front1);
		triangles.push_back(front2);
	}

};


struct mesh
{
	Shape shape;
	std::vector<triangle> tris;
};


struct model {
	unsigned int id;
	bool isSolid = true;
	std::string texture;
	bool inFocus = false;
	bool isInDOF = true;
	mesh modelMesh;
	glm::vec3 position;
	glm::mat4 modelMatrix;
};
