#include "Engine3D.h"

Engine3D::Engine3D(int width, int height, float near, float far, float fov, EventController* ec)
				: width(width), height(height), near(near), far(far), fov(fov), eventController(ec)
{
	aspectRatio = (float)height / (float)width;
	fovRad = 1.0f / tanf(fov * 0.5f / 180.0f * M_PI);
	isActive = false;

	fillProjMatrix();

	projectionMatrix = glm::perspective(glm::radians((float)fov), (float)width / (float)height, near, far);

	//camera
	cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

	viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	camera = vec3d{ 0, 0, 0 };
	lookDir = vec3d{ 0, 0, 1 };
	up = vec3d{ 0, 1, 0 };
	target = vec3d{ 0, 0, 1 };
	right = vec3d{ 1, 0, 0 };
	left = vec3d{-1, 0, 0 };
	forward = vec3d{ 0, 0, 1 };
	light = vec3d{ 0, 1, -1 };

	matCameraRotY90CW = mat4x4::getRotMatrixY(-cfg.M_PI_HALF);
	matCameraRotY90CCW = mat4x4::getRotMatrixY(cfg.M_PI_HALF);

	//planes to clip against and their normals
	planeTop = vec3d{ 0.0f, 0.0f, 0.0f };
	planeTopNormal = vec3d{ 0.0f, 1.0f, 0.0f };
	planeBottom = vec3d{ 0.0f, (float)height -1.0f, 0.0f};
	planeBottomNormal = vec3d{ 0.0f, -1.0f, 0.0f };
 	planeLeft = vec3d{ 0.0f, 0.0f, 0.0f };
	planeLeftNormal = vec3d{ 1.0f, 0.0f, 0.0f };
	planeRight = vec3d{ (float)width - 1.0f, 0.0f, 0.0f };
	planeRightNormal = vec3d{ -1.0f, 0.0f, 0.0f };
	nearPlane = vec3d{ 0.0f, 0.0f, 0.3f };
	nearPlaneNormal = vec3d{ 0.0f, 0.0f, 1.0f };
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
	mtx.lock();
	move(elapsedTime);
	/*
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

	clearDepthBuffer();
	*/

	projectionMatrix = glm::perspective(glm::radians((float)fov), (float)width / (float)height, near, far);
	viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	mtx.unlock();

	//for each model to raster
	for (auto &model : modelsToRaster)
	{

		mtx.lock();
		glm::mat4 modelMatrix = glm::mat4(1.0f); //make sure to initialize matrix to identity matrix first
		modelMatrix = glm::translate(modelMatrix, model.position);
		model.modelMatrix = modelMatrix;
		mtx.unlock();

		for (auto tri : model.modelMesh.tris)
		{
			// glm::vec4 test[2] = { {0, 1, 2, 3}, {4, 5, 6, 7} };
			// std::cout << "test!0: " << test[0].w << "test!1: " << test[1].w << std::endl;
			glm::vec4 pt[3];
			pt[0] = { tri.p[0].x, tri.p[0].y, tri.p[0].z, tri.p[0].w };
			pt[1] = { tri.p[1].x, tri.p[1].y, tri.p[1].z, tri.p[1].w };
			pt[2] = { tri.p[2].x, tri.p[2].y, tri.p[2].z, tri.p[2].w };

			pt[0] = projectionMatrix * viewMatrix * modelMatrix * pt[0];
			pt[1] = projectionMatrix * viewMatrix * modelMatrix * pt[1];
			pt[2] = projectionMatrix * viewMatrix * modelMatrix * pt[2];

			tri.p[0] = pt[0];
			tri.p[1] = pt[1];
			tri.p[2] = pt[2];
/*
			tri.p[0].x = pt[0].x;
			tri.p[0].y = pt[0].y;
			tri.p[0].z = pt[0].z;
			tri.p[0].w = pt[0].w;

			tri.p[1].x = pt[1].x;
			tri.p[1].y = pt[1].y;
			tri.p[1].z = pt[1].z;
			tri.p[1].w = pt[1].w;

			tri.p[2].x = pt[2].x;
			tri.p[2].y = pt[2].y;
			tri.p[2].z = pt[2].z;
			tri.p[2].w = pt[2].w;
*/

/*
			float area = std::abs( ( tri.pt[0].x * ( tri.pt[1].y - tri.pt[2].y ) + tri.pt[1].x * ( tri.pt[2].y - tri.pt[0].y ) + tri.pt[2].x * ( tri.pt[0].y - tri.pt[1].y ) ) / 2.0f );
			std::cout << "area: " << area << std::endl;

			 std::cout << "x: " << tri.pt[0].x << std::endl;
			 std::cout << "0y: " << tri.pt[0].y << std::endl;
			 std::cout << "1y: " << tri.pt[1].y << std::endl;
			 std::cout << "2y: " << tri.pt[2].y << std::endl;
*/
			vec3d center{ 0, 0, 0 };
			bool lookingAtTriangle = tri.contains(center);
			if (lookingAtTriangle)
			{
				model.inFocus = true;
			}else{
				model.inFocus = false;
			}
			// if (model.inFocus) {
			// 	std::cout << "in focus!" << std::endl;
			// 	break;
			// }else {
			// 	std::cout << "not in focus!" << std::endl;
			// }

		}
		mtx.unlock();

		/*
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

				//perspective correction for texture vertices
				if (triProjected.p[0].w > 0) triProjected.t[0].u = triProjected.t[0].u / triProjected.p[0].w;
				if (triProjected.p[1].w > 0) triProjected.t[1].u = triProjected.t[1].u / triProjected.p[1].w;
				if (triProjected.p[2].w > 0) triProjected.t[2].u = triProjected.t[2].u / triProjected.p[2].w;

				if (triProjected.p[0].w > 0) triProjected.t[0].v = triProjected.t[0].v / triProjected.p[0].w;
				if (triProjected.p[1].w > 0) triProjected.t[1].v = triProjected.t[1].v / triProjected.p[1].w;
				if (triProjected.p[2].w > 0) triProjected.t[2].v = triProjected.t[2].v / triProjected.p[2].w;

				if (triProjected.p[0].w > 0) triProjected.t[0].w = 1.0f / triProjected.p[0].w;
				if (triProjected.p[1].w > 0) triProjected.t[1].w = 1.0f / triProjected.p[1].w;
				if (triProjected.p[2].w > 0) triProjected.t[2].w = 1.0f / triProjected.p[2].w;

				triProjected = triProjected.divByW();

				//carry luminance
				triProjected.luminance = triTranslated.luminance;

				newTrianglesToProject.push_back(triProjected);

			}

		}

		*/

	}
	//mtx.unlock();

	// mtx.lock();
	// trianglesToRaster = newTrianglesToProject;
	// mtx.unlock();

	return true;
}

std::unique_ptr<std::list<triangle>> Engine3D::clip(triangle& tri)
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

void Engine3D::move(float elapsedTime)
{
	float cameraSpeed = static_cast<float>(1.5 * elapsedTime);

	if (eventController != nullptr)
	{
		bool* keysPressed = eventController->getKeysPressed();
		int mouseDistanceX = eventController->getMouseDistanceX();
		int mouseDistanceY = eventController->getMouseDistanceY();
		float multiplierX = (float)mouseDistanceX * 5;
		float multiplierY = (float)mouseDistanceY * 5;

		if (keysPressed[SupportedKeys::W]) {
			camera = camera + forward;
			cameraPos += cameraSpeed * cameraFront;
		} else if (keysPressed[SupportedKeys::S]) {
			camera = camera - forward;
			cameraPos -= cameraSpeed * cameraFront;
		}

		if (keysPressed[SupportedKeys::A]) {
			right = forward * matCameraRotY90CCW;
			camera = camera + right;
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

		} else if (keysPressed[SupportedKeys::D]) {
			left = forward * matCameraRotY90CW;
			camera = camera + left;
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		}

		if (keysPressed[SupportedKeys::LEFT_ARROW] || keysPressed[SupportedKeys::MOUSE_LEFT]) {
			yaw -= (multiplierX * elapsedTime) > 0.0f ? multiplierX * elapsedTime : 0;
		} else if (keysPressed[SupportedKeys::RIGHT_ARROW] || keysPressed[SupportedKeys::MOUSE_RIGHT]) {
			yaw += (multiplierX * elapsedTime) > 0.0f ? multiplierX * elapsedTime : 0;
		}

		if (keysPressed[SupportedKeys::UP_ARROW] || keysPressed[SupportedKeys::MOUSE_UP]) {
			pitch += (multiplierY * elapsedTime) > 0.000001f ? multiplierY * elapsedTime : 0;
		} else if (keysPressed[SupportedKeys::DOWN_ARROW] || keysPressed[SupportedKeys::MOUSE_DOWN]) {
			pitch -= (multiplierY * elapsedTime) > 0.000001f ? multiplierY * elapsedTime : 0;
		}

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(front);
	}
}

void Engine3D::textureTriangle(triangle& tri) {

	int x1 = tri.p[0].x;
	int y1 = tri.p[0].y;
	float u1 = tri.t[0].u;
	float v1 = tri.t[0].v;
	float w1 = tri.t[0].w;
	int x2 = tri.p[1].x;
	int y2 = tri.p[1].y;
	float u2 = tri.t[1].u;
	float v2 = tri.t[1].v;
	float w2 = tri.t[1].w;
	int x3 = tri.p[2].x;
	int y3 = tri.p[2].y;
	float u3 = tri.t[2].u;
	float v3 = tri.t[2].v;
	float w3 = tri.t[2].w;

	if (y2 < y1)
	{
		std::swap(y1, y2);
		std::swap(x1, x2);
		std::swap(u1, u2);
		std::swap(v1, v2);
		std::swap(w1, w2);
	}

	if (y3 < y1)
	{
		std::swap(y1, y3);
		std::swap(x1, x3);
		std::swap(u1, u3);
		std::swap(v1, v3);
		std::swap(w1, w3);
	}

	if (y3 < y2)
	{
		std::swap(y2, y3);
		std::swap(x2, x3);
		std::swap(u2, u3);
		std::swap(v2, v3);
		std::swap(w2, w3);
	}

	int dy1 = y2 - y1;
	int dx1 = x2 - x1;
	float dv1 = v2 - v1;
	float du1 = u2 - u1;
	float dw1 = w2 - w1;

	int dy2 = y3 - y1;
	int dx2 = x3 - x1;
	float dv2 = v3 - v1;
	float du2 = u3 - u1;
	float dw2 = w3 - w1;

	float tex_u, tex_v, tex_w;

	float dax_step = 0, dbx_step = 0, du1_step = 0, dv1_step = 0, dw1_step = 0, du2_step = 0, dv2_step = 0, dw2_step = 0;

	if (dy1) dax_step = dx1 / (float)abs(dy1);
	if (dy2) dbx_step = dx2 / (float)abs(dy2);

	if (dy1) du1_step = du1 / (float)abs(dy1);
	if (dy2) dv1_step = dv1 / (float)abs(dy1);
	if (dy2) dw1_step = dw1 / (float)abs(dy1);

	if (dy2) du2_step = du2 / (float)abs(dy2);
	if (dy2) dv2_step = dv2 / (float)abs(dy2);
	if (dw2) dw2_step = dw2 / (float)abs(dy2);

	if (dy1)
	{
		for (int i = y1; i <= y2; i+=1)
		{
			int ax = x1 + (float)(i - y1) * dax_step;
			int bx = x1 + (float)(i - y1) * dbx_step;

			float tex_su = u1 + (float)(i - y1) * du1_step;
			float tex_sv = v1 + (float)(i - y1) * dv1_step;
			float tex_sw = w1 + (float)(i - y1) * dw1_step;

			float tex_eu = u1 + (float)(i - y1) * du2_step;
			float tex_ev = v1 + (float)(i - y1) * dv2_step;
			float tex_ew = w1 + (float)(i - y1) * dw2_step;

			if (ax > bx)
			{
				std::swap(ax, bx);
				std::swap(tex_su, tex_eu);
				std::swap(tex_sv, tex_ev);
				std::swap(tex_sw, tex_ew);
			}
			tex_u = tex_su;
			tex_v = tex_sv;
			tex_w = tex_sw;

			float tstep = 1.0f / ((float)(bx - ax));
			//std::cout << tstep << std::endl;
			float t = 0.0f;

			for (int j = ax; j < bx; j+=1)
			{
				tex_u = (1.0f - t) * tex_su + t * tex_eu;
				tex_v = (1.0f - t) * tex_sv + t * tex_ev;
				tex_w = (1.0f - t) * tex_sw + t * tex_ew;

				t += tstep;

				if (tex_w <= depthBuffer[i * width + j]) continue;
				else depthBuffer[i * width + j] = tex_w;

				vec2d tx;
				tx.u = tex_u;
				tx.v = tex_v;
				tx.w = tex_w;

				vec2d p;
				p.u = j;
				p.v = i;

				texturePoint tp;
				tp.t = tx;
				tp.p = p;

				tri.texturePoints.push_back(tp);
			}

		}

	}

	dy1 = y3 - y2;
	dx1 = x3 - x2;
	dv1 = v3 - v2;
	du1 = u3 - u2;
	dw1 = w3 - w2;

	if (dy1) dax_step = dx1 / (float)abs(dy1);
	if (dy2) dbx_step = dx2 / (float)abs(dy2);

	du1_step = 0, dv1_step = 0;
	if (dy1) du1_step = du1 / (float)abs(dy1);
	if (dy2) dv1_step = dv1 / (float)abs(dy1);
	if (dy2) dw1_step = dw1 / (float)abs(dy1);

	if (dy1)
	{
		for (int i = y2; i <= y3; i++)
		{
			int ax = x2 + (float)(i - y2) * dax_step;
			int bx = x1 + (float)(i - y1) * dbx_step;

			float tex_su = u2 + (float)(i - y2) * du1_step;
			float tex_sv = v2 + (float)(i - y2) * dv1_step;
			float tex_sw = w2 + (float)(i - y2) * dw1_step;

			float tex_eu = u1 + (float)(i - y1) * du2_step;
			float tex_ev = v1 + (float)(i - y1) * dv2_step;
			float tex_ew = w1 + (float)(i - y1) * dw2_step;

			if (ax > bx)
			{
				std::swap(ax, bx);
				std::swap(tex_su, tex_eu);
				std::swap(tex_sv, tex_ev);
				std::swap(tex_sw, tex_ew);
			}
			tex_u = tex_su;
			tex_v = tex_sv;
			tex_w = tex_sw;

			float tstep = 1.0f / ((float)(bx - ax));
			float t = 0.0f;

			for (int j = ax; j < bx; j++)
			{
				tex_u = (1.0f - t) * tex_su + t * tex_eu;
				tex_v = (1.0f - t) * tex_sv + t * tex_ev;
				tex_w = (1.0f - t) * tex_sw + t * tex_ew;

				t += tstep;

				if (tex_w <= depthBuffer[i * width + j]) continue;
				else depthBuffer[i * width + j] = tex_w;

				vec2d tx;
				tx.u = tex_u;
				tx.v = tex_v;
				tx.w = tex_w;

				vec2d p;
				p.u = j;
				p.v = i;

				texturePoint tp;
				tp.t = tx;
				tp.p = p;

				tri.texturePoints.push_back(tp);

			}

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

glm::mat4 Engine3D::getProjectionMatrix() const
{
	return projectionMatrix;
}

glm::vec3 Engine3D::getCameraPos() const
{
	return cameraPos;
}

glm::vec3 Engine3D::getCameraFront() const
{
	return cameraFront;
}

glm::vec3 Engine3D::getCameraUp() const
{
	return cameraUp;
}

glm::mat4 Engine3D::getViewMatrix() const
{
	return viewMatrix;
}

void Engine3D::clearDepthBuffer()
{
	//clear depth buffer
	for (int i = 0; i < width * height; i++)
		depthBuffer[i] = 0.0f;
}


