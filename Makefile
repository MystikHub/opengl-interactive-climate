debug:
	glslangValidator shaders/*
	gcc main.cpp actor.cpp camera.cpp -lGLEW -lGL -lglut -lstdc++ -lm -lassimp\
		-g -o application -Wall
	mangohud ./application

build:
	glslangValidator shaders/*
	gcc main.cpp actor.cpp camera.cpp -lGLEW -lGL -lglut -lstdc++ -lm -lassimp\
		-g -o application -Wall