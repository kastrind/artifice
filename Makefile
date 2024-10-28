#SRCS specifies which files to compile as part of the project

IMGUI_SRCS = imgui/imgui.cpp imgui/imgui_impl_sdl2.cpp imgui/imgui_impl_opengl3.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/imgui_draw.cpp imgui/imgui_stdlib.cpp imgui/imgui_demo.cpp
IMGUI_OBJS = imgui.o imgui_impl_sdl2.o imgui_impl_opengl3.o  imgui_tables.o imgui_widgets.o imgui_draw.o imgui_stdlib.o imgui_demo.o

SRCS = main.cpp Initiator.cpp Level.cpp ShaderProgram.cpp ArtificeShaderProgram.cpp EventController.cpp Engine3D.cpp Engine3DEd.cpp #$(IMGUI_SRCS)
OBJS = main.o Initiator.o Level.o ShaderProgram.o ArtificeShaderProgram.o EventController.o Engine3D.o Engine3DEd.o #$(IMGUI_OBJS)

#CC specifies which compiler we're using
CC = g++

#INCLUDE_PATHS specifies the additional include paths we'll need
INCLUDE_PATHS = -IC:\msys64\mingw64\include

#LIBRARY_PATHS specifies the additional library paths we'll need
LIBRARY_PATHS = -LC:\msys64\mingw64\lib

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
# -Wl,-subsystem,windows gets rid of the console window
COMPILER_FLAGS = -w #-Wl,-subsystem,windows

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lglew32 -lopengl32

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = artifice

#This is the target that compiles our executable
all : $(SRCS)
	$(CC) $(SRCS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)

link :
	$(CC) ${OBJS} $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LINKER_FLAGS) -o ${OBJ_NAME}

gui :
	$(CC) -c $(IMGUI_SRCS) $(COMPILER_FLAGS)

main :
	$(CC) -c main.cpp $(COMPILER_FLAGS)

init :
	$(CC) -c Initiator.cpp $(COMPILER_FLAGS)

level :
	$(CC) -c Level.cpp $(COMPILER_FLAGS)

shader :
	$(CC) -c ShaderProgram.cpp ArtificeShaderProgram.cpp $(COMPILER_FLAGS)

event :
	$(CC) -c EventController.cpp $(COMPILER_FLAGS)

engine :
	$(CC) -c Engine3D.cpp Engine3DEd.cpp $(COMPILER_FLAGS)

clean :
	rm *.o *.exe || true

#objs : clean gui main init level shader event engine
objs : clean main init level shader event engine

build : objs link


