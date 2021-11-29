debug:
	glslangValidator shaders/*
	gcc ./*.cpp \
		./actors/*.cpp\
		-lGLEW -lGL -lglut -lstdc++ -lm -lassimp\
		-g -o application -Wall
	mangohud ./application

build:
	glslangValidator shaders/*
	gcc ./*.cpp \
		./actors/*.cpp\
		-lGLEW -lGL -lglut -lstdc++ -lm -lassimp\
		-g -o application # -Wall