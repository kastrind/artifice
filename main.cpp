#include "Initiator.h"
#include "EventController.h"
#include "Level.h"
#include "Engine3D.h"
#include <thread>
#include <chrono>

CFG cfg;

int main( int argc, char* args[] )
{

	Initiator init(cfg);

	//start up SDL and create window
	if(!init())
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		SDL_Window* gWindow = init.gWindow;
		SDL_Rect* windowRect = &init.windowRect;

		//input event controller
		EventController eventController;

		//start listening for input events
		// printf( "Starting input events listener thread...\n" );
		// std::thread eventListenerThread = eventController.startListening();
		
		//instantiate the game engine
		Engine3D* artificeEngine = new Engine3D(gWindow, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT,
												cfg.NEAR, cfg.FAR, cfg.FOV, cfg.DOF,
												cfg.COLLIDING_DISTANCE,
												cfg.GRAVITATIONAL_PULL,
												cfg.CAMERA_SPEED_FACTOR,
												cfg.USER_MODE, &eventController);

		Level level;
		level.load(cfg.LEVELS_PATH + "\\level0.lvl");
		artificeEngine->setLevel(&level);

		//artificeEngine->renderingThread();

		//start the 3D engine
		printf( "Starting Engine thread...\n" );
		std::thread engineThread = artificeEngine->startEngine();

		//main loop flag
		bool quit = false;

		//event handler
		SDL_Event e;
		
		//enable text input
		SDL_StartTextInput();

		// Define a duration of 3 milliseconds
		std::chrono::milliseconds duration_milliseconds(3);

		//while application is running
		while( !quit )
		{
			eventController.clearMouseMotionState();

			//handle events on queue
			while( SDL_PollEvent( &e ) != 0 )
			{

				//user requests quit
				if( e.type == SDL_QUIT )
				{
					quit = true;
					artificeEngine->isActive = false;
					eventController.isActive = false;
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
					//eventController.pushEvent(e);
					eventController.processEvent(&e);
				}
				//just a temporary proof-of-concept to modify world on user input
				// if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_0) {
				// 	for (auto &model : artificeEngine->modelsToRender) {
				// 		std::cout << "removing triangle!" << std::endl;
				// 		if (model.modelMesh.tris.size()) model.modelMesh.tris.pop_back();
				// 		artificeEngine->updateVerticesFlag = true;
				// 	}
				// }
			}

			//render
			//if (artificeEngine->isActive) artificeEngine->render();

			//update screen
			//SDL_GL_SwapWindow( gWindow );

			// Sleep for 3 milliseconds
    		std::this_thread::sleep_for(duration_milliseconds);
			//std::cout << "mouseDistX: " << eventController.getMouseDistanceX() << " ! " << std::endl;
		}

		//disable text input
		SDL_StopTextInput();

		printf("Stopping threads...\n");

		engineThread.join();
		//eventListenerThread.join();
	}

	//free resources and close SDL
	init.close();

	return 0;
}
