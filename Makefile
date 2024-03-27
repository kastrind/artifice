#SRCS specifies which files to compile as part of the project
SRCS = main.cpp Initiator.cpp Level.cpp ShaderProgram.cpp ArtificeShaderProgram.cpp EventController.cpp Engine3D.cpp
OBJS = main.o Initiator.o Level.o ShaderProgram.o ArtificeShaderProgram.o EventController.o Engine3D.o

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
	$(CC) -c Engine3D.cpp $(COMPILER_FLAGS)

clean : 
	rm *.o *.exe || true

objs : clean main init level shader event engine

build : objs link


