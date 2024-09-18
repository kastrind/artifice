#include "Engine3D.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Engine3D::Engine3D(
				   SDL_Window* gWindow,
				   int width, int height,
				   float near, float far,
				   float fov, float dof,
				   float collidingDistance,
				   float gravitationalPull,
				   float jumpSpeedFactor,
				   float cameraSpeedFactor,
				   UserMode userMode, EventController* ec
				   )
				   : gWindow(gWindow),
				   width(width), height(height),
				   near(near), far(far),
				   fov(fov), dof(dof),
				   collidingDistance(collidingDistance),
				   gravitationalPull(gravitationalPull),
				   jumpSpeedFactor(jumpSpeedFactor),
				   cameraSpeedFactor(cameraSpeedFactor),
				   userMode(userMode), eventController(ec)
{
	isActive = false;

	updateVerticesFlag = false;

	projectionMatrix = glm::perspective(glm::radians((float)fov), (float)width / (float)height, near, far);

	//camera
	cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);

	viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	if (userMode == UserMode::EDITOR)
	{
		this->gravitationalPull = 0.0f;
		originalCollidingDistance = this->collidingDistance;
		this->collidingDistance = -0.1f;
	}

	cubemapFaceIndexMap["right"] = 0;
	cubemapFaceIndexMap["left"] = 1;
	cubemapFaceIndexMap["top"] = 2;
	cubemapFaceIndexMap["bottom"] = 3;
	cubemapFaceIndexMap["back"] = 4;
	cubemapFaceIndexMap["front"] = 5;
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

	rendererThread = startRendering();

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

std::thread Engine3D::startRendering()
{
	//start thread
	std::thread t = std::thread(&Engine3D::renderingThread, this);

	return t;
}

void Engine3D::renderingThread()
{
	//create context
	printf( "Creating OpenGL context...\n" );
	gContext = SDL_GL_CreateContext( gWindow );
	if( gContext == NULL )
	{
		printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
		return;
	}
	else
	{
		//initialize GLEW
		printf( "Initializing GLEW...\n" );
		glewExperimental = GL_TRUE; 
		GLenum glewError = glewInit();
		if( glewError != GLEW_OK )
		{
			printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
			return;
		}
		//initialize OpenGL
		printf( "Initializing OpenGL shaders, arrays, buffers, textures, cubemaps...\n" );
		if( !initGL() )
		{
			printf( "Unable to initialize OpenGL!\n" );
			return;
		}
		//initialize ImGui
		if (!initUI() )
		{
			printf( "Unable to initialize ImGui!\n" );
			return;
		}
	}

	while (isActive)
	{
		render();
	}
}

bool Engine3D::initGL()
{
	bool success = true;

	printf( "Loading shader programs...\n" );
	if (!cubeMapShader.loadProgram("shaders/cubemap.glvs", "shaders/cubemap.glfs"))
	{
		printf( "Unable to load cubemap shader!\n" );
		success = false;
	}
	if (!skyBoxShader.loadProgram("shaders/skybox.glvs", "shaders/skybox.glfs"))
	{
		printf( "Unable to load skybox shader!\n" );
		success = false;
	}
	else if(!textureShader.loadProgram("shaders/texture.glvs", "shaders/texture.glfs"))
	{
		printf( "Unable to load cubemap shader!\n" );
		success = false;
	}
	else
	{
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthFunc(GL_LESS); 

		gCubeMapProgramID = cubeMapShader.getProgramID();
		gSkyBoxProgramID = skyBoxShader.getProgramID();
		gTextureProgramID = textureShader.getProgramID();
		
		//create VAOs
		glGenVertexArrays(1, &gVAO);
		glGenVertexArrays(1, &gCubeVAO);

		//create VBOs
		glGenBuffers( 1, &gVBO );
		glGenBuffers( 1, &gCubeVBO );

		//create IBO
		glGenBuffers( 1, &gIBO );
		glGenBuffers( 1, &gCubeIBO );

		//update buffers with the new vertices
		updateVertices();

		//initialize clear color
		glClearColor( 0.f, 0.f, 0.f, 1.f );

		//generates and binds textures
		loadTextures(textureIdsMap);

		for (std::pair<const std::string, GLuint>& entry : textureIdsMap )
		{
			textureNames.push_back(entry.first);
		}

		//generates and binds cubemap
		loadCubemaps(cubemapIdsMap);

		for (std::pair<const std::string, GLuint>& entry : cubemapIdsMap )
		{
			cubemapNames.push_back(entry.first);
		}
		
	}
	return success;
}

void Engine3D::loadTextures(std::map<std::string, GLuint>& textureIdsMap)
{
	std::string texturesPath = cfg.ASSETS_PATH + std::string("\\textures");
	std::string filename;
	for (const auto & entry : std::filesystem::directory_iterator(texturesPath))
	{
		if (entry.is_regular_file())
		{
			if (entry.path().filename().has_extension()) {
				filename = entry.path().filename().replace_extension("").string();
			} else {
				filename = entry.path().filename().string();
			}

			textureIdsMap[filename] = -1;
			//declare texture
			glGenTextures(1, &textureIdsMap[filename]);
			//bind texture
			glBindTexture(GL_TEXTURE_2D, textureIdsMap[filename]);

			//load image
			int width, height, nrChannels;
			unsigned char *data = stbi_load(entry.path().string().c_str(), &width, &height, &nrChannels, 0);
			// if first pixel is pure magenta, then texture is considered to have transparency
			bool hasTransparency = (int)data[0] == 255 && (int)data[1] == 0 && (int)data[2] == 255;
			textureTransparencyMap[filename] = hasTransparency;
			if (data)
			{
				//generate texture
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);
			} else {
				std::cout << "Failed to load texture " << filename << std::endl;
			}

			//free image memory
			stbi_image_free(data);

			//set the texture wrapping/filtering options (on the currently bound texture object)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			if (hasTransparency) { //to achieve transparency
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			} else { //filter texture
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
		}
	}
	//activate shader
	textureShader.bind();
	//set the uniforms
	glUniform1i(glGetUniformLocation(textureShader.getProgramID(), "frameIndex"), 0);
	textureShader.setInt("userMode", (int)cfg.USER_MODE);
	for (const auto& kv : textureIdsMap) {
		glUniform1i(glGetUniformLocation(textureShader.getProgramID(), std::string("texture" + std::to_string(kv.second)).c_str()), 0);
	}
}

void Engine3D::loadCubemaps(std::map<std::string, GLuint>& cubemapIdsMap)
{
	std::string cubemapsDirNames[2] = {std::string("cubemaps"), std::string("skyboxes")};
	std::string filename;
	std::string name;
	for (std::string cubemapsDirName : cubemapsDirNames) {
		std::string cubemapsPath = cfg.ASSETS_PATH + std::string("\\") + cubemapsDirName;
		for (const auto & entry : std::filesystem::directory_iterator(cubemapsPath))
		{
			if (entry.is_directory())
			{
				name = entry.path().filename().string();

				cubemapIdsMap[name] = -1;
				glGenTextures(1, &cubemapIdsMap[name]);
				glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapIdsMap[name]);

				for (const auto & face : std::filesystem::directory_iterator(entry))
				{
					if (face.is_regular_file())
					{
						if (face.path().filename().has_extension()) {
							filename = face.path().filename().replace_extension("").string();
						} else {
							filename = face.path().filename().string();
						}

						int width, height, nrChannels;
						int i = 0;
						if (cubemapFaceIndexMap[filename] >= 0)
						{
							unsigned char *data = stbi_load(face.path().string().c_str(), &width, &height, &nrChannels, 0);
							if (data)
							{
								glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubemapFaceIndexMap[filename], 
												0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
								);
								stbi_image_free(data);
							}
							else
							{
								std::cout << "Cubemap texture failed to load at path: " << face.path() << std::endl;
								stbi_image_free(data);
							}
						}
						glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
						glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
						glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
						cubeMapShader.bind();
						glUniform1i(glGetUniformLocation(cubeMapShader.getProgramID(), std::string("cubemap" + std::to_string(cubemapIdsMap[name])).c_str()), 0);
						cubeMapShader.unbind();
						skyBoxShader.bind();
						glUniform1i(glGetUniformLocation(skyBoxShader.getProgramID(), std::string("skybox" + std::to_string(cubemapIdsMap[name])).c_str()), 0);
						skyBoxShader.unbind();
					}
				}
			}
		}
	}
}

bool Engine3D::initUI()
{
	bool success = false;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	success = ImGui_ImplSDL2_InitForOpenGL(gWindow, gContext);
	if (!success) return success;
	success = ImGui_ImplOpenGL3_Init();
	return success;
}

void Engine3D::render()
{
	if (updateVerticesFlag) updateVertices();
	//if (finalCubeModelsToRender.size() == 0) return;

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	//clear color buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cubeMapShader.bind();
	cubeMapShader.setMat4("projection", getProjectionMatrix());
	cubeMapShader.setMat4("view", getViewMatrix());
	//lighting
	cubeMapShader.setVec3("lightPos", getLightPos());
	cubeMapShader.setVec3("viewPos", getCameraPos());
	cubeMapShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
	cubeMapShader.unbind();

	textureShader.bind();
	textureShader.setMat4("projection", getProjectionMatrix());
	textureShader.setMat4("view", getViewMatrix());
	textureShader.unbind();

	skyBoxShader.bind();
	skyBoxShader.setMat4("projection", getProjectionMatrix());
	skyBoxShader.setMat4("view", getViewMatrixNoTranslation());
	skyBoxShader.unbind();

	// //lighting
	// textureShader->setVec3("lightPos", getLightPos());
	// textureShader->setVec3("viewPos", getCameraPos());
	// textureShader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);

	//render active skybox
	skyBoxShader.bind();
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gCubeIBO);
	glBindVertexArray(gCubeVAO);
	glActiveTexture(GL_TEXTURE0);
	if (finalSkyBoxToRender != nullptr)
	{
		cubeModel& cm = *finalSkyBoxToRender;
		cm.render(&skyBoxShader, cubemapIdsMap[cm.texture]);
	}
	glBindVertexArray(0);
	skyBoxShader.unbind();

	//render cubemaps
	cubeMapShader.bind();
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gCubeIBO);
	glBindVertexArray(gCubeVAO);
	glActiveTexture(GL_TEXTURE0);
	mtx.lock();
	for (auto itr = finalCubeModelsToRender.begin(); itr != finalCubeModelsToRender.end(); itr++)
	{
		if (!(*itr)) { std::cout << "nullptr!" << std::endl; continue; }
		
		cubeModel& cm = dynamic_cast<cubeModel &>(*(*itr));
				
		if (cm.removeFlag) continue;

		if (cm.isSkyBox) {
			if (cm.isActiveSkyBox)
			{
				finalSkyBoxToRender = std::make_shared<cubeModel>(cm);
			}
			continue;
		}

		cm.render(&cubeMapShader, cubemapIdsMap[cm.texture]);
	}
	glBindVertexArray(0);
	cubeMapShader.unbind();

	//render other models
	textureShader.bind();
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
	glBindVertexArray(gVAO);
	glActiveTexture(GL_TEXTURE0);
	for (auto itr = finalModelsToRender.begin(); itr != finalModelsToRender.end(); itr++)
	{
		if (!(*itr)) { std::cout << "nullptr!" << std::endl; continue; }

		model& model = *(*itr);

		if (model.removeFlag) continue;

		if (model.texture.length() && textureTransparencyMap[model.texture]) {
			finalTransparentModelsToRender.insert(*itr);
			continue;	
		}

		model.render(&textureShader, textureIdsMap[model.texture]);
	}

	for (auto itr = finalTransparentModelsToRender.begin(); itr != finalTransparentModelsToRender.end(); itr++)
	{
		if (!(*itr)) { std::cout << "nullptr!" << std::endl; continue; }

		model& model = *(*itr);
		model.render(&textureShader, textureIdsMap[model.texture]);
	}
	finalTransparentModelsToRender.clear();

	glBindVertexArray(0);
	textureShader.unbind();

	mtx.unlock();

	renderUI();

	//update screen
	SDL_GL_SwapWindow( gWindow );
}

void Engine3D::updateVertices()
{
	std::cout << "Updating vertices" << std::endl;
	std::vector<GLfloat> vertexData;
	std::vector<GLuint> indexData;
	GLuint indexCounter = 0;
	std::vector<GLfloat> cubeVertexData;
	std::vector<GLuint> cubeIndexData;
	GLuint cubeIndexCounter = 0;

	std::vector<GLfloat>* vdp = &vertexData;

	//populate vertex vectors with triangle vertex information for each model
	for (auto &ptrModel : ptrModelsToRender)
	{
		if (!ptrModel) continue;
		model& model = *ptrModel;

		if (model.removeFlag) continue;

		vdp = model.modelMesh.shape == shapetype::CUBE ? &cubeVertexData : &vertexData;

		for (auto &tri : model.modelMesh.tris)
		{
			glm::vec3 line1 = tri.p[1] - tri.p[0];
			glm::vec3 line2 = tri.p[2] - tri.p[0];
			glm::vec3 normal = glm::normalize(glm::cross(line1, line2));

			for (int i = 0; i < 3; i++)
			{
				vdp->push_back(tri.p[i].x);
				vdp->push_back(tri.p[i].y);
				vdp->push_back(tri.p[i].z);
				vdp->push_back(normal.x);
				vdp->push_back(normal.y);
				vdp->push_back(normal.z);
				vdp->push_back((float)tri.R/255.0f);
				vdp->push_back((float)tri.G/255.0f);
				vdp->push_back((float)tri.B/255.0f);
				vdp->push_back(tri.t[i].x);
				vdp->push_back(tri.t[i].y);
				if (model.modelMesh.shape != shapetype::CUBE)
				{
					indexData.push_back(indexCounter++);
				}
				else
				{
					cubeIndexData.push_back(cubeIndexCounter++);
				}
			}
		}
	}

	// std::cout << "  vertex data size: " << vertexData.size() << std::endl;
	// std::cout << "  index data size: " << indexData.size() << std::endl;
	// std::cout << "  cube vertex data size: " << cubeVertexData.size() << std::endl;
	// std::cout << "  cube index data size: " << cubeIndexData.size() << std::endl;

	//update VBO
	glBindVertexArray(gVAO);
	glBindBuffer( GL_ARRAY_BUFFER, gVBO );
	glBufferData( GL_ARRAY_BUFFER, vertexData.size() * sizeof(GL_FLOAT), vertexData.data(), GL_STATIC_DRAW );

	//position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(0);
	//normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	//color attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)(6 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);
	//texture coord attribute
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)(9 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(3);

	//update IBO
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GL_UNSIGNED_INT), indexData.data(), GL_STATIC_DRAW );

	//update cubeVBO
	glBindVertexArray(gCubeVAO);
	glBindBuffer( GL_ARRAY_BUFFER, gCubeVBO );
	glBufferData( GL_ARRAY_BUFFER, cubeVertexData.size() * sizeof(GL_FLOAT), cubeVertexData.data(), GL_STATIC_DRAW );

	//position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(0);
	//normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	//color attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)(6 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);
	//texture coord attribute
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)(9 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(3);

	//update cubeIBO
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gCubeIBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, cubeIndexData.size() * sizeof(GL_UNSIGNED_INT), cubeIndexData.data(), GL_STATIC_DRAW );

	updateVerticesFlag = false;

	std::cout << "Updated vertices" << std::endl;
}

void Engine3D::renderUI()
{
	//(after event loop)
	//start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	//ImGui::ShowDemoWindow(); // Show demo window! :)
	//display a piece of text directly on the screen
	ImGui::SetNextWindowPos(ImVec2(width/2, height/2));
	ImGui::SetNextWindowSize(ImVec2(0, 0));
	ImGui::Begin("InvisibleWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs);
	ImGui::Text(".");
	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool Engine3D::onUserCreate()
{
	return true;
}

bool Engine3D::onUserUpdate(float elapsedTime)
{
	mtx.lock();

	captureInput();

	glm::vec3 prevCameraPos = cameraPos;
	glm::vec3 center{ 0, 0, 0 };
	float modelDistance = dof;
	float maxModelDist = 0.0f;
	glm::vec4 collidingTriPts[3];

	move(elapsedTime);

	if (modelsInFocus.size()) { prevModelInFocus = *(modelsInFocus.begin()); }

	collides = false;
	collidesFront = false;
	collidesBack = false;
	collidesRight = false;
	collidesLeft = false;

	canSlide = false;
	hasLanded = false;

	modelsInFocus.clear();

	projectionMatrix = glm::perspective(glm::radians((float)fov), (float)width / (float)height, near, far);
	viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	mtx.unlock();

	//for each model to render
	for (auto &ptrModel : ptrModelsToRender)
	{
		if (!ptrModel) continue;
		model& model = *ptrModel;

		mtx.lock();
		glm::mat4 modelMatrix = glm::mat4(1.0f); //make sure to initialize matrix to identity matrix first
		modelMatrix = glm::translate(modelMatrix, model.position);
		modelMatrix = modelMatrix * model.rotationMatrix;
		model.modelMatrix = modelMatrix;

		model.inFocus = false;
		bool checkAgainForFocus = true;
		model.isInFOV = false;
		bool checkAgainForFOV = true;
		float minX = 1.0f, maxX = -1.0f, minY = 1.0f, maxY = -1.0f, minZ = 100000.0f, maxZ = -100000.0f;

		//if cube is skybox, then do not process further
		if (model.modelMesh.shape == shapetype::CUBE)
		{
			cubeModel& cube = dynamic_cast<cubeModel &>(model);
			if (cube.isSkyBox)
			{
				model.isInDOF = true;
				model.isInFOV = true;
				model.isCovered = false;
				mtx.unlock();
				continue;
			}
		}
		mtx.unlock();

		modelDistance = dof;

		for (auto tri : model.modelMesh.tris)
		{
			glm::vec4 pt[3];
			pt[0] = tri.p[0];
			pt[1] = tri.p[1];
			pt[2] = tri.p[2];

			//model transformation
			pt[0] = modelMatrix * pt[0];
			pt[1] = modelMatrix * pt[1];
			pt[2] = modelMatrix * pt[2];

			//finds distance from model
			glm::vec4 avgP = ( (pt[0]) + (pt[1]) + (pt[2]) ) / 3.0f;
			float avgDist = glm::distance(cameraPos, glm::vec3(avgP));
			model.distance = avgDist;
			float dist1 = glm::distance(cameraPos, glm::vec3(pt[0]));
			float dist2 = glm::distance(cameraPos, glm::vec3(pt[1]));
			float dist3 = glm::distance(cameraPos, glm::vec3(pt[2]));
			float maxDist = std::max(dist1, std::max(dist2, dist3));
			//std::cout << "maxDist: " << maxDist << std::endl;
			if (avgDist < modelDistance) {
				modelDistance =  avgDist;
				maxModelDist = maxDist;
				collidingTriPts[0] = pt[0];
				collidingTriPts[1] = pt[1];
				collidingTriPts[2] = pt[2];
			}
			//std::cout << "modelDistance: " << modelDistance << std::endl;

			//camera/view transformation
			pt[0] = projectionMatrix * viewMatrix * pt[0];
			pt[1] = projectionMatrix * viewMatrix * pt[1];
			pt[2] = projectionMatrix * viewMatrix * pt[2];

			mtx.lock();
			tri.p[0] = pt[0];
			tri.p[1] = pt[1];
			tri.p[2] = pt[2];
			mtx.unlock();

			//determines whether looking at the current model
			if (checkAgainForFocus && tri.contains(glm::vec4(center, 1.0f)))
			{
				model.inFocus = true;
				mtx.lock();
				modelsInFocus.insert(ptrModel);
				mtx.unlock();
				checkAgainForFocus = false;
			}

			//calculate bounding box
			minX = std::min(minX, std::min(std::min(pt[0].x/pt[0].w, pt[1].x/pt[1].w), pt[2].x/pt[2].w));
			minY = std::min(minY, std::min(std::min(pt[0].y/pt[0].w, pt[1].y/pt[1].w), pt[2].y/pt[2].w));
			minZ = std::min(minZ, std::min(std::min(pt[0].z, pt[1].z), pt[2].z));
			maxX = std::max(maxX, std::max(std::max(pt[0].x/pt[0].w, pt[1].x/pt[1].w), pt[2].x/pt[2].w));
			maxY = std::max(maxY, std::max(std::max(pt[0].y/pt[0].w, pt[1].y/pt[1].w), pt[2].y/pt[2].w));
			maxZ = std::max(maxZ, std::max(std::max(pt[0].z, pt[1].z), pt[2].z));
		}

		//assign bounding box, to check for coverage later
		minX = (std::max(-1.0f, minX) + 1) / 2.0f;
		minY = (std::max(-1.0f, minY) + 1) / 2.0f;
		maxX = (std::min(1.0f, maxX) + 1) / 2.0f;
		maxY = (std::min(1.0f, maxY) + 1) / 2.0f;
		//std::cout << "X: " << minX << " - "<< maxX << ", Y: " << minY << " - " << maxY << ", Z: " << minZ << " - " << maxZ << std::endl;
		boundingbox bbox = { minX, maxX, minY, maxY, minZ, maxZ };
		model.bbox = bbox;

		//mark out-of-FOV models to avoid needless rendering
		model.isInFOV = ((model.bbox.minX > 0 && model.bbox.minX < 1) || (model.bbox.maxX > 0 && model.bbox.maxX < 1)) && ((model.bbox.minY > 0 && model.bbox.minY < 1) || (model.bbox.maxY > 0 && model.bbox.maxY < 1));

		//mark out-of-DOF models to avoid needless rendering
		model.isInDOF = modelDistance < dof;

		//detect if colliding with the model
		if (model.isSolid && modelDistance <= collidingDistance * 1.5f)
		{
			//std::cout << "modelDistance: " << modelDistance << std::endl;

			//get the triangle normal
			glm::vec3 line1 = collidingTriPts[1] - collidingTriPts[0];
			glm::vec3 line2 = collidingTriPts[2] - collidingTriPts[0];
			glm::vec3 normal = glm::normalize(glm::cross(line1, line2));
			// std::cout << normal.x << ", " << normal.y << ", " << normal.z << std::endl;

			//based on dp and normal, determine if able to slide and the desired motion
			float dpFront = glm::dot(cameraFront, normal);
			float absDP = std::abs(dpFront);
			float dpRight = glm::dot(cameraRight, normal);
			float dpLeft = glm::dot(-cameraRight, normal);
			float dpBack = glm::dot(-cameraFront, normal);

			//normal is up means collision with floor
			if (collidingTriPts[0].y <= cameraPos.y && normal.y < normal.z && normal.y < normal.x) {
				hasLanded = true;
			//else collision with wall
			}else if (modelDistance < collidingDistance && maxModelDist < collidingDistance * 1.5f) {
				canSlide = absDP < 0.8f && absDP > 0.0f;
				//std::cout << "dpFront: " << dpFront << std::endl;
				//std::cout << "maxModelDist: " << maxModelDist << std::endl;
				//std::cout << "modelDistance: " << modelDistance << std::endl;
				collides = true;
				if (!collidesFront) collidesFront = dpFront < dpRight && dpFront < dpLeft && dpFront < dpBack;
				if (!collidesBack)  collidesBack  = dpBack < dpRight && dpBack < dpLeft && dpBack < dpFront;
				if (!collidesRight) collidesRight = dpRight < dpLeft && dpRight < dpFront && dpRight < dpBack;
				if (!collidesLeft)  collidesLeft  = dpLeft < dpRight && dpLeft < dpFront && dpLeft < dpBack;
				//std::cout << "front: " << collidesFront << ", right: " << collidesRight << ", left: " << collidesLeft << ", back: " << collidesBack << std::endl;
				glm::vec3 undesiredMotion = normal * dpFront;
				desiredMotion = cameraFront - undesiredMotion;
				desiredMotion = glm::normalize(desiredMotion);
				//std::cout << desiredMotion.x << ", " << desiredMotion.z << std::endl;
			}
		}

		if (model.isSolid && modelDistance < collidingDistance * 0.5f)
		{
			float camY = cameraPos.y; // to be able to fall if collided in the air
			cameraPos = prevCameraPos;
			cameraPos.y = camY;
		}
	}
	lightPos = cameraFront;

	
	//mark covered models to avoid needless rendering
	//disabled to achieve transparency
	//mtx.lock();
	//markCoveredModels();
	//mtx.unlock();

	mtx.lock();
	for (auto &ptrModel : ptrModelsToRender)
	{
		if (!ptrModel) continue;

		//move model
		if (ptrModel->speed > 0) {
			//ptrModel->position += ptrModel->speed * ptrModel->front * elapsedTime;
			ptrModel->position += ptrModel->speed * glm::normalize(cameraPos - ptrModel->position) * elapsedTime;
		}

		//render if model is near, or not covered, in DOF and in FOV
		if (ptrModel->distance <= 2.0f || (!ptrModel->isCovered && ptrModel->isInDOF && ptrModel->isInFOV))
		{
			if (ptrModel->modelMesh.shape == shapetype::CUBE) finalCubeModelsToRender.insert(ptrModel);
			else finalModelsToRender.insert(ptrModel);
		}else
		{
			if (ptrModel->modelMesh.shape == shapetype::CUBE) finalCubeModelsToRender.erase(ptrModel);
			else finalModelsToRender.erase(ptrModel);
		}

	}
	//std::cout << "final models to render: " << finalCubeModelsToRender.size() << std::endl;
	mtx.unlock();

	edit(elapsedTime);

	//std::cout << "collides? " << collides << ", canSlide? " << canSlide << ", hasLanded? " << hasLanded << std::endl;
	return true;
}

void Engine3D::captureInput()
{
	//listen for input
	std::memcpy(prevKeysPressed, keysPressed, SupportedKeys::ALL_KEYS * sizeof(bool));
	std::memcpy(keysPressed, eventController->getKeysPressed(), SupportedKeys::ALL_KEYS * sizeof(bool));
}

void Engine3D::markCoveredModels()
{
	for (auto &ptrCube1 : ptrModelsToRender)
	{
		if (!ptrCube1) continue;
		//if (!ptrCube1->isInDOF || !ptrCube1->isInFOV) continue;
		model& model1 = *ptrCube1;
		for (auto &ptrCube2 : ptrModelsToRender)
		{
			if (!ptrCube2) continue;
			//if (!ptrCube2->isInDOF || !ptrCube2->isInFOV) continue;
			model& model2 = *ptrCube2;
			model1.isCovered = model2.bbox.minX > 0 && model2.bbox.maxX < 1 &&
							   model2.bbox.minY > 0 && model2.bbox.maxY < 1 &&
							   model1.bbox.minX > model2.bbox.minX &&
							   model1.bbox.maxX < model2.bbox.maxX &&
							   model1.bbox.minY > model2.bbox.minY &&
							   model1.bbox.maxY < model2.bbox.maxY &&
							   model1.bbox.maxZ > model2.bbox.minZ;

			if (model1.isCovered) break;

		}
		//std::cout << "model " << model1.id << " is covered? " << model1.isCovered << std::endl;
	}
}

void Engine3D::move(float elapsedTime)
{
	float cameraSpeed = static_cast<float>(cameraSpeedFactor * elapsedTime);

	cameraPos += !hasLanded ? gravitationalPull * cameraSpeed * glm::vec3(0, -1, 0) : glm::vec3(0, 0, 0);

	if (eventController != nullptr)
	{
		//bool* keysPressed = eventController->getKeysPressed();

		int mouseDistanceX = eventController->getMouseDistanceX();
		int mouseDistanceY = eventController->getMouseDistanceY();
		//std::cout << "mouseDistX: " << mouseDistanceX << ", mouseDistY: " << mouseDistanceY << std::endl;
		//std::cout << "left: " << keysPressed[SupportedKeys::MOUSE_LEFT] << ", right: " << keysPressed[SupportedKeys::MOUSE_RIGHT] << ", up: " << keysPressed[SupportedKeys::MOUSE_UP] << ", down: " << keysPressed[SupportedKeys::MOUSE_DOWN] << std::endl;
		float multiplierX = (float)mouseDistanceX * 5;
		float multiplierY = (float)mouseDistanceY * 5;

		//WSAD camera movement here
		if (keysPressed[SupportedKeys::W] && hasLanded && collides && canSlide) {
			desiredMotion.y = 0;
			cameraPos += 0.5f * cameraSpeed * desiredMotion;

		} else if (keysPressed[SupportedKeys::W] && hasLanded && !collides) {
			cameraFront.y = 0;
			cameraPos += cameraSpeed * cameraFront;

		} else if (keysPressed[SupportedKeys::W] && (/*!hasLanded ||*/ userMode == UserMode::EDITOR)) {
			cameraPos += cameraSpeed * cameraFront;

		} else if (keysPressed[SupportedKeys::S] && hasLanded && collides && canSlide) {
			desiredMotion.y = 0;
			cameraPos -= 0.5f * cameraSpeed * desiredMotion;

		} else if (keysPressed[SupportedKeys::S] && hasLanded && !collides) {
			cameraFront.y = 0;
			cameraPos -= cameraSpeed * cameraFront;

		} else if (keysPressed[SupportedKeys::S] && (/*!hasLanded ||*/ userMode == UserMode::EDITOR)) {
			cameraPos -= cameraSpeed * cameraFront;
		}
		if (keysPressed[SupportedKeys::A] && hasLanded && collides && !collidesLeft) {
			desiredMotion.y = 0;
			bool desiresRight = glm::dot(cameraRight, desiredMotion) < 0;
			if (desiresRight) cameraPos += 0.5f * cameraSpeed * desiredMotion;
			else cameraPos -= 0.5f * cameraSpeed * desiredMotion;
		
		}else if (keysPressed[SupportedKeys::A] && (hasLanded && !collides || userMode == UserMode::EDITOR)) {
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		
		} else if (keysPressed[SupportedKeys::D] && hasLanded && collides && !collidesRight) {
			desiredMotion.y = 0;
			bool desiresRight = glm::dot(cameraRight, desiredMotion) < 0;
			if (desiresRight) cameraPos -= 0.5f * cameraSpeed * desiredMotion;
			else cameraPos += 0.5f * cameraSpeed * desiredMotion;

		} else if (keysPressed[SupportedKeys::D] && (hasLanded && !collides || userMode == UserMode::EDITOR)) {
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		}

		//jumping
		if (keysPressed[SupportedKeys::SPACE] && hasLanded) {
			jumpSpeed = gravitationalPull * jumpSpeedFactor;
			cameraPos.y += elapsedTime * jumpSpeed;
			if (keysPressed[SupportedKeys::W] && !collides) { //if jumped while moving forward
				cameraFrontOnJump = cameraFront;
				cameraFrontOnJump.y = 0;
			}else {
				cameraFrontOnJump = glm::vec3(0, 0, 0);
			}
		}
		//keep elevating if in the air and there is jump speed
		if (!hasLanded && jumpSpeed > 0) {
			cameraPos.y += elapsedTime * jumpSpeed;
			jumpSpeed -= jumpSpeed * elapsedTime;
			if (!collides) { //if not colliding, move forward too
				cameraFront.y = 0;
				cameraPos += 0.5f * cameraSpeed * cameraFrontOnJump;
			}
		}
		//std::cout << "jumpSpeed" << jumpSpeed << std::endl;

		//if in editor mode, arrows control track camera movement to the sides, above or below camera position
		if (userMode == UserMode::EDITOR)
		{
			if (keysPressed[SupportedKeys::LEFT_ARROW]) {
				cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			} else if (keysPressed[SupportedKeys::RIGHT_ARROW]) {
				cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			}
			if (keysPressed[SupportedKeys::UP_ARROW]) {
				cameraPos -= glm::normalize(glm::cross(cameraFront, cameraRight)) * cameraSpeed;
			} else if (keysPressed[SupportedKeys::DOWN_ARROW]) {
				cameraPos += glm::normalize(glm::cross(cameraFront, cameraRight)) * cameraSpeed;
			}
		}

		//mouse motion
		if (keysPressed[SupportedKeys::MOUSE_LEFT]) {
			yaw -= multiplierX * elapsedTime;
		} else if (keysPressed[SupportedKeys::MOUSE_RIGHT]) {
			yaw += multiplierX * elapsedTime;
		}

		if (keysPressed[SupportedKeys::MOUSE_UP]) {
			pitch += multiplierY * elapsedTime;
		} else if (keysPressed[SupportedKeys::MOUSE_DOWN]) {
			pitch -= multiplierY * elapsedTime;
		}

		//make sure that when pitch is out of bounds, screen doesn't get flipped
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		//make sure yaw does not become too large
		if (yaw > 360.0f)
			yaw -= 360.0f;
		if (yaw < 0.0f)
			yaw += 360.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(front);

		cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
	}
}

bool Engine3D::onUserDestroy()
{
	std::cout << "Destroying Engine3D..." << std::endl;

	printf("Stopping rendering thread...\n");

	rendererThread.join();

	printf("Unbinding and deleting shader programs...\n");

	//unbind program - deactivate shader
	cubeMapShader.unbind();
	textureShader.unbind();
	skyBoxShader.unbind();

	//deallocate programs
	cubeMapShader.freeProgram();
	textureShader.freeProgram();
	skyBoxShader.freeProgram();

	printf("Shutting down ImGui...\n");
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	return true;
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

glm::vec3 Engine3D::getLightPos() const
{
	return lightPos;
}

glm::mat4 Engine3D::getViewMatrix() const
{
	return viewMatrix;
}

glm::mat4 Engine3D::getViewMatrixNoTranslation() const
{
	glm::mat4 viewWithoutTranslation = glm::mat4(glm::mat3(viewMatrix)); 
	return viewWithoutTranslation;
}

void Engine3D::setLevel(Level* level)
{
	for (auto &ptrModel : level->models)
	{
		model& m = *ptrModel;

		if (m.modelMesh.shape == shapetype::CUBE)
		{
			cubeModel& cm = dynamic_cast<cubeModel &>(m);
			ptrModelsToRender.push_back(std::make_shared<cubeModel>(cm));
		}
		else
		{
			ptrModelsToRender.push_back(std::make_shared<model>(m));
		}
	}
	modelPointsCnt = level->modelPointsCnt;
	cubePointsCnt = level->cubePointsCnt;
	this->cameraPos = level->playerPosition;
	this->level = std::make_shared<Level>(*level);
}