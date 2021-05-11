#OBJS specifies which files to compile as part of the project
#OBJS= src/main.cpp
OBJS= src/blockmain.cpp

#CC specifies which compiler we're using
CC = g++

#INCLUDE_PATHS specifies the additional include paths we'll need
#INCLUDE_PATHS = -I./include/

#LIBRARY_PATHS specifies the additional library paths we'll need
#LIBRARY_PATHS = -L./include/

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
# -Wl,-subsystem,windows gets rid of the console window
COMPILER_FLAGS = -Wall -pthread

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = raytracer.exe

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)