#include "Initiator.h"

CFG cfg;

int main( int argc, char* args[] )
{

	Initiator init(cfg, "level0.lvl");

	//start up SDL and create window, OpenGL context, 3D Engine
	if(!init())
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		SDL_Window* gWindow = init.gWindow;
		SDL_Rect* windowRect = &init.windowRect;
		EventController* eventController = &init.eventController;
		Engine3D* artificeEngine = init.artificeEngine;

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
					eventController->isActive = false;
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
				else if( e.type == SDL_KEYDOWN || e.type == SDL_KEYUP || e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP  || e.type == SDL_MOUSEWHEEL)
				{
					eventController->pushEvent(e);
				}
				//just a temporary proof-of-concept to modify world on user input
				// if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_0) {
				// 	for (auto &model : artificeEngine->modelsToRaster) {
				// 		std::cout << "removing triangle!" << std::endl;
				// 		if (model.modelMesh.tris.size()) model.modelMesh.tris.pop_back();
				// 		artificeEngine->updateVerticesFlag = true;
				// 	}
				// }
			}

			//render
			artificeEngine->render();

			//update screen
			SDL_GL_SwapWindow( gWindow );

			artificeEngine->isTouched = false;
		}

		//disable text input
		SDL_StopTextInput();
	}

	//free resources and close SDL
	init.close();

	return 0;
}
