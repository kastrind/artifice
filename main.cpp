#include "Initiator.h"
#include "EventController.h"
#include "Level.h"
#include "Engine3D.h"
#include <chrono>

CFG cfg;

int main( int argc, char* args[] )
{

	Initiator init(&cfg);

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
		EventController eventController(cfg.MOUSE_SENSITIVITY_X, cfg.MOUSE_SENSITIVITY_Y,
										cfg.KEY_ASCEND, cfg.KEY_DESCEND, cfg.KEY_LEFT,
										cfg.KEY_RIGHT, cfg.KEY_FORWARD, cfg.KEY_BACKWARD,
										cfg.KEY_PLACE, cfg.KEY_REMOVE, cfg.KEY_NEXT,
										cfg.KEY_PREVIOUS, cfg.KEY_JUMP);

		//create a default camera object
		Camera camera;
		//camera.offset = glm::vec3(0.0f, 0.5f, -0.8f);
		
		//instantiate the game engine
		Engine3D* artificeEngine = new Engine3D(gWindow, camera,
												cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT,
												cfg.NEAR, cfg.FAR, cfg.FOV, cfg.DOF,
												cfg.PERSON_WIDTH, //horizontal colliding distance
												cfg.PERSON_HEIGHT, //vertical colliding distance
												cfg.GRAVITATIONAL_PULL,
												cfg.JUMP_SPEED_FACTOR,
												cfg.PERSON_SPEED_FACTOR,
												cfg.USER_MODE, &eventController);

		Level level;
		level.load(cfg.LEVELS_PATH + cfg.PATH_SEP + "level0.lvl");
		artificeEngine->setLevel(&level);

		//start the 3D engine
		printf( "Starting Engine thread...\n" );
		std::thread engineThread = artificeEngine->startEngine();

		//main loop flag
		bool quit = false;

		//event handler
		SDL_Event e;
		
		//enable text input
		SDL_StartTextInput();

		//define a duration of 3 milliseconds
		std::chrono::milliseconds duration_milliseconds(25);

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
					eventController.decodeEvent(&e);
					//eventController.bufferKeysPressed();

					//ImGui_ImplSDL2_ProcessEvent(&e); // Forward your event to backend
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
			
			//sleep for 3 milliseconds
			std::this_thread::sleep_for(duration_milliseconds);
		}

		//disable text input
		SDL_StopTextInput();

		printf("Stopping threads...\n");

		engineThread.join();
	}

	//free resources and close SDL
	init.close();

	return 0;
}
