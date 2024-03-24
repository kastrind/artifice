#include "Initiator.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool Initiator::init()
{
	//initialization flag
	bool success = true;

	//initialize SDL
	printf( "Initializing SDL...\n" );
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//use OpenGL 3.1 core
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

		//create window
		printf( "Creating window...\n" );
		gWindow = SDL_CreateWindow( "Artifice Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
		//confine mouse cursor to the window and hide it
		SDL_SetWindowMouseGrab(gWindow, SDL_TRUE);
		SDL_SetWindowMouseRect(gWindow, &windowRect);
		SDL_SetRelativeMouseMode(SDL_TRUE);
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//create context
			printf( "Creating OpenGL context...\n" );
			gContext = SDL_GL_CreateContext( gWindow );
			if( gContext == NULL )
			{
				printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
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
				}

				//use Vsync
				printf( "Setting VSync...\n" );
				if( SDL_GL_SetSwapInterval( 1 ) < 0 )
				{
					printf( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
				}

				//instantiate the game engine
				artificeEngine = new Engine3D(cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT,
											  cfg.NEAR, cfg.FAR, cfg.FOV, cfg.DOF,
											  cfg.COLLIDING_DISTANCE,
											  cfg.GRAVITATIONAL_PULL,
											  cfg.CAMERA_SPEED_FACTOR,
											  cfg.USER_MODE, &eventController);

				//initialize OpenGL
				printf( "Initializing OpenGL...\n" );
				if( !initGL() )
				{
					printf( "Unable to initialize OpenGL!\n" );
					success = false;
				}

                //start the 3D engine
				printf( "Starting Engine thread...\n" );
		        engineThread = artificeEngine->startEngine();

				//start listening for input events
				printf( "Starting input events listener thread...\n" );
				eventListenerThread = eventController.startListening();
			}
		}
	}

    initiated = success;
	return success;
}

bool Initiator::initGL()
{
	bool success = true;

	printf( "Loading shader programs...\n" );
	if (!cubeMapShader.loadProgram("shaders/cubemap.glvs", "shaders/cubemap.glfs"))
	{
		printf( "Unable to load cubemap shader!\n" );
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
		// glDepthFunc(GL_LESS); 
		// glEnable(GL_CULL_FACE);
		// glCullFace(GL_BACK);

		gCubeMapProgramID = cubeMapShader.getProgramID();
		gTextureProgramID = textureShader.getProgramID();

		Level level;
		level.load(cfg.LEVELS_PATH + "\\" + levelFile);
		artificeEngine->modelsToRaster = level.models;

		/*
		//create a rectangle
		rectangle rect0{0.2, 0.4};
		model mdl0; mdl0.texture = "brickwall";
		mdl0.position = glm::vec3( -0.7f,  0.5f,  0.2f);
		rect0.toTriangles(mdl0.modelMesh.tris);
		mdl0.modelMesh.shape = Shape::RECTANGLE;
		artificeEngine->modelsToRaster.push_back(mdl0);

		//create a rectangle
		rectangle rect0b{0.2, 0.2};
		model mdl0b; mdl0b.texture = "walnut";
		mdl0b.position = glm::vec3( 0.2f,  0.7f,  0.5f);
		rect0b.toTriangles(mdl0b.modelMesh.tris);
		mdl0b.modelMesh.shape = Shape::RECTANGLE;
		artificeEngine->modelsToRaster.push_back(mdl0b);

		rectangle rect0c{0.2, 0.2};
		model mdl0c; mdl0c.texture = "brickwallPainted";
		mdl0c.position = glm::vec3( 0.5f,  0.1f,  0.1f);
		rect0c.toTriangles(mdl0c.modelMesh.tris);
		mdl0c.modelMesh.shape = Shape::RECTANGLE;
		artificeEngine->modelsToRaster.push_back(mdl0c);

		//create a cube
		cube cube0{0.2f};
		std::vector<triangle> cube0Triangles;
		cube0.toTriangles(cube0Triangles);

		//create models
		model mdl; mdl.texture = "cubemap";
		mdl.position = glm::vec3( 0.0f,  0.0f,  0.0f);
		mdl.modelMesh.tris = cube0Triangles;
		mdl.modelMesh.shape = Shape::CUBE;
		artificeEngine->modelsToRaster.push_back(mdl);

		model mdl2; mdl2.texture = "cubemap";
		mdl2.position = glm::vec3( 0.0f,  0.0f,  0.2f);
		mdl2.modelMesh.tris = cube0Triangles;
		mdl2.modelMesh.shape = Shape::CUBE;
		artificeEngine->modelsToRaster.push_back(mdl2);

		model mdl3; mdl3.texture = "cubemap";
		mdl3.position = glm::vec3( -0.2f,  0.2f,  0.0f);
		mdl3.modelMesh.tris = cube0Triangles;
		mdl3.modelMesh.shape = Shape::CUBE;
		artificeEngine->modelsToRaster.push_back(mdl3);

		model mdl4; mdl4.texture = "cubemap";
		mdl4.position = glm::vec3( -0.2f,  0.2f, 0.2f);
		mdl4.modelMesh.tris = cube0Triangles;
		mdl4.modelMesh.shape = Shape::CUBE;
		artificeEngine->modelsToRaster.push_back(mdl4);

		model mdl5; mdl5.texture = "cubemap";
		mdl5.position = glm::vec3( 0.0f,  0.0f, 0.4f);
		mdl5.modelMesh.tris = cube0Triangles;
		mdl5.modelMesh.shape = Shape::CUBE;
		artificeEngine->modelsToRaster.push_back(mdl5);

		model mdl6; mdl6.texture = "box";
		mdl6.position = glm::vec3( 0.2f,  0.2f, 0.0f);
		mdl6.modelMesh.tris = cube0Triangles;
		mdl6.modelMesh.shape = Shape::CUBE;
		artificeEngine->modelsToRaster.push_back(mdl6);

		model mdl7; mdl7.texture = "box";
		mdl7.position = glm::vec3( 0.2f,  0.2f, 0.2f);
		mdl7.modelMesh.tris = cube0Triangles;
		mdl7.modelMesh.shape = Shape::CUBE;
		artificeEngine->modelsToRaster.push_back(mdl7);

		model mdl8; mdl8.texture = "walnut";
		mdl8.position = glm::vec3( 1.0f,  0.0f, 0.2f);
		cuboid cuboid0{0.2, 0.5, 0.2};
		cuboid0.toTriangles(mdl8.modelMesh.tris);
		mdl8.modelMesh.shape = Shape::CUBOID;
		artificeEngine->modelsToRaster.push_back(mdl8);

		model mdl9; mdl9.texture = "brickwall";
		mdl9.position = glm::vec3( 1.5f,  1.0f, 0.2f);
		cuboid cuboid1{0.2, 0.5, 0.2};
		cuboid1.toTriangles(mdl9.modelMesh.tris);
		mdl9.modelMesh.shape = Shape::CUBOID;
		artificeEngine->modelsToRaster.push_back(mdl9);
		*/
		
		//create VAOs
		glGenVertexArrays(1, &gVAO);
		glGenVertexArrays(1, &gCubeVAO);

		//create VBOs
		glGenBuffers( 1, &gVBO );
		glGenBuffers( 1, &gCubeVBO );

		//create IBO
		glGenBuffers( 1, &gIBO );
		glGenBuffers( 1, &gCubeIBO );

		artificeEngine->gVBO = &gVBO;
		artificeEngine->gIBO = &gIBO;
		artificeEngine->gVAO = &gVAO;
		artificeEngine->gCubeVBO = &gCubeVBO;
		artificeEngine->gCubeIBO = &gCubeIBO;
		artificeEngine->gCubeVAO = &gCubeVAO;

		//update buffers with the new vertices
		artificeEngine->updateVertices();

		//initialize clear color
		glClearColor( 0.f, 0.f, 0.f, 1.f );

		//generates and binds textures
		loadTextures(textureIdsMap);
		artificeEngine->textureShader = &textureShader;
		artificeEngine->textureIdsMap = &textureIdsMap;

		//generates and binds cubemap
		loadCubemaps(cubemapIdsMap);
		artificeEngine->cubeMapShader = &cubeMapShader;
		artificeEngine->cubemapIdsMap = &cubemapIdsMap;
	}
	return success;
}

void Initiator::loadTextures(std::map<std::string, GLuint>& textureIdsMap)
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

			//set the texture wrapping/filtering options (on the currently bound texture object)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			//load image
			int width, height, nrChannels;
			unsigned char *data = stbi_load(entry.path().string().c_str(), &width, &height, &nrChannels, 0);
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
		}
	}
	//activate shader
	textureShader.bind();
	//set the uniforms
	for (const auto& kv : textureIdsMap) {
		glUniform1i(glGetUniformLocation(textureShader.getProgramID(), std::string("texture" + std::to_string(kv.second)).c_str()), 0);
	}
}

void Initiator::loadCubemaps(std::map<std::string, GLuint>& cubemapIdsMap)
{
    std::string cubemapsPath = cfg.ASSETS_PATH + std::string("\\cubemaps");
	std::string filename;
	std::string name;
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
				}
			}
		}
	}
}

void Initiator::close()
{
	printf("Unbinding and deleting shader programs...\n");

	//unbind program - deactivate shader
	cubeMapShader.unbind();
	textureShader.unbind();

	//deallocate programs
	cubeMapShader.freeProgram();
	textureShader.freeProgram();

	printf("Destroying SDL window...\n");

	//destroy window	
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	printf("Stopping threads...\n");

	engineThread.join();
	eventListenerThread.join();

	printf("Quitting SDL subsystems...\n");

	//quit SDL subsystems
	SDL_Quit();
}