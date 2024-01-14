#include "Engine3D.h"

Engine3D::Engine3D(std::string name, int width, int height, float near, float far, float fov, EventController* ec)
			: name(name), width(width), height(height), near(near), far(far), fov(fov), eventController(ec)
{
	aspectRatio = (float)height / (float)width;
	fovRad = 1.0f / tanf(fov * 0.5f / 180.0f * M_PI);
	isActive = false;

	fillProjMatrix();

	camera = { 0, 0, 0 };
	lookDir = { 0, 0, 1 };
	up = { 0, 1, 0 };
	target = { 0, 0, 1 };
	right = { 1, 0, 0 };
	left = {-1, 0, 0 };
	forward = { 0, 0, 1 };
	light = { 0, 1, -1 };

	matCameraRotY90CW = getRotMatrixY(-cfg.M_PI_HALF);
	matCameraRotY90CCW = getRotMatrixY(cfg.M_PI_HALF);
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

	//rectangle rect{0, 0, 0, 1,    1, 1};
	// std::cout << rect.w << rect.h << std::endl;
	std::vector<triangle> tris;
	//rect.toTriangles(tris);
	// std::cout << "tris size: " << tris.size() << std::endl;
	// std::cout << tris.at(0).p->x << ", " << tris.at(0).p->y << ", " << tris.at(0).p->z << std::endl;
	cuboid box1{0, 0, 0, 1,    1, 1, 1};
	// cuboid box2{5, 0, 0, 1,    2, 2, 2};
	// cuboid box3{10, 0, 7, 1,    8, 3, 3};
	// cuboid box4{15, -3, 12, 1,    4, 8, 4};
	box1.toTriangles(tris);
	// box2.toTriangles(tris);
	// box3.toTriangles(tris);
	// box4.toTriangles(tris);
	mdl.modelMesh.tris = tris;

	// mdl.modelMesh.tris = {

	// 		//SOUTH
	// 		{0.0f, 0.0f, 0.0f, 1.0f,      0.0f, 1.0f, 0.0f, 1.0f,      1.0f, 1.0f, 0.0f, 1.0f},
	// 		{0.0f, 0.0f, 0.0f, 1.0f,      1.0f, 1.0f, 0.0f, 1.0f,      1.0f, 0.0f, 0.0f, 1.0f},

	// 		//EAST
	// 		{1.0f, 0.0f, 0.0f, 1.0f,      1.0f, 1.0f, 0.0f, 1.0f,      1.0f, 1.0f, 1.0f, 1.0f},
	// 		{1.0f, 0.0f, 0.0f, 1.0f,      1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f, 1.0f, 1.0f},

	// 		//NORTH
	// 		{1.0f, 0.0f, 1.0f, 1.0f,     1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 1.0f, 1.0f, 1.0f},
	// 		{1.0f, 0.0f, 1.0f, 1.0f,     0.0f, 1.0f, 1.0f, 1.0f,     0.0f, 0.0f, 1.0f, 1.0f},

	// 		//WEST
	// 		{0.0f, 0.0f, 1.0f, 1.0f,     0.0f, 1.0f, 1.0f, 1.0f,     0.0f, 1.0f, 0.0f, 1.0f},
	// 		{0.0f, 0.0f, 1.0f, 1.0f,     0.0f, 1.0f, 0.0f, 1.0f,     0.0f, 0.0f, 0.0f, 1.0f},

	// 		//BOTTOM
	// 		{0.0f, 1.0f, 0.0f, 1.0f,     0.0f, 1.0f, 1.0f, 1.0f,     1.0f, 1.0f, 1.0f, 1.0f},
	// 		{0.0f, 1.0f, 0.0f, 1.0f,     1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 1.0f, 0.0f, 1.0f},

	// 		//TOP
	// 		{1.0f, 0.0f, 1.0f, 1.0f,     0.0f, 0.0f, 1.0f, 1.0f,     0.0f, 0.0f, 0.0f, 1.0f},
	// 		{1.0f, 0.0f, 1.0f, 1.0f,     0.0f, 0.0f, 0.0f, 1.0f,     1.0f, 0.0f, 0.0f, 1.0f},

	// };
	return true;
}

bool Engine3D::onUserUpdate(float elapsedTime)
{

	//theta += 1.0f * elapsedTime;

	if (eventController != nullptr)
	{
		bool* keysPressed = eventController->getKeysPressed();
		int mouseDistanceX = eventController->getMouseDistanceX();
		int mouseDistanceY = eventController->getMouseDistanceY();
		float multiplierX = std::max((float)mouseDistanceX/5, 1.0f);
		float multiplierY = std::max((float)mouseDistanceY/5, 1.0f);

		if (keysPressed[SupportedKeys::W]) {
			//camera.z += 1.0f * elapsedTime;
			camera = camera + forward;
		} else if (keysPressed[SupportedKeys::S]) {
			//camera.z -= 1.0f * elapsedTime;
			camera = camera - forward;
		}

		if (keysPressed[SupportedKeys::A]) {
			//camera.x -= 1.0f * elapsedTime;
			right = forward * matCameraRotY90CCW;
			camera = camera + right;

		} else if (keysPressed[SupportedKeys::D]) {
			//camera.x += 1.0f * elapsedTime;
			left = forward * matCameraRotY90CW;
			camera = camera + left;
		}

		if (keysPressed[SupportedKeys::LEFT_ARROW] || keysPressed[SupportedKeys::MOUSE_LEFT]) {
			yaw += (multiplierX * elapsedTime) > 0.000001f ? multiplierX * elapsedTime : 0;
			//yaw = yaw > 0.1 ? yaw : 0;
		} else if (keysPressed[SupportedKeys::RIGHT_ARROW] || keysPressed[SupportedKeys::MOUSE_RIGHT]) {
			yaw -= (multiplierX * elapsedTime) > 0.000001f ? multiplierX * elapsedTime : 0;
		}

		if (keysPressed[SupportedKeys::UP_ARROW] || keysPressed[SupportedKeys::MOUSE_UP]) {
			pitch += (multiplierY * elapsedTime) > 0.000001f ? multiplierY * elapsedTime : 0;
		} else if (keysPressed[SupportedKeys::DOWN_ARROW] || keysPressed[SupportedKeys::MOUSE_DOWN]) {
			pitch -= (multiplierY * elapsedTime) > 0.000001f ? multiplierY * elapsedTime : 0;
		}
	}

	//std::cout<< "theta:" << theta << std::endl;
	std::vector<triangle> newTrianglesToProject;

	//rotate
	//mat4x4 matRotZ = getRotMatrixZ(theta);
	//mat4x4 matRotX = getRotMatrixX(theta);
	//mat4x4 matRotY = getRotMatrixY(theta);

	//translate further along Z
	mat4x4 matTrans = getTranslMatrix(0.0f, 0.0f, 5.0f);

	mat4x4 matWorld = matTrans;
	//matWorld = matRotZ * matWorld;
	//matWorld = matRotX * matWorld;

	up = { 0, 1, 0 };
	target = { 0, 0, 1 };
	forward = lookDir * 1.0f * elapsedTime;
	mat4x4 matCameraRotY = getRotMatrixY(yaw);
	mat4x4 matCameraRotX = getRotMatrixX(pitch);
	lookDir = (target * matCameraRotX) * matCameraRotY;
	target = camera + lookDir;

	mat4x4 matCamera = camera.pointAt(target, up);

	mat4x4 matView = matCamera.invertRotationOrTranslationMatrix();

	std::vector<triangle> trianglesToProject;

	vec3d normal, line1, line2, camLine, lightLine;

	//project triangles into camera view
	for (auto &tri : mdl.modelMesh.tris)
	{
		triangle triProjected, triTranslated, triViewed;

		//rotate, translate further along Z
		triTranslated = tri * matWorld;

		//get the triangle normal
		line1 = triTranslated.p[1] - triTranslated.p[0];
		line2 = triTranslated.p[2] - triTranslated.p[0];
		normal = line1.getNormal(line2);
		normal.normalize();
		//get the camera line
		camLine = triTranslated.p[0] - camera;
		camLine.normalize();
		
		//align light with camera
		light = camera;

		//only render visible triangles, i.e. whose normals have negative dot product with the camera line
		if (normal.getDotProduct(camLine) < 0.0f)
		{

			//illumination
			lightLine = triTranslated.p[0] - light;
			lightLine.normalize();
			triTranslated.luminance = std::max(0.2f, std::abs(normal.getDotProduct(lightLine)));

			//convert to view space
			triViewed = triTranslated * matView;

			//project triangles from 3D -> 2D
			triProjected = triViewed * matProj;
			triProjected = triProjected.divByW();

			//convert to screen coords: -1...+1 => 0...2 and adjust it with halved screen dimensions
			triProjected = triProjected + vec3d{ 1, 1, 0, 0 };
			triProjected = triProjected * vec3d{ 0.5f * (float)width, 0.5f * (float)height, 1, 1 };

			//carry luminance
			triProjected.luminance = triTranslated.luminance;

			// Clip Viewed Triangle against near plane and screen edges. This could form additional triangles. 
			triangle clipped[2];
			std::list<triangle> listTriangles;
			vec3d planeTop = { 0.0f, 0.0f, 0.0f };
			vec3d planeTopNormal = { 0.0f, 1.0f, 0.0f };

			vec3d planeBottom = { 0.0f, (float)height -1.0f, 0.0f};
			vec3d planeBottomNormal = { 0.0f, -1.0f, 0.0f };

			vec3d planeLeft = { 0.0f, 0.0f, 0.0f };
			vec3d planeLeftNormal = { 1.0f, 0.0f, 0.0f };

			vec3d planeRight = { (float)width - 1.0f, 0.0f, 0.0f };
			vec3d planeRightNormal = { -1.0f, 0.0f, 0.0f };

			vec3d nearPlane = { 0.0f, 0.0f, 0.1f };
			vec3d nearPlaneNormal = { 0.0f, 0.0f, 1.0f };

			listTriangles.push_back(triProjected);
			int nNewTriangles = 1;
			
			for (int p = 0; p < 5; p++)
			{
				int nTrisToAdd = 0;
				while (nNewTriangles > 0)
				{
					// Take triangle from front of queue
					triangle test = listTriangles.front();
					listTriangles.pop_front();
					nNewTriangles--;

					// Clip it against a plane. We only need to test each 
					// subsequent plane, against subsequent new triangles
					// as all triangles after a plane clip are guaranteed
					// to lie on the inside of the plane. I like how this
					// comment is almost completely and utterly justified
					switch (p)
					{
					case 0:	nTrisToAdd = test.clipAgainstPlane(planeTop, planeTopNormal, clipped[0], clipped[1]); break;
					//case 0: nTrisToAdd = 1; clipped[0] = test; break;
					case 1:	nTrisToAdd = test.clipAgainstPlane(planeBottom, planeBottomNormal, clipped[0], clipped[1]); break;
					//case 1: nTrisToAdd = 1; clipped[0] = test; break;
					case 2:	nTrisToAdd = test.clipAgainstPlane(planeLeft, planeLeftNormal, clipped[0], clipped[1]); break;
					//case 2: nTrisToAdd = 1; clipped[0] = test; break;
					case 3:	nTrisToAdd = test.clipAgainstPlane(planeRight, planeRightNormal, clipped[0], clipped[1]); break;
					//case 3: nTrisToAdd = 1; clipped[0] = test; break;
					case 4: nTrisToAdd = test.clipAgainstPlane(nearPlane, nearPlaneNormal, clipped[0], clipped[1]);
					//case 4: nTrisToAdd = 1; clipped[0] = test; break;
					}

					// Clipping may yield a variable number of triangles, so
					// add these new ones to the back of the queue for subsequent
					// clipping against next planes
					for (int w = 0; w < nTrisToAdd; w++)
						listTriangles.push_back(clipped[w]);
				}
				nNewTriangles = listTriangles.size();
			}
			
			for (auto& triProjected : listTriangles)
			{
				//std::vector<texturePoint> texturePoints = textureTriangle(triProjected);
				//triProjected.texturePoints = texturePoints;
				//trianglesToProject.push_back(triProjected);
				newTrianglesToProject.push_back(triProjected);
			}

			//newTrianglesToProject.push_back(triProjected);
		}

	}

	mtx.lock();
	trianglesToRaster = newTrianglesToProject;
	mtx.unlock();

	return true;
}

bool Engine3D::onUserDestroy()
{
	std::cout << "Destroying Engine3D..." << std::endl;
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


