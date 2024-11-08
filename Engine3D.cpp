#include "Engine3D.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Engine3D::Engine3D(
					SDL_Window* gWindow, Camera cam,
					int width, int height,
					float near, float far,
					float fov, float dof,
					float collidingDistanceH,
					float collidingDistanceV,
					float gravitationalPull,
					float jumpSpeedFactor,
					float personSpeedFactor,
					UserMode userMode, EventController* ec
					)
					: gWindow(gWindow), camera(cam),
					width(width), height(height),
					near(near), far(far),
					fov(fov), dof(dof),
					collidingDistanceH(collidingDistanceH),
					collidingDistanceV(collidingDistanceV),
					gravitationalPull(gravitationalPull),
					jumpSpeedFactor(jumpSpeedFactor),
					personSpeedFactor(personSpeedFactor),
					userMode(userMode), eventController(ec)
{
	isActive = false;

	updateVerticesFlag = false;

	projectionMatrix = glm::perspective(glm::radians((float)fov), (float)width / (float)height, near, far);

	if (userMode == UserMode::EDITOR)
	{
		this->gravitationalPull = 0.0f;
		originalCollidingDistanceH = this->collidingDistanceH;
		this->collidingDistanceH = -0.1f;
		originalCollidingDistanceV = this->collidingDistanceV;
		this->collidingDistanceV = -0.1f;
		camera = Camera(camerapositionmode::ATTACHED_TO_PERSON, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	//camera
	if (camera.positionMode == camerapositionmode::FIXED)
	{
		setCameraPos(camera.position);
		setCameraFront(camera.front);
		setCameraRight(glm::vec3(1.0f, 0.0f, 0.0f));
		setCameraUp(glm::vec3(0.0f, 1.0f, 0.0f));
	} else if (camera.positionMode == camerapositionmode::ATTACHED_TO_PERSON)
	{
		setCameraOffset(camera.offset);
	}
	setPersonPos(glm::vec3(0.0f, 0.0f, 0.0f));
	setPersonFront(glm::vec3(0.0f, 0.0f, -1.0f));
	setPersonRight(glm::vec3(1.0f, 0.0f, 0.0f));
	setPersonUp(glm::vec3(0.0f, 1.0f, 0.0f));

	viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

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
		// if (!initUI() )
		// {
		// 	printf( "Unable to initialize ImGui!\n" );
		// 	return;
		// }
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
	if(!geometryShader.loadProgram("shaders/geometry.glvs", "shaders/geometry.glfs"))
	{
		printf( "Unable to load geometry shader!\n" );
		success = false;
	}
	else if(!geometryCubemapShader.loadProgram("shaders/geometryCubemap.glvs", "shaders/geometryCubemap.glfs"))
	{
		printf( "Unable to load geometry cubemap shader!\n" );
		success = false;
	}
	else if(!geometrySkyboxShader.loadProgram("shaders/geometrySkybox.glvs", "shaders/geometrySkybox.glfs"))
	{
		printf( "Unable to load geometry skybox shader!\n" );
		success = false;
	}
	else if(!lightingShader.loadProgram("shaders/lighting.glvs", "shaders/lighting.glfs"))
	{
		printf( "Unable to load lighting shader!\n" );
		success = false;
	}
	else if(!postProcShader.loadProgram("shaders/postproc.glvs", "shaders/postproc.glfs"))
	{
		printf( "Unable to load post-processing shader!\n" );
		success = false;
	}
	else
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glFrontFace(GL_CCW);

		gGeometryProgramID = geometryShader.getProgramID();
		gGeometryCubemapProgramID = geometryCubemapShader.getProgramID();
		gGeometrySkyboxProgramID = geometrySkyboxShader.getProgramID();
		gLightingProgramID = lightingShader.getProgramID();
		gPostProcProgramID = postProcShader.getProgramID();

		geometryShader.bind();
		geometryShader.setInt("userMode", (int)cfg.USER_MODE);
		geometryShader.unbind();

		geometryCubemapShader.bind();
		geometryCubemapShader.setInt("userMode", (int)cfg.USER_MODE);
		geometryCubemapShader.unbind();

		lightingShader.bind();
		lightingShader.setBool("phongLighting", cfg.PHONG_LIGHTING);
		lightingShader.unbind();

		postProcShader.bind();
		postProcShader.setInt("SCREEN_WIDTH", cfg.SCREEN_WIDTH);
		postProcShader.setInt("SCREEN_HEIGHT", cfg.SCREEN_HEIGHT);
		postProcShader.setBool("isFXAAOn", cfg.FXAA);
		postProcShader.unbind();
		
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

		if (cfg.MSAA && cfg.MSAA_SAMPLES > 1) {
			//configure MSAA framebuffer
			glGenFramebuffers(1, &gBOMS);
			glBindFramebuffer(GL_FRAMEBUFFER, gBOMS);

			//multisampled position color buffer
			glGenTextures(1, &gPositionMS);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gPositionMS);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, cfg.MSAA_SAMPLES, GL_RGBA16F, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, GL_TRUE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, gPositionMS, 0);

			//multisampled normal color buffer
			glGenTextures(1, &gNormalMS);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gNormalMS);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, cfg.MSAA_SAMPLES, GL_RGBA16F, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, GL_TRUE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, gNormalMS, 0);

			//multisampled albedo color buffer
			glGenTextures(1, &gAlbedoMS);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gAlbedoMS);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, cfg.MSAA_SAMPLES, GL_RGBA, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, GL_TRUE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D_MULTISAMPLE, gAlbedoMS, 0);

			//multisampled lightmap color buffer
			glGenTextures(1, &gLightmapMS);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gLightmapMS);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, cfg.MSAA_SAMPLES, GL_RGBA, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, GL_TRUE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D_MULTISAMPLE, gLightmapMS, 0);

			//multisampled view dir color buffer
			glGenTextures(1, &gViewDirMS);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gViewDirMS);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, cfg.MSAA_SAMPLES, GL_RGBA16F, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, GL_TRUE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D_MULTISAMPLE, gViewDirMS, 0);

			//tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
			unsigned int attachmentsMS[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
			glDrawBuffers(5, attachmentsMS);

			//create and attach multisampled depth buffer (renderbuffer)
			glGenRenderbuffers(1, &depthRenderBOMS);
			glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBOMS);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, cfg.MSAA_SAMPLES, GL_DEPTH_COMPONENT, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBOMS);

			//finally check if framebuffer is complete
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "MSAA Framebuffer not complete!" << std::endl;

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		//configure G-Buffer for deferred rendering
		glGenFramebuffers(1, &gBO);
		glBindFramebuffer(GL_FRAMEBUFFER, gBO);

		//position color buffer
		glGenTextures(1, &gPosition);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

		//normal color buffer
		glGenTextures(1, &gNormal);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

		//albedo color buffer
		glGenTextures(1, &gAlbedo);
		glBindTexture(GL_TEXTURE_2D, gAlbedo);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

		//lightmap color buffer
		glGenTextures(1, &gLightmap);
		glBindTexture(GL_TEXTURE_2D, gLightmap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gLightmap, 0);

		//view dir color buffer
		glGenTextures(1, &gViewDir);
		glBindTexture(GL_TEXTURE_2D, gViewDir);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gViewDir, 0);

		//tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
		unsigned int attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
		glDrawBuffers(5, attachments);

		//create and attach depth buffer (renderbuffer)
		glGenRenderbuffers(1, &depthRenderBO);
		glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBO);

		//finally check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "G Framebuffer not complete!" << std::endl;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//configure lighting color buffer for post-processing
		glGenFramebuffers(1, &lightingBO);
		glBindFramebuffer(GL_FRAMEBUFFER, lightingBO);

		//screen texture color buffer
		glGenTextures(1, &screenTexture);
		glBindTexture(GL_TEXTURE_2D, screenTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);

		//tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
		unsigned int attachmentsLighting[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, attachmentsLighting);

		//finally check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Lighting framebuffer not complete!" << std::endl;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//initialize clear color
		glClearColor( 0.f, 0.f, 0.f, 1.f );

		//generates and binds textures, lightmaps, normalmaps and parallax maps (displacementmaps)
		loadTextures(textureIdsMap, lightmapIdsMap, normalmapIdsMap, displacementmapIdsMap);

		for (std::pair<const std::string, GLuint>& entry : textureIdsMap )
		{
			textureNames.push_back(entry.first);
		}

		//generates and binds cubemaps, skyboxes, cube lightmaps, cube normalmaps and cube parallaxmaps (displacementmaps)
		loadCubemaps(cubemapIdsMap, cubeLightmapIdsMap, cubeNormalmapIdsMap, cubeDisplacementmapIdsMap);

		for (std::pair<const std::string, GLuint>& entry : cubemapIdsMap )
		{
			cubemapNames.push_back(entry.first);
		}
		
	}
	return success;
}

void Engine3D::loadTextures(std::map<std::string, GLuint>& textureIdsMap, std::map<std::string, GLuint>& lightmapIdsMap, std::map<std::string, GLuint>& normalmapIdsMap, std::map<std::string, GLuint>& displacementmapIdsMap)
{
	std::string textureDirNames[4] = {std::string("textures"), std::string("lightmaps"), std::string("normalmaps"), std::string("displacementmaps")};
	std::string filename;
	unsigned short i = 0;
	std::map<std::string, GLuint>* idsMap;
	for (std::string textureDirName : textureDirNames) {
		if (++i==2)    { if (!cfg.LIGHT_MAPPING)        { continue; } idsMap = &lightmapIdsMap; }
		else if (i==3) { if (!cfg.NORMAL_MAPPING)       { continue; } idsMap = &normalmapIdsMap; }
		else if (i==4) { if (!cfg.DISPLACEMENT_MAPPING) { continue; } idsMap = &displacementmapIdsMap; }
		else { idsMap = &textureIdsMap; }
		std::string texturesPath = cfg.ASSETS_PATH + std::string("\\") + textureDirName;
		for (const auto & entry : std::filesystem::directory_iterator(texturesPath))
		{
			if (entry.is_regular_file())
			{
				if (entry.path().filename().has_extension()) {
					filename = entry.path().filename().replace_extension("").string();
				} else {
					filename = entry.path().filename().string();
				}

				//declare texture
				glGenTextures(1, &(*idsMap)[filename]);
				//std::cout << filename << ": " << (*idsMap)[filename] << std::endl;
				//bind texture
				glBindTexture(GL_TEXTURE_2D, (*idsMap)[filename]);

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
					std::cout << "Failed to load " << filename << " from path: " << texturesPath << std::endl;
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
	}
	//activate shader
	geometryShader.bind();
	//set the uniforms
	geometryShader.setInt("frameIndex", 0);
	geometryShader.unbind();
}

void Engine3D::loadCubemaps(std::map<std::string, GLuint>& cubemapIdsMap, std::map<std::string, GLuint>& cubeLightmapIdsMap, std::map<std::string, GLuint>& cubeNormalmapIdsMap, std::map<std::string, GLuint>& cubeDisplacementmapIdsMap)
{
	std::string cubemapsDirNames[5] = {std::string("cubemaps"), std::string("skyboxes"), std::string("cubelightmaps"), std::string("cubenormalmaps"), std::string("cubeDisplacementmaps")};
	std::string filename;
	std::string name;
	unsigned short i = 0;
	std::map<std::string, GLuint>* idsMap;
	for (std::string cubemapsDirName : cubemapsDirNames) {
		if (++i == 3)  { if (!cfg.LIGHT_MAPPING)        { continue; } idsMap = &cubeLightmapIdsMap; }
		else if (i==4) { if (!cfg.NORMAL_MAPPING)       { continue; } idsMap = &cubeNormalmapIdsMap; }
		else if (i==5) { if (!cfg.DISPLACEMENT_MAPPING) { continue; } idsMap = &cubeDisplacementmapIdsMap; }
		else { idsMap = &cubemapIdsMap; }
		std::string cubemapsPath = cfg.ASSETS_PATH + std::string("\\") + cubemapsDirName;
		for (const auto & entry : std::filesystem::directory_iterator(cubemapsPath))
		{
			if (entry.is_directory())
			{
				name = entry.path().filename().string();

				glGenTextures(1, &(*idsMap)[name]);
				glBindTexture(GL_TEXTURE_CUBE_MAP, (*idsMap)[name]);

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
								std::cout << "Cubemap texture failed to load from path: " << face.path() << std::endl;
								stbi_image_free(data);
							}
						}
						glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
						glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
						glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
					}
				}
			}
		}
	}
}

/*
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
*/

void Engine3D::renderScreenQuad()
{
	if (scrQuadVAO == 0)
	{
		float quadVertices[] = {
				// positions        // texture Coords
				-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
				 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &scrQuadVAO);
		glGenBuffers(1, &scrQuadVBO);
		glBindVertexArray(scrQuadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, scrQuadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(scrQuadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void Engine3D::render()
{
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	if (updateVerticesFlag) updateVertices();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//clear color buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//geometry pass: write geometry to the multisampling buffer or the regular one
	if (cfg.MSAA && cfg.MSAA_SAMPLES > 1) {
		glEnable(GL_MULTISAMPLE);
		glBindFramebuffer(GL_FRAMEBUFFER, gBOMS);
	} else {
		glBindFramebuffer(GL_FRAMEBUFFER, gBO);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//render skybox
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	glCullFace(GL_FRONT);
	geometrySkyboxShader.bind();
	geometrySkyboxShader.setMat4("projection", getProjectionMatrix());
	geometrySkyboxShader.setMat4("view", getViewMatrixNoTranslation());
	if (finalSkyBoxToRender != nullptr)
	{
		cubeModel& cm = *finalSkyBoxToRender;
		cm.render(&geometrySkyboxShader, gCubeVAO, gCubeIBO, cubemapIdsMap[cm.texture], 0, 0, 0);
	}
	geometrySkyboxShader.unbind();

	glCullFace(GL_BACK);

	//render cubemaps
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	geometryCubemapShader.bind();
	geometryCubemapShader.setMat4("projection", getProjectionMatrix());
	geometryCubemapShader.setMat4("view", getViewMatrix());
	geometryCubemapShader.setVec3("viewPos", getCameraPos());
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
		cm.render(&geometryCubemapShader, gCubeVAO, gCubeIBO, cubemapIdsMap[cm.texture], cubeLightmapIdsMap[cm.texture], cubeNormalmapIdsMap[cm.texture], cubeDisplacementmapIdsMap[cm.texture]);
	}

	//render other models
	geometryShader.bind();
	geometryShader.setMat4("projection", getProjectionMatrix());
	geometryShader.setMat4("view", getViewMatrix());
	geometryShader.setVec3("viewPos", getCameraPos());
	for (auto itr = finalModelsToRender.begin(); itr != finalModelsToRender.end(); itr++)
	{
		if (!(*itr)) { std::cout << "nullptr!" << std::endl; continue; }

		model& model = *(*itr);

		if (model.removeFlag) continue;

		if (model.texture.length() && textureTransparencyMap[model.texture]==true) {
			finalTransparentModelsToRender.insert(*itr);
			continue;
		}
		model.render(&geometryShader, gVAO, gIBO, textureIdsMap[model.texture], lightmapIdsMap[model.texture], normalmapIdsMap[model.texture], displacementmapIdsMap[model.texture]);
	}

	for (auto itr = finalTransparentModelsToRender.begin(); itr != finalTransparentModelsToRender.end(); itr++)
	{
		if (!(*itr)) { std::cout << "nullptr!" << std::endl; continue; }

		model& model = *(*itr);

		model.render(&geometryShader, gVAO, gIBO, textureIdsMap[model.texture], lightmapIdsMap[model.texture], normalmapIdsMap[model.texture], displacementmapIdsMap[model.texture]);
	}
	finalTransparentModelsToRender.clear();

	//resolve multisampling
	if (cfg.MSAA && cfg.MSAA_SAMPLES > 1) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBOMS);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gBO);
		glBlitFramebuffer(0, 0, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, 0, 0, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, 0, 0, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glBlitFramebuffer(0, 0, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, 0, 0, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glReadBuffer(GL_COLOR_ATTACHMENT2);
		glDrawBuffer(GL_COLOR_ATTACHMENT2);
		glBlitFramebuffer(0, 0, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, 0, 0, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glReadBuffer(GL_COLOR_ATTACHMENT3);
		glDrawBuffer(GL_COLOR_ATTACHMENT3);
		glBlitFramebuffer(0, 0, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, 0, 0, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glReadBuffer(GL_COLOR_ATTACHMENT4);
		glDrawBuffer(GL_COLOR_ATTACHMENT4);
		glBlitFramebuffer(0, 0, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, 0, 0, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	//lighting pass
	glBindFramebuffer(GL_FRAMEBUFFER, lightingBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	lightingShader.bind();
	lightingShader.setVec3("viewPos", getCameraPos());
	lightingShader.setVec3("light.direction", light.direction);
	lightingShader.setVec3("light.color", light.color);
	lightingShader.setFloat("light.ambientIntensity", light.ambientIntensity);
	lightingShader.setFloat("light.diffuseIntensity", light.diffuseIntensity);
	lightingShader.setFloat("light.specularIntensity", light.specularIntensity);
	lightingShader.setInt("gPosition", 0);
	lightingShader.setInt("gNormal", 1);
	lightingShader.setInt("gAlbedo", 2);
	lightingShader.setInt("gLightmap", 3);
	lightingShader.setInt("gViewDir", 4);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gLightmap);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, gViewDir);

	renderScreenQuad();

	//post-processing pass
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	postProcShader.bind();
	postProcShader.setInt("screenTexture", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, screenTexture);

	renderScreenQuad();

	// glBindFramebuffer(GL_READ_FRAMEBUFFER, gBO);
	// glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
	// // blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
	// // the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
	// // depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
	// glBlitFramebuffer(0, 0, cfg.SCREEN_WIDTH, cfg.SCREEN_WIDTH, 0, 0, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	// glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//textureShader.unbind();

	mtx.unlock();

	//renderUI();

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
			glm::vec3 normal = glm::normalize(glm::cross(line2, line1));
			tri.tang = tri.calcTangent();

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
				vdp->push_back(tri.tang.x);
				vdp->push_back(tri.tang.y);
				vdp->push_back(tri.tang.z);
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(0);
	//normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	//color attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GL_FLOAT), (void*)(6 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);
	//texture coord attribute
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GL_FLOAT), (void*)(9 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(3);
	//tangent attribute
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GL_FLOAT), (void*)(11 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(4);

	//update IBO
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GL_UNSIGNED_INT), indexData.data(), GL_STATIC_DRAW );

	//update cubeVBO
	glBindVertexArray(gCubeVAO);
	glBindBuffer( GL_ARRAY_BUFFER, gCubeVBO );
	glBufferData( GL_ARRAY_BUFFER, cubeVertexData.size() * sizeof(GL_FLOAT), cubeVertexData.data(), GL_STATIC_DRAW );

	//position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(0);
	//normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	//color attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GL_FLOAT), (void*)(6 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);
	//texture coord attribute
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GL_FLOAT), (void*)(9 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(3);
	//tangent attribute
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GL_FLOAT), (void*)(11 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(4);

	//update cubeIBO
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gCubeIBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, cubeIndexData.size() * sizeof(GL_UNSIGNED_INT), cubeIndexData.data(), GL_STATIC_DRAW );

	updateVerticesFlag = false;

	std::cout << "Updated vertices" << std::endl;
}

/*
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
*/

bool Engine3D::onUserCreate()
{
	return true;
}

bool Engine3D::onUserUpdate(float elapsedTime)
{
	mtx.lock();

	captureInput();

	glm::vec3 prevPersonPos = personPos;
	glm::vec3 center{ 0, 0, 0 };
	float modelDistance = dof;
	float maxModelDist = dof;
	float minModelDist = dof;
	glm::vec4 collidingTriPts[3];

	move(elapsedTime);

	collides = false;
	collidesFront = false;
	collidesBack = false;
	collidesRight = false;
	collidesLeft = false;

	canSlide = false;
	hasLanded = false;
	shouldClimb = false;

	modelsInFocus.clear();

	projectionMatrix = glm::perspective(glm::radians((float)fov), (float)width / (float)height, near, far);
	viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	mtx.unlock();

	//for each model to render
	for (auto &ptrModel : ptrModelsToRender)
	{
		if (!ptrModel) continue;
		model& mdl = *ptrModel;

		//ignore for some loops
		if (mdl.ignoreForCycles > 0) { mdl.ignoreForCycles--; continue; }

		if (mdl.id == 4088797665) {
			mdl.isSolid = false;
			mdl.position = getPersonPos();
		}

		mtx.lock();
		glm::mat4 modelMatrix = glm::mat4(1.0f); //make sure to initialize matrix to identity matrix first
		modelMatrix = glm::translate(modelMatrix, mdl.position);
		modelMatrix = modelMatrix * mdl.rotationMatrix;
		mdl.modelMatrix = modelMatrix;

		mdl.inFocus = false;
		bool checkAgainForFocus = true;
		mdl.isInFOV = false;
		bool checkAgainForFOV = true;
		float minX = 1.0f, maxX = -1.0f, minY = 1.0f, maxY = -1.0f, minZ = 100000.0f, maxZ = -100000.0f;

		//if cube is skybox, then do not process further
		if (mdl.modelMesh.shape == shapetype::CUBE)
		{
			cubeModel& cube = dynamic_cast<cubeModel &>(mdl);
			if (cube.isSkyBox)
			{
				mdl.isInDOF = true;
				mdl.isInFOV = true;
				mdl.isCovered = false;
				mtx.unlock();
				continue;
			}
		}
		mtx.unlock();

		modelDistance = dof;
		maxModelDist = dof;
		minModelDist = dof;
		
		//initialize highest and lowest Y positions of model
		float highestYOfModel = (modelMatrix * mdl.modelMesh.tris[0].p[0]).y;
		float lowestYOfModel  = (modelMatrix * mdl.modelMesh.tris[0].p[0]).y;

		for (auto tri : mdl.modelMesh.tris)
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
			float avgDist = glm::distance(personPos, glm::vec3(avgP));
			float dist1 = glm::distance(personPos, glm::vec3(pt[0]));
			float dist2 = glm::distance(personPos, glm::vec3(pt[1]));
			float dist3 = glm::distance(personPos, glm::vec3(pt[2]));
			float maxDist = std::max(dist1, std::max(dist2, dist3));
			float minDist = std::min(dist1, std::min(dist2, dist3));
			float highestYInTri = std::max(pt[0].y, std::max(pt[1].y, pt[2].y));
			float lowestYInTri = std::min(pt[0].y, std::min(pt[1].y, pt[2].y));
			if (highestYOfModel < highestYInTri) { highestYOfModel = highestYInTri; mdl.highestY = highestYInTri; }
			if (lowestYOfModel > lowestYInTri) { lowestYOfModel = lowestYInTri; mdl.lowestY = lowestYInTri; }
			if (avgDist < modelDistance) {
				modelDistance = avgDist;
				maxModelDist = maxDist;
				minModelDist = minDist;
				mdl.distance = minDist;
				collidingTriPts[0] = pt[0];
				collidingTriPts[1] = pt[1];
				collidingTriPts[2] = pt[2];
			}
			if (minDist < minModelDist) {
				minModelDist = minDist;
				mdl.distance = minDist;
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
				mdl.inFocus = true;
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
		mdl.bbox = bbox;

		//mark out-of-FOV models to avoid needless rendering
		mdl.isInFOV = ((mdl.bbox.minX > 0 && mdl.bbox.minX < 1) || (mdl.bbox.maxX > 0 && mdl.bbox.maxX < 1)) && ((mdl.bbox.minY > 0 && mdl.bbox.minY < 1) || (mdl.bbox.maxY > 0 && mdl.bbox.maxY < 1));

		//mark out-of-DOF models to avoid needless rendering
		mdl.isInDOF = (camera.positionMode == camerapositionmode::ATTACHED_TO_PERSON) ? modelDistance < dof : glm::distance(camera.position, mdl.position) < dof;

		//get the triangle normal
		glm::vec3 line1 = collidingTriPts[1] - collidingTriPts[0];
		glm::vec3 line2 = collidingTriPts[2] - collidingTriPts[0];
		glm::vec3 normal = glm::normalize(glm::cross(line1, line2));
		float dpBottom = glm::dot(personUp, normal); //dot product is near to 1 means collision with floor

		//detect vertical collision (floor)
		if (mdl.isSolid && minModelDist < collidingDistanceV * 1.5f && std::abs(dpBottom) > 0.5f && highestYOfModel < personPos.y/2.0f && lowestYOfModel < personPos.y) {
			hasLanded = true;
			//determine whether to climb (stairs, ramps, etc.): if the highest Y is above ground and lower than half the person height
			if (personPos.y - collidingDistanceV < highestYOfModel) {
				shouldClimb = true;
				//std::cout << "SHOULD CLIMB... " << std::endl;
			}
			//std::cout << "personPos.y = " << personPos.y << ", minModelDist = " << minModelDist << ", highestYOfModel = " << highestYOfModel << std::endl;
			//std::cout << normal.x << ", " << normal.y << ", " << normal.z << std::endl;
		} else if (mdl.isSolid && minModelDist < collidingDistanceV * 2.0f && std::abs(dpBottom) < 0.5f && lowestYOfModel >= personPos.y && highestYOfModel > personPos.y) {
			jumpSpeed = 0.0f;
		}
		//detect horizontal collision (wall)
		if (!collides && mdl.isSolid && modelDistance <= collidingDistanceH * 1.5f && std::abs(dpBottom) <= 0.5f)
		{
			//std::cout << "modelDistance: " << modelDistance << std::endl;
			//std::cout << "dpBottom: " << dpBottom << std::endl;

			//if (maxModelDist < collidingDistanceH * 1.5f) {

				//based on dp and normal, determine if able to slide and the desired motion
				float dpFront = glm::dot(personFront, normal);
				float absDP = std::abs(dpFront);
				float dpRight = glm::dot(personRight, normal);
				float dpLeft = glm::dot(-personRight, normal);
				float dpBack = glm::dot(-personFront, normal);

				canSlide = absDP < 0.8f && absDP > 0.0f;
				//std::cout << "absDP: " << absDP << std::endl;
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
				desiredMotion = personFront - undesiredMotion;
				desiredMotion = glm::normalize(desiredMotion);
				//std::cout << desiredMotion.x << ", " << desiredMotion.z << std::endl;
			//}
		}

		if (mdl.isSolid && modelDistance < collidingDistanceH * 0.5f)
		{
			float personPosY = personPos.y; // to be able to fall if collided in the air
			setPersonPos(glm::vec3(prevPersonPos.x, personPosY, prevPersonPos.z));
		}

		//models that are far away will be ignored for some loops
		float camDist = glm::distance(getCameraPos(), mdl.position);
		if (mdl.ignoreForCycles == 0 && camDist > dof/5.0f) { mdl.ignoreForCycles = camDist * 3; }

	}
	
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
			ptrModel->position += ptrModel->speed * glm::normalize(personPos - ptrModel->position) * elapsedTime;
		}

		//render if model is near, or not covered, in DOF and in FOV
		if (ptrModel->distance <= 2.0f || (!ptrModel->isCovered && ptrModel->isInDOF && ptrModel->isInFOV))
		{
			if (ptrModel->modelMesh.shape == shapetype::CUBE) finalCubeModelsToRender.insert(ptrModel);
			else finalModelsToRender.insert(ptrModel);
		} else
		{
			if (ptrModel->modelMesh.shape == shapetype::CUBE) finalCubeModelsToRender.erase(ptrModel);
			else finalModelsToRender.erase(ptrModel);
		}

	}
	//std::cout << "final models to render: " << finalCubeModelsToRender.size() << std::endl;
	mtx.unlock();

	edit(elapsedTime);

	//std::cout <<"collides? " << collides << ", canSlide? " << canSlide << ", hasLanded? " << hasLanded << std::endl;
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
	float personSpeed = static_cast<float>(personSpeedFactor * elapsedTime);

	//falling
	if (!hasLanded) {
		setPersonPos(personPos + static_cast<float>(gravitationalPull * elapsedTime) * glm::vec3(0, -1, 0));
	}

	//climbing (stairs, ramps, etc.)
	if (shouldClimb && userMode == UserMode::PLAYER) {
		setPersonPos(personPos + 0.5f * personSpeed * glm::vec3(0, 1, 0));
	}

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
			setPersonPos(personPos + 0.5f * personSpeed * desiredMotion);

		} else if (keysPressed[SupportedKeys::W] && hasLanded && (!collides || !collidesFront)) {
			personFront.y = 0;
			setPersonPos(personPos + personSpeed * personFront);

		} else if (keysPressed[SupportedKeys::W] && (/*!hasLanded ||*/ userMode == UserMode::EDITOR)) {
			setPersonPos(personPos + personSpeed * personFront);

		} else if (keysPressed[SupportedKeys::S] && hasLanded && collides && canSlide) {
			desiredMotion.y = 0;
			setPersonPos(personPos - 0.5f * personSpeed * desiredMotion);

		} else if (keysPressed[SupportedKeys::S] && hasLanded && (!collides || !collidesBack)) {
			personFront.y = 0;
			setPersonPos(personPos - personSpeed * personFront);

		} else if (keysPressed[SupportedKeys::S] && (/*!hasLanded ||*/ userMode == UserMode::EDITOR)) {
			setPersonPos(personPos - personSpeed * personFront);
		}
		if (keysPressed[SupportedKeys::A] && hasLanded && collides && !collidesLeft) {
			desiredMotion.y = 0;
			bool desiresRight = glm::dot(personRight, desiredMotion) < 0;
			if (desiresRight) setPersonPos(personPos + 0.5f * personSpeed * desiredMotion);
			else setPersonPos(personPos - 0.5f * personSpeed * desiredMotion);
		
		} else if (keysPressed[SupportedKeys::A] && (hasLanded && !collides || userMode == UserMode::EDITOR)) {
			setPersonPos(personPos - glm::normalize(glm::cross(personFront, personUp)) * personSpeed);
		
		} else if (keysPressed[SupportedKeys::D] && hasLanded && collides && !collidesRight) {
			desiredMotion.y = 0;
			bool desiresRight = glm::dot(personRight, desiredMotion) < 0;
			if (desiresRight) setPersonPos(personPos - 0.5f * personSpeed * desiredMotion);
			else setPersonPos(personPos + 0.5f * personSpeed * desiredMotion);

		} else if (keysPressed[SupportedKeys::D] && (hasLanded && !collides || userMode == UserMode::EDITOR)) {
			setPersonPos(personPos + glm::normalize(glm::cross(personFront, personUp)) * personSpeed);
		}

		//jumping
		if (keysPressed[SupportedKeys::SPACE] && hasLanded) {
			jumpSpeed = gravitationalPull * jumpSpeedFactor;
			setPersonPos(personPos + glm::vec3(0.0f, elapsedTime * jumpSpeed, 0.0f));
			if (keysPressed[SupportedKeys::W] && !collides) { //if jumped while moving forward
				personFrontOnJump = personFront;
				personFrontOnJump.y = 0;
			}else {
				personFrontOnJump = glm::vec3(0, 0, 0);
			}
		}
		//keep elevating if in the air and there is jump speed
		if (!hasLanded && jumpSpeed > 0) {
			setPersonPos(personPos + glm::vec3(0.0f, elapsedTime * jumpSpeed, 0.0f));
			jumpSpeed -= jumpSpeed * elapsedTime;
			if (!collides) { //if not colliding, move forward too
				personFront.y = 0;
				setPersonPos(personPos + 0.5f * personSpeed * personFrontOnJump);
			}
		}
		//std::cout << "jumpSpeed" << jumpSpeed << std::endl;

		//if in editor mode, arrows control track camera movement to the sides, above or below camera position
		if (userMode == UserMode::EDITOR)
		{
			if (keysPressed[SupportedKeys::LEFT_ARROW]) {
				setPersonPos(personPos - glm::normalize(glm::cross(personFront, personUp)) * personSpeed);
			} else if (keysPressed[SupportedKeys::RIGHT_ARROW]) {
				setPersonPos(personPos + glm::normalize(glm::cross(personFront, personUp)) * personSpeed);
			}
			if (keysPressed[SupportedKeys::UP_ARROW]) {
				setPersonPos(personPos - glm::normalize(glm::cross(personFront, personRight)) * personSpeed);
			} else if (keysPressed[SupportedKeys::DOWN_ARROW]) {
				setPersonPos(personPos + glm::normalize(glm::cross(personFront, personRight)) * personSpeed);
			}
		}

		//std::cout << "x: " << (eventController->mouseRelX) << std::endl;

		if (eventController->mouseRelX > 0) {
			yaw += ((float)eventController->mouseRelX / width) * 180;
		} else if (eventController->mouseRelX < 0) {
			yaw += ((float)eventController->mouseRelX / width) * 180;
		}

		if (eventController->mouseRelY > 0) {
			pitch -= ((float)eventController->mouseRelY / height) * 180;
		} else if (eventController->mouseRelY < 0) {
			pitch -= ((float)eventController->mouseRelY / height) * 180;
		}

		//mouse motion
		if (keysPressed[SupportedKeys::MOUSE_LEFT]) {
			//yaw -= multiplierX * elapsedTime;
		} else if (keysPressed[SupportedKeys::MOUSE_RIGHT]) {
			//yaw += multiplierX * elapsedTime;
		}

		if (keysPressed[SupportedKeys::MOUSE_UP]) {
			//pitch += multiplierY * elapsedTime;
		} else if (keysPressed[SupportedKeys::MOUSE_DOWN]) {
			//pitch -= multiplierY * elapsedTime;
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
		if (camera.positionMode == camerapositionmode::ATTACHED_TO_PERSON) {
			setPersonFront(glm::normalize(front));
			setPersonRight(glm::normalize(glm::cross(personFront, personUp)));
		} else if (camera.positionMode == camerapositionmode::FIXED) {
			setCameraFront(glm::normalize(front));
			setCameraRight(glm::normalize(glm::cross(personFront, personUp)));
		}
		setPersonPos(getPersonPos());
	}
}

bool Engine3D::onUserDestroy()
{
	std::cout << "Destroying Engine3D..." << std::endl;

	printf("Stopping rendering thread...\n");

	rendererThread.join();

	printf("Unbinding and deleting shader programs...\n");

	//unbind program - deactivate shader
	geometryShader.unbind();
	geometryCubemapShader.unbind();
	geometrySkyboxShader.unbind();
	lightingShader.unbind();
	postProcShader.unbind();

	//deallocate programs
	geometryShader.freeProgram();
	geometryCubemapShader.freeProgram();
	geometrySkyboxShader.freeProgram();
	lightingShader.freeProgram();
	postProcShader.freeProgram();

	// printf("Shutting down ImGui...\n");
	// ImGui_ImplOpenGL3_Shutdown();
	// ImGui_ImplSDL2_Shutdown();
	// ImGui::DestroyContext();

	return true;
}

glm::mat4 Engine3D::getProjectionMatrix() const
{
	return projectionMatrix;
}

void Engine3D::setCameraPos(glm::vec3 pos)
{
	cameraPos = pos;
}

glm::vec3 Engine3D::getCameraPos() const
{
	return cameraPos;
}

void Engine3D::setCameraOffset(glm::vec3 pos)
{
	cameraOffset = pos;
}

glm::vec3 Engine3D::getCameraOffset() const
{
	return cameraOffset;
}

void Engine3D::setCameraFront(glm::vec3 pos)
{
	cameraFront = pos;
}

glm::vec3 Engine3D::getCameraFront() const
{
	return cameraFront;
}

void Engine3D::setCameraUp(glm::vec3 pos)
{
	cameraUp = pos;
}

glm::vec3 Engine3D::getCameraUp() const
{
	return cameraUp;
}

void Engine3D::setCameraRight(glm::vec3 pos)
{
	cameraRight = pos;
}

glm::vec3 Engine3D::getCameraRight() const
{
	return cameraRight;
}

void Engine3D::setPersonPos(glm::vec3 pos)
{
	personPos = pos;
	if (camera.positionMode == camerapositionmode::ATTACHED_TO_PERSON) {
		glm::vec3 worldOffset = (cameraOffset.x * getPersonRight()) + (cameraOffset.y * getPersonUp()) + (cameraOffset.z * getPersonFront());
		cameraPos = personPos + worldOffset;
	}
}

glm::vec3 Engine3D::getPersonPos() const
{
	return personPos;
}

void Engine3D::setPersonFront(glm::vec3 pos)
{
	personFront = pos;
	if (camera.positionMode == camerapositionmode::ATTACHED_TO_PERSON) {
		glm::vec3 worldOffset = (cameraOffset.x * getPersonRight()) + (cameraOffset.y * getPersonUp()) + (cameraOffset.z * getPersonFront());
		cameraFront = glm::normalize(personFront - worldOffset);
	} else if (camera.positionMode == camerapositionmode::FIXED) {
		cameraFront = personFront;
	}
}

glm::vec3 Engine3D::getPersonFront() const
{
	return personFront;
}

void Engine3D::setPersonUp(glm::vec3 pos)
{
	if (camera.positionMode == camerapositionmode::ATTACHED_TO_PERSON) {
		cameraUp = personUp = pos;
	} else if (camera.positionMode == camerapositionmode::FIXED) {
		personUp = pos;
	}
}

glm::vec3 Engine3D::getPersonUp() const
{
	return personUp;
}

void Engine3D::setPersonRight(glm::vec3 pos)
{
	if (camera.positionMode == camerapositionmode::ATTACHED_TO_PERSON) {
		cameraRight = personRight = pos;
	} else if (camera.positionMode == camerapositionmode::FIXED) {
		personRight = pos;
	}
}

glm::vec3 Engine3D::getPersonRight() const
{
	return personRight;
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
	setPersonPos(level->playerPosition);
	this->light = level->light;
	this->level = std::make_shared<Level>(*level);
}