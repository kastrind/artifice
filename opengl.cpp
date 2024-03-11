#include "Initiator.h"

CFG cfg;

std::vector<std::string> texturePaths;
std::vector<std::string> cubemapPaths;

int main( int argc, char* args[] )
{
	//TODO: relocate
	texturePaths.push_back("brickwall.bmp");
	texturePaths.push_back("brickwallPainted.bmp");
	texturePaths.push_back("walnut.bmp");

	cubemapPaths.push_back("brickwallPainted.bmp"); //right
	cubemapPaths.push_back("brickwall.bmp"); //left
	cubemapPaths.push_back("brickwallPainted.bmp"); //top
	cubemapPaths.push_back("brickwall.bmp"); //bottom
	cubemapPaths.push_back("brickwallPainted.bmp"); //back
	cubemapPaths.push_back("brickwall.bmp"); //front

	Initiator initor(cfg, texturePaths, cubemapPaths);

	//start up SDL and create window, OpenGL context, 3D Engine
	if(!initor())
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		SDL_Window* gWindow = initor.gWindow;
		SDL_Rect* windowRect = &initor.windowRect;
		EventController* eventController = &initor.eventController;
		Engine3D* artificeEngine = initor.artificeEngine;
		GLuint* gCubeMapProgramID = &initor.gCubeMapProgramID;
		GLuint* gTextureProgramID = &initor.gTextureProgramID;
		GLuint* gVBO = &initor.gVBO;
		GLuint* gIBO = &initor.gIBO;
		GLuint* gVAO = &initor.gVAO;
		GLuint* gCubeVBO = &initor.gCubeVBO;
		GLuint* gCubeIBO = &initor.gCubeIBO;
		GLuint* gCubeVAO = &initor.gCubeVAO;
		ArtificeShaderProgram* textureShader = &initor.textureShader;
		ArtificeShaderProgram* cubeMapShader = &initor.cubeMapShader;
		std::map<std::string, GLuint>* textureIdsMap = &initor.textureIdsMap;
		std::map<std::string, GLuint>* cubemapIdsMap = &initor.cubemapIdsMap;

		//main loop flag
		bool quit = false;

		//event handler
		SDL_Event e;
		
		//enable text input
		SDL_StartTextInput();

		//while application is running
		while( !quit )
		{

			eventController->clearMouseMotionState();

			//handle events on queue
			while( SDL_PollEvent( &e ) != 0 )
			{
				//user requests quit
				if( e.type == SDL_QUIT )
				{
					quit = true;
					artificeEngine->isActive = false;
				}else if (e.key.keysym.sym == SDLK_ESCAPE && SDL_GetWindowMouseGrab(gWindow) == SDL_TRUE) {
					//free mouse cursor from the window and reveal it
					SDL_SetWindowMouseGrab(gWindow, SDL_FALSE);
					SDL_SetWindowMouseRect(gWindow, NULL);
					SDL_SetRelativeMouseMode(SDL_FALSE);
				}else if (e.type == SDL_MOUSEBUTTONDOWN && SDL_GetWindowMouseGrab(gWindow) == SDL_FALSE) {
					//confine mouse cursor to the window and hide it
					SDL_SetWindowMouseGrab(gWindow, SDL_TRUE);
					SDL_SetWindowMouseRect(gWindow, windowRect);
					SDL_SetRelativeMouseMode(SDL_TRUE);
				}
				//user presses or releases a key
				else if( e.type == SDL_KEYDOWN || e.type == SDL_KEYUP || e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP )
				{
					eventController->processEvent(&e);
				}
				//just a temporary proof-of-concept to modify world on user input
				if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_0) {
					for (auto &model : artificeEngine->modelsToRaster) {
						std::cout << "removing triangle!" << std::endl;
						if (model.modelMesh.tris.size()) model.modelMesh.tris.pop_back();
						artificeEngine->isTouched = true;
					}
				}
			}

			//just a temporary proof-of-concept to update vertices when world is modified
			if (artificeEngine->isTouched)
			{
				artificeEngine->updateVertices(gVBO, gIBO, gVAO, gCubeVBO, gCubeIBO, gCubeVAO);
				artificeEngine->isTouched = false;
			}

			//render
			artificeEngine->render(textureShader, textureIdsMap, cubeMapShader, cubemapIdsMap, gVAO, gCubeVAO);

			//update screen
			SDL_GL_SwapWindow( gWindow );
		}

		//disable text input
		SDL_StopTextInput();
	}

	//free resources and close SDL
	initor.close();

	return 0;
}
