#include "Engine3D.h"

Engine3D::Engine3D(std::string name, int width, int height, float near, float far, float fov)
			: name(name), width(width), height(height), near(near), far(far), fov(fov)
{
	aspectRatio = (float)height / (float)width;
	fovRad = 1.0f / tanf(fov * 0.5f / 180.0f * M_PI);
	isActive = false;

	fillProjMatrix();
}

std::thread Engine3D::startEngine()
{
	//start thread
	std::thread t = std::thread(&Engine3D::engineThread, this);
	
	return t;
}

void Engine3D::engineThread()
{
	//create user resources as part of this thread
	if (!onUserCreate())
		isActive = false;
	else
		isActive = true;

	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();

	//run as fast as possible
	while (isActive)
	{
		//std::cout << "hello from thread 1" << std::endl;

		//handle timing
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTimeDuration = tp2 - tp1;
		tp1 = tp2;
		elapsedTime = elapsedTimeDuration.count();

		//handle frame update
		if (!onUserUpdate(elapsedTime))
			isActive = false;

		//std::this_thread::sleep_for(std::chrono::seconds(1));

		//if (!blockRaster) //redraw window RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);

	}

	if (onUserDestroy())
	{
		//user has permitted destroy, so exit and clean up

	}
	else
	{
		//user denied destroy for some reason, so continue running
		isActive = true;
	}
}

bool Engine3D::onUserCreate()
{
	depthBuffer = new float[width * height];
	// triangle a({0,0,0, 0,1,0, 1,1,0});
	// // triangle b;
	// // triangle c;
	// mdl.modelMesh.tris.push_back(a);

    mdl.modelMesh.tris = {

            // SOUTH
            {0.0f, 0.0f, 0.0f,      0.0f, 1.0f, 0.0f,       1.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 0.0f,      1.0f, 1.0f, 0.0f,       1.0f, 0.0f, 0.0f},

            // EAST
            {1.0f, 0.0f, 0.0f,      1.0f, 1.0f, 0.0f,       1.0f, 1.0f, 1.0f},
            {1.0f, 0.0f, 0.0f,      1.0f, 1.0f, 1.0f,       1.0f, 0.0f, 1.0f},

            // NORTH
            {1.0f, 0.0f, 1.0f,      1.0f, 1.0f, 1.0f,       0.0f, 1.0f, 1.0f},
            {1.0f, 0.0f, 1.0f,      0.0f, 1.0f, 1.0f,       0.0f, 0.0f, 1.0f},

            // WEST
            {0.0f, 0.0f, 1.0f,      0.0f, 1.0f, 1.0f,       0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f,      0.0f, 1.0f, 0.0f,       0.0f, 0.0f, 0.0f},

            // TOP
            {0.0f, 1.0f, 0.0f,      0.0f, 1.0f, 1.0f,       1.0f, 1.0f, 1.0f},
            {0.0f, 1.0f, 0.0f,      1.0f, 1.0f, 1.0f,       1.0f, 1.0f, 0.0f},

            // BOTTOM
            {1.0f, 0.0f, 1.0f,      0.0f, 0.0f, 1.0f,       0.0f, 0.0f, 0.0f},
            {1.0f, 0.0f, 1.0f,      0.0f, 0.0f, 0.0f,       1.0f, 0.0f, 0.0f},

    };
	return true;
}

bool Engine3D::onUserUpdate(float elapsedTime)
{
	theta += 1.0f*elapsedTime;
	//std::cout<< "theta:" << theta << std::endl;
	std::vector<triangle> newTrianglesToProject;

	mat4x4 matRotZ = getRotMatrixZ(theta);
    mat4x4 matRotX = getRotMatrixX(theta);
	mat4x4 matRotY = getRotMatrixY(theta);

    // mat4x4 matTrans = getTranslMatrix(0.0f, 0.0f, 5.0f);

    // mat4x4 matWorld = matTrans;
    // //matWorld = matWorld * matRotZ;
    // matWorld = matWorld * matRotY;
    // matWorld = matTrans * matWorld;

    std::vector<triangle> trianglesToProject;

    // Project triangles into camera view
    for (auto &tri : mdl.modelMesh.tris)
    {


			triangle triProjected, triTranslated, triRotatedZ, triRotatedZX;

            // Rotate Z
            mat4x4 matRotZ = getRotMatrixZ(theta);
            MultiplyMatrixVector(tri.p[0], triRotatedZ.p[0], matRotZ);
            MultiplyMatrixVector(tri.p[1], triRotatedZ.p[1], matRotZ);
            MultiplyMatrixVector(tri.p[2], triRotatedZ.p[2], matRotZ);

            // Rotate X
            mat4x4 matRotX = getRotMatrixX(theta);
            MultiplyMatrixVector(triRotatedZ.p[0], triRotatedZX.p[0], matRotX);
            MultiplyMatrixVector(triRotatedZ.p[1], triRotatedZX.p[1], matRotX);
            MultiplyMatrixVector(triRotatedZ.p[2], triRotatedZX.p[2], matRotX);

            // Translate further along Z
            triTranslated = triRotatedZX;
            triTranslated.p[0].z = triRotatedZX.p[0].z + 3.0f;
            triTranslated.p[1].z = triRotatedZX.p[1].z + 3.0f;
            triTranslated.p[2].z = triRotatedZX.p[2].z + 3.0f;

            MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
            MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
            MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);

            // Scale into view
            triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
            triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
            triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;

            triProjected.p[0].x *= 0.5f * (float)width;
            triProjected.p[0].y *= 0.5f * (float)height;
            triProjected.p[1].x *= 0.5f * (float)width;
            triProjected.p[1].y *= 0.5f * (float)height;
            triProjected.p[2].x *= 0.5f * (float)width;
            triProjected.p[2].y *= 0.5f * (float)height;

            newTrianglesToProject.push_back(triProjected);


        // triangle triTranslated, triProjected;

        // triTranslated = tri * matWorld;

		// // Project triangles from 3D -> 2D
		// triProjected = triTranslated * matProj;

		// // Convert to screen coords: -1...+1 => 0...2 and adjust it with halved screen dimensions
		// triProjected = triProjected + vec3d{ 1, 1, 0, 0 };
		// triProjected = triProjected * vec3d{ 0.5f * (float)width, 0.5f * (float)height, 1, 1 };
		// triProjected = triProjected * vec3d{ 1, -1, 1, 1 };
		// triProjected = triProjected + vec3d{ 0, (float)height, 0, 0 };

		// newTrianglesToProject.push_back(triProjected);
    }

    mtx.lock();
    trianglesToRaster = newTrianglesToProject;
    mtx.unlock();   

	return true;
}

void Engine3D::MultiplyMatrixVector(vec3d& in, vec3d& out, mat4x4& m) {
	out.x = in.x * m.m[0][0] + in.y * m.m[1][0] + in.z * m.m[2][0] + m.m[3][0];
	out.y = in.x * m.m[0][1] + in.y * m.m[1][1] + in.z * m.m[2][1] + m.m[3][1];
	out.z = in.x * m.m[0][2] + in.y * m.m[1][2] + in.z * m.m[2][2] + m.m[3][2];
	float w = in.x * m.m[0][3] + in.y * m.m[1][3] + in.z * m.m[2][3] + m.m[3][3];

	if (w != 0.0f) {
		out.x /= w; out.y /= w; out.z /= w;
	}
}

bool Engine3D::onUserDestroy()
{
	std::cout << "Destroying Engine3D..." << std::endl;
	//delete depth buffer
	delete[] depthBuffer;
	isActive = false;
	return true;
}

void Engine3D::fillProjMatrix()
{
	//projection matrix
	matProj.m[0][0] = aspectRatio * fovRad;
	matProj.m[1][1] = fovRad;
	matProj.m[2][2] = far / (far - near);
	matProj.m[3][2] = (-far * near) / (far - near);
	matProj.m[2][3] = 1.0f;
	matProj.m[3][3] = 0.0f;
}

mat4x4 Engine3D::getProjMatrix()
{
	return matProj;
}

mat4x4 Engine3D::getIdMatrix()
{
	mat4x4 matId;
	matId.m[0][0] = 1.0f;
	matId.m[1][1] = 1.0f;
	matId.m[2][2] = 1.0f;
	matId.m[3][3] = 1.0f;
	return matId;
}

mat4x4 Engine3D::getTranslMatrix(float x, float y, float z)
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

mat4x4 Engine3D::getRotMatrixX(float theta)
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

mat4x4 Engine3D::getRotMatrixY(float theta)
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

mat4x4 Engine3D::getRotMatrixZ(float theta)
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

void Engine3D::clearDepthBuffer()
{
	//clear depth buffer
	for (int i = 0; i < width * height; i++)
		depthBuffer[i] = 0.0f;
}


