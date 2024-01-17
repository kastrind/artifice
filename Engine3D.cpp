#include "Engine3D.h"

Engine3D::Engine3D(int width, int height, float near, float far, float fov, EventController* ec)
				: width(width), height(height), near(near), far(far), fov(fov), eventController(ec)
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

	matCameraRotY90CW = mat4x4::getRotMatrixY(-cfg.M_PI_HALF);
	matCameraRotY90CCW = mat4x4::getRotMatrixY(cfg.M_PI_HALF);

	//planes to clip against and their normals
	planeTop = { 0.0f, 0.0f, 0.0f };
	planeTopNormal = { 0.0f, 1.0f, 0.0f };
	planeBottom = { 0.0f, (float)height -1.0f, 0.0f};
	planeBottomNormal = { 0.0f, -1.0f, 0.0f };
 	planeLeft = { 0.0f, 0.0f, 0.0f };
	planeLeftNormal = { 1.0f, 0.0f, 0.0f };
	planeRight = { (float)width - 1.0f, 0.0f, 0.0f };
	planeRightNormal = { -1.0f, 0.0f, 0.0f };
	nearPlane = { 0.0f, 0.0f, 0.3f };
	nearPlaneNormal = { 0.0f, 0.0f, 1.0f };
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
		//handle timing
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTimeDuration = tp2 - tp1;
		tp1 = tp2;
		elapsedTime = elapsedTimeDuration.count();

		//handle frame update
		if (!onUserUpdate(elapsedTime))
			isActive = false;
	}

	if (onUserDestroy())
	{
		//user has permitted destroy, so exit and clean up
		isActive = false;
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
	return true;
}

bool Engine3D::onUserUpdate(float elapsedTime)
{
	//theta += 1.0f * elapsedTime;
	move();

	//std::cout<< "theta:" << theta << std::endl;
	std::vector<triangle> newTrianglesToProject;

	//translate further along Z
	mat4x4 matTrans = mat4x4::getTranslMatrix(0.0f, 0.0f, 5.0f);

	mat4x4 matWorld = matTrans;

	up = { 0, 1, 0 };
	target = { 0, 0, 1 };
	forward = lookDir * 1.0f * elapsedTime;
	mat4x4 matCameraRotY = mat4x4::getRotMatrixY(yaw);
	mat4x4 matCameraRotX = mat4x4::getRotMatrixX(pitch);
	lookDir = (target * matCameraRotX) * matCameraRotY;
	target = camera + lookDir;

	mat4x4 matCamera = camera.pointAt(target, up);

	mat4x4 matView = matCamera.invertRotationOrTranslationMatrix();

	std::vector<triangle> trianglesToProject;

	vec3d normal, line1, line2, camLine, lightLine;

	//for each model to raster
	for (auto &model : modelsToRaster)
	{
		//project its triangles into camera view
		for (auto &tri : model.modelMesh.tris)
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

				vec3d center{ width/2, height/2, 0 };
				bool lookingAtTriangle = triProjected.contains(center);
				if (lookingAtTriangle) {
					std::cout << "lookingAtTriangle: " << lookingAtTriangle << std::endl;
					triProjected.R = 255;
					triProjected.G = 0;
					triProjected.B = 255;
				}

				std::unique_ptr<std::list<triangle>> listTriangles = clip(triProjected);

				for(auto &triProjected : *listTriangles) {
					//std::vector<texturePoint> texturePoints = textureTriangle(triProjected);
					//triProjected.texturePoints = texturePoints;
					//trianglesToProject.push_back(triProjected);
					newTrianglesToProject.push_back(triProjected);
					
				}

			}

		}

	}

	mtx.lock();
	trianglesToRaster = newTrianglesToProject;
	mtx.unlock();

	return true;
}

std::unique_ptr<std::list<triangle>> Engine3D::clip(triangle tri)
{
	auto listTriangles = std::make_unique<std::list<triangle>>();
	//clip viewed triangle against near plane and screen edges. This could form additional triangles. 
	triangle clipped[2];

	listTriangles->push_back(tri);
	int nNewTriangles = 1;

	for (int p = 0; p < 5; p++)
	{
		int nTrisToAdd = 0;
		while (nNewTriangles > 0)
		{
			//take triangle from front of queue
			triangle test = listTriangles->front();
			listTriangles->pop_front();
			nNewTriangles--;

			//clip it against a plane, as we only need to test each 
			//subsequent plane, against subsequent new triangles
			//as all triangles after a plane clip are guaranteed
			//to lie on the inside of the plane.
			switch (p)
			{
				case 0:	nTrisToAdd = test.clipAgainstPlane(planeTop, planeTopNormal, clipped[0], clipped[1]); break;
				case 1:	nTrisToAdd = test.clipAgainstPlane(planeBottom, planeBottomNormal, clipped[0], clipped[1]); break;
				case 2:	nTrisToAdd = test.clipAgainstPlane(planeLeft, planeLeftNormal, clipped[0], clipped[1]); break;
				case 3:	nTrisToAdd = test.clipAgainstPlane(planeRight, planeRightNormal, clipped[0], clipped[1]); break;
				case 4: nTrisToAdd = test.clipAgainstPlane(nearPlane, nearPlaneNormal, clipped[0], clipped[1]);
			}

			//clipping may yield a variable number of triangles, so
			//add these new ones to the back of the queue for subsequent
			//clipping against next planes
			for (int w = 0; w < nTrisToAdd; w++)
				listTriangles->push_back(clipped[w]);
		}
		nNewTriangles = listTriangles->size();
	}
	return listTriangles;
}

void Engine3D::move()
{
	if (eventController != nullptr)
	{
		bool* keysPressed = eventController->getKeysPressed();
		int mouseDistanceX = eventController->getMouseDistanceX();
		int mouseDistanceY = eventController->getMouseDistanceY();
		float multiplierX = std::max((float)mouseDistanceX/5, 1.0f);
		float multiplierY = std::max((float)mouseDistanceY/5, 1.0f);

		if (keysPressed[SupportedKeys::W]) {
			camera = camera + forward;
		} else if (keysPressed[SupportedKeys::S]) {
			camera = camera - forward;
		}

		if (keysPressed[SupportedKeys::A]) {
			right = forward * matCameraRotY90CCW;
			camera = camera + right;

		} else if (keysPressed[SupportedKeys::D]) {
			left = forward * matCameraRotY90CW;
			camera = camera + left;
		}

		if (keysPressed[SupportedKeys::LEFT_ARROW] || keysPressed[SupportedKeys::MOUSE_LEFT]) {
			yaw += (multiplierX * elapsedTime) > 0.000001f ? multiplierX * elapsedTime : 0;
		} else if (keysPressed[SupportedKeys::RIGHT_ARROW] || keysPressed[SupportedKeys::MOUSE_RIGHT]) {
			yaw -= (multiplierX * elapsedTime) > 0.000001f ? multiplierX * elapsedTime : 0;
		}

		if (keysPressed[SupportedKeys::UP_ARROW] || keysPressed[SupportedKeys::MOUSE_UP]) {
			pitch += (multiplierY * elapsedTime) > 0.000001f ? multiplierY * elapsedTime : 0;
		} else if (keysPressed[SupportedKeys::DOWN_ARROW] || keysPressed[SupportedKeys::MOUSE_DOWN]) {
			pitch -= (multiplierY * elapsedTime) > 0.000001f ? multiplierY * elapsedTime : 0;
		}
	}
}

bool Engine3D::onUserDestroy()
{
	std::cout << "Destroying Engine3D..." << std::endl;
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

void Engine3D::clearDepthBuffer()
{
	//clear depth buffer
	for (int i = 0; i < width * height; i++)
		depthBuffer[i] = 0.0f;
}


