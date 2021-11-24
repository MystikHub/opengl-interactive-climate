debug:
	glslangValidator shaders/*
	gcc main.cpp actor.cpp camera.cpp -lGLEW -lGL -lglut -lstdc++ -lm -lassimp\
		-g -o application -Wall
	# mangohud ./application
	./application
	rm ./application

build:
	glslangValidator shaders/*
	gcc main.cpp actor.cpp camera.cpp -lGLEW -lGL -lglut -lstdc++ -lm -lassimp\
		-g -o application -Wall