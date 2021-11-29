#include <cstring>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <string> 
#include <vector>
// Random numbers
#include <stdlib.h>
#include <time.h>

// #include "maths_funcs.h" //Anton's math class
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "actor.hpp"
#include "camera.hpp"
#include "actors/deer_leg.hpp"
#include "actors/bird.hpp"

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;
GLuint shaderProgramID;

// Global input state
struct input_state {
    int location[3];
    int rotation[3];
    int scale[3];
    int camera_location[3];
    int camera_rotation[3];
};
input_state input;

float CAMERA_SPEED = 5;
float MOUSE_SENSITIVITY = 0.5;
float MESH_TRANSLATE_SPEED = 20;

// vector<Actor> actors;
Camera camera = Camera();
vector<Actor*> actors;

chrono::time_point last_time = chrono::high_resolution_clock::time_point::min();
chrono::time_point start_time = chrono::high_resolution_clock::now();

std::string readShaderSource(const std::string& fileName) {
    // std::ifstream file(fileName.c_str()); 
    // if(file.fail()) {
    //     cout << "error loading shader called " << fileName;
    //     exit (1); 
    // } 
    
    // std::stringstream stream;
    // stream << file.rdbuf();
    // file.close();

    // return stream.str();
	fstream filehandle;
	filehandle.open(fileName, ios::in);
	string fileContents ("");
	if(filehandle.is_open()) {
		string newline;
		while(getline(filehandle, newline))
			fileContents += newline + "\n";
		
		filehandle.close();
	}

	char* cstr = new char[fileContents.length()];
	strcpy(cstr, fileContents.c_str());
	return cstr;
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType) {
    // create a shader object
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }
    std::string outShader = readShaderSource(pShaderText);
    const char* pShaderSource = outShader.c_str();

    // Bind the source code to the shader, this happens before compilation
    glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
    // compile the shader and check for errors
    glCompileShader(ShaderObj);
    GLint success;
    // check for shader related errors using glGetShaderiv
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }
    // Attach the compiled shader object to the program object
    glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders() {
    //Start the process of setting up our shaders by creating a program ID
    //Note: we will link all the shaders together into this ID
    shaderProgramID = glCreateProgram();
    if (shaderProgramID == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    // Create two shader objects, one for the vertex, and one for the fragment shader
    AddShader(shaderProgramID, "shaders/simpleVertexShader.vert", GL_VERTEX_SHADER);
    AddShader(shaderProgramID, "shaders/simpleFragmentShader.frag", GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    // After compiling all shader objects and attaching them to the program, we can finally link it
    glLinkProgram(shaderProgramID);
    // check for program related errors using glGetProgramiv
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    // program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
    glValidateProgram(shaderProgramID);
    // check for program related errors using glGetProgramiv
    glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }
    // Finally, use the linked shader program
    // Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
    glUseProgram(shaderProgramID);

    return shaderProgramID;
}

void display() {

    // tell GL to only draw onto a pixel if the shape is closer to the viewer
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glClearColor(0.9f, 0.9f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramID);

    for(unsigned int i = 0; i < actors.size(); i++) {
        actors[i]->renderMesh();
    }

    glutSwapBuffers();
}

void updateScene() {	

    chrono::time_point curr_time = chrono::high_resolution_clock::now();
    if (last_time == chrono::high_resolution_clock::time_point::min())
        last_time = curr_time;

    float delta = chrono::duration_cast<chrono::milliseconds>(curr_time - last_time).count();

    // Sent to each actor's update function
    float delta_seconds = delta * 0.001f;
    float curr_time_seconds = chrono::duration_cast<chrono::milliseconds>(curr_time - start_time).count() * 0.001f;

    last_time = curr_time;

    // Handle kb&m input for input x, y, and z (iterator i)
    for(int i = 0; i < 3; i++) {
        camera.rotation[i] += input.camera_rotation[i] * MOUSE_SENSITIVITY * delta_seconds;

        input.camera_rotation[i] = 0;
    }
    if(camera.rotation.y > M_PI / 2)
        camera.rotation.y = M_PI / 2;
    else if(camera.rotation.y < -M_PI / 2)
        camera.rotation.y = -M_PI / 2;

    // Camera xz movement
    float camera_direction_offset = 0;
    // If moving forwards
    if(input.camera_location[2] == 1) {
        // And to the left
        if(input.camera_location[0] == -1)
            camera_direction_offset = -(M_PI / 4);
        // Or to the right
        else if(input.camera_location[0] == 1)
            camera_direction_offset = M_PI / 4;
        // Otherwise just forwards
        else
            camera_direction_offset = 0;

    // Or if moving backwards
    } else if(input.camera_location[2] == -1) {
        // And to the left
        if(input.camera_location[0] == -1)
            camera_direction_offset = -(M_PI * (3.0f/4.0f));
        // Or to the right
        else if(input.camera_location[0] == 1)
            camera_direction_offset = M_PI * (3.0f/4.0f);
        // Otherwise just backwards
        else
            camera_direction_offset = M_PI;
    // Or just left
    } else if(input.camera_location[0] == -1) {
        camera_direction_offset = -(M_PI / 2);
    // Or just right
    } else if(input.camera_location[0] == 1) {
        camera_direction_offset = M_PI / 2;
    }
    
    int moving = input.camera_location[0] || input.camera_location[2];

    float x_change = sin(camera.rotation[0] + camera_direction_offset);
    float z_change = cos(camera.rotation[0] + camera_direction_offset);

    camera.location[0] += -1 * x_change * CAMERA_SPEED * delta_seconds * moving;
    camera.location[1] += input.camera_location[1] * CAMERA_SPEED * delta_seconds;
    camera.location[2] += z_change * CAMERA_SPEED * delta_seconds * moving;

    // Notify actors that the world has updated
    for(unsigned int i = 0; i < actors.size(); i++) {
        actors[i]->update(curr_time_seconds, delta_seconds, actors);
    }

    // Draw the next frame
    glutPostRedisplay();
}

void resetInput() {
    // Loop through each dimension
    for(int i = 0; i < 3; i++) {
        // Reset the value at each index, except camera rotation
        input.location[i] = 0;
        input.rotation[i] = 0;
        input.scale[i] = 0;
        input.camera_location[i] = 0;
    }
}

int mouse_move_threshold = 1;
int threshLeft = camera.horizontal_center - mouse_move_threshold;
int threshRight = camera.horizontal_center + mouse_move_threshold;
int threshTop = camera.vertical_center + mouse_move_threshold;
int threshBottom = camera.vertical_center - mouse_move_threshold;
int prevX = camera.horizontal_center;
int prevY = camera.vertical_center;

void processMouseLocation(int x, int y) {
    int mouseDeltaX = x - prevX;
    int mouseDeltaY = y - prevY;
    prevX = x;
    prevY = y;
    input.camera_rotation[0] = mouseDeltaX;
    input.camera_rotation[1] = mouseDeltaY;

    // Don't re-center the mouse until it's left some threshold
    if(!((x > threshLeft) && (x < threshRight) && (y < threshTop) && (y > threshBottom)))
        glutWarpPointer(camera.horizontal_center, camera.vertical_center);
}

void keyDownHandler(unsigned char key, int x, int y) {
    switch(key) {
        // WASDQE
        // W: Y+
        case 'w': input.camera_location[2] = 1; break;
        // A: X-
        case 'a': input.camera_location[0] = -1; break;
        // S: Y-
        case 's': input.camera_location[2] = -1; break;
        // D: X+
        case 'd': input.camera_location[0] = 1; break;
        // Q: Z-
        case 'q': input.camera_location[1] = -1; break;
        // E: Z+
        case 'e': input.camera_location[1] = 1; break;
        // Uniform scale
        case 'r':
            input.scale[0] = 1;
            input.scale[1] = 1;
            input.scale[2] = 1;
            break;
        case 'R':
            input.scale[0] = -1;
            input.scale[1] = -1;
            input.scale[2] = -1;
            break;
        // Reset position
        case 'f':
            camera.location = glm::vec3(0.0, 0.0, -40.0);
            camera.rotation = glm::vec3(0.0, 0.0, 0.0);
            break;
        // Press ESC to quit
        case 27: exit(0); break;
        // Toggle projection
        // case 'p': PROJECT_ORTHOGRAPHIC = !PROJECT_ORTHOGRAPHIC; break;
        default: break;
    }

    // printf("[INFO]\tKey is %c, x is %d, and y is %d\n", key, x, y);
    // printf("Location: %d, %d, %d\n", input.location[0], input.location[1], input.location[2]);
    // printf("Rotation: %d, %d, %d\n", input.rotation[0], input.rotation[1], input.rotation[2]);
    // printf("Scale: %d, %d, %d\n", input.scale[0], input.scale[1], input.scale[2]);
    // printf("Camera location: %d, %d, %d\n", input.camera_location[0], input.camera_location[1], input.camera_location[2]);
    // printf("Camera rotation: %f, %f, %f\n", camera.rotation[0], camera.rotation[1], camera.rotation[2]);
    // printf("Project orthographic is %d\n", PROJECT_ORTHOGRAPHIC);
}

void keyUpHandler(unsigned char key, int x, int y) {
    switch(key) {
        // WASDQE
        // W: Y+
        case 'w': input.camera_location[2] = 0; break;
        // A: X-
        case 'a': input.camera_location[0] = 0; break;
        // S: Y-
        case 's': input.camera_location[2] = 0; break;
        // D: X+
        case 'd': input.camera_location[0] = 0; break;
        // Q: Z-
        case 'q': input.camera_location[1] = 0; break;
        // E: Z+
        case 'e': input.camera_location[1] = 0; break;
        // Uniform scale
        case 'r':
            input.scale[0] = 0;
            input.scale[1] = 0;
            input.scale[2] = 0;
            break;
        case 'R':
            input.scale[0] = 0;
            input.scale[1] = 0;
            input.scale[2] = 0;
            break;
        // Reset position
        case 'f':
            camera.location = glm::vec3(0.0, 0.0, -30.0);
            camera.rotation = glm::vec3(0.0, 0.0, 0.0);
            break;
        default: break;
    }
}

void specialKeyHandler(int key, int x, int y) {
    input.location[0] = 0;
    input.location[2] = 0;

    switch(key) {
        case GLUT_KEY_UP:       input.location[2] = -1; break;
        case GLUT_KEY_DOWN:     input.location[2] = 1; break;
        case GLUT_KEY_RIGHT:    input.location[0] = 1; break;
        case GLUT_KEY_LEFT:     input.location[0] = -1; break;
    }
}

void init() {
    // Set up the shaders
    GLuint shaderProgramID = CompileShaders();

    printf("Loading floor...");
    Actor* floor = new Actor(&camera);
    floor->shaderProgramID = shaderProgramID;
    floor->diffuse_texture = "materials/textures/blue_floor_tiles_01_diff_4k.jpg";
    floor->specularity = 256.0f;
    floor->loadMesh("models/floor.dae");
    floor->setupBufferObjects();
    actors.push_back(floor);

    printf("Loading benches wood...");
    Actor* benches_wood = new Actor(&camera);
    benches_wood->shaderProgramID = shaderProgramID;
    benches_wood->diffuse_texture = "materials/textures/weathered_brown_planks_diff_4k.jpg";
    benches_wood->loadMesh("models/benches_wood.dae");
    benches_wood->setupBufferObjects();
    actors.push_back(benches_wood);

    printf("Loading planter and walls...");
    Actor* planter_and_walls = new Actor(&camera);
    planter_and_walls->shaderProgramID = shaderProgramID;
    planter_and_walls->diffuse_texture = "materials/textures/concrete_floor_worn_001_diff_4k.jpg";
    planter_and_walls->loadMesh("models/planter_and_walls.dae");
    planter_and_walls->specularity = 128.0f;
    planter_and_walls->setupBufferObjects();
    actors.push_back(planter_and_walls);

    printf("Loading bench metal...");
    Actor* bench_metal = new Actor(&camera);
    bench_metal->shaderProgramID = shaderProgramID;
    bench_metal->diffuse_texture = "materials/textures/Metal038_4K_Color.jpg";
    bench_metal->loadMesh("models/bench_metal.dae");
    bench_metal->specularity = 512.0f;
    bench_metal->setupBufferObjects();
    actors.push_back(bench_metal);

    printf("Loading mountain...");
    Actor* mountain = new Actor(&camera);
    mountain->shaderProgramID = shaderProgramID;
    mountain->diffuse_texture = "materials/textures/13_c.jpg";
    mountain->loadMesh("models/mountain.dae");
    mountain->setupBufferObjects();
    actors.push_back(mountain);

    printf("Loading bird...");
    Actor* bird = new Actor(&camera);
    bird->shaderProgramID = shaderProgramID;
    bird->diffuse_texture = "materials/textures/Metal038_4K_Color.jpg";
    bird->loadMesh("models/bird-static.dae");
    bird->setupBufferObjects();
    actors.push_back(bird);

    printf("Loading deer torso...");
    Actor* deer_torso = new Actor(&camera);
    deer_torso->shaderProgramID = shaderProgramID;
    deer_torso->loadMesh("models/deer-torso.dae");
    deer_torso->diffuse_texture = "materials/textures/doe-body-1.jpg";
    deer_torso->location.z = 7;
    deer_torso->rotation.y = M_PI;
    deer_torso->setupBufferObjects();
    actors.push_back(deer_torso);

    printf("Loading deer leg upper...");
    DeerLeg* deer_leg_upper = new DeerLeg(&camera);
    deer_leg_upper->shaderProgramID = shaderProgramID;
    deer_leg_upper->loadMesh("models/deer-leg-upper.dae");
    deer_leg_upper->diffuse_texture = "materials/textures/doe-body-2.jpg";
    deer_leg_upper->parent = deer_torso;
    deer_leg_upper->location = glm::vec3(-0.112856f, 0.689876f, 0.280958f);
    deer_leg_upper->setupBufferObjects();
    actors.push_back(deer_leg_upper);

    printf("Loading deer leg lower...");
    DeerLeg* deer_leg_lower = new DeerLeg(&camera);
    deer_leg_lower->shaderProgramID = shaderProgramID;
    deer_leg_lower->loadMesh("models/deer-leg-lower.dae");
    deer_leg_lower->diffuse_texture = "materials/textures/doe-body-3.jpg";
    deer_leg_lower->parent = deer_leg_upper;
    deer_leg_lower->location = glm::vec3(0.019246f, -0.265585f, 0.020285f);
    deer_leg_lower->setupBufferObjects();
    actors.push_back(deer_leg_lower);

    printf("Loading deer head...");
    Actor* deer_head = new Actor(&camera);
    deer_head->shaderProgramID = shaderProgramID;
    deer_head->loadMesh("models/deer-head.dae");
    deer_head->diffuse_texture = "materials/textures/doe-head.jpg";
    deer_head->parent = deer_torso;
    deer_head->location = glm::vec3(0.0f, 0.905768f, 0.437629f);
    deer_head->setupBufferObjects();
    actors.push_back(deer_head);

    printf("Loading planter soil...");
    Actor* planter_soil = new Actor(&camera);
    planter_soil->shaderProgramID = shaderProgramID;
    planter_soil->loadMesh("models/planter_soil.dae");
    planter_soil->diffuse_texture = "materials/textures/dry_mud_field_001_diff_4k.jpg";
    planter_soil->specularity = 4.0f;
    planter_soil->setupBufferObjects();
    actors.push_back(planter_soil);

    printf("Setting up bird crowd...");
    // Spawn some birds for a crowd
    // Don't reload the data each time
    ModelData* bird_model;
    bool bird_model_loaded = false;

    // Bounds for birds' starting positions
    // Box is 10x10x7m
    glm::vec3 birds_start = glm::vec3(-5.0f, 7.0f, -5.0f);
    glm::vec3 birds_end = glm::vec3(5.0f, 1.0f, 5.0f);
    srand(0);               // Debug seed, TODO: remove before submission

    for(int i = 0; i < 20; i++) {
        Bird* bird = new Bird(&camera);
        bird->shaderProgramID = shaderProgramID;
        if(!bird_model_loaded) {
            bird->loadMesh("models/bird.dae");
            bird_model = bird->mesh;
            bird_model_loaded = true;
        } else {
            bird->mesh = bird_model;
        }
        bird->diffuse_texture = "materials/textures/Metal038_4K_Color.jpg";
        bird->setupBufferObjects();

        // Bird starting location
        bird->location.x = birds_start.x + (((float) rand() / (float) RAND_MAX) * birds_end.x);
        bird->location.y = birds_start.y + (((float) rand() / (float) RAND_MAX) * birds_end.y);
        bird->location.z = birds_start.z + (((float) rand() / (float) RAND_MAX) * birds_end.z);

        actors.push_back(bird);
    }

    camera.location.y = -1.6;
}

int main(int argc, char** argv){

    // Set up the window
    glutInit(&argc, argv);
    // glutInitContextVersion(3, 3);
    // glutInitContextFlags(GLUT_DEBUG);
    // glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(camera.width, camera.height);
    glutCreateWindow("OpenGL simple hierarchy");

    // Tell glut where the display function is
    glutDisplayFunc(display);
    glutIdleFunc(updateScene);

    // Point GLUT to keyboard and mouse handler
    glutKeyboardFunc(keyDownHandler);
    glutKeyboardUpFunc(keyUpHandler);
    glutMotionFunc(processMouseLocation);
    glutPassiveMotionFunc(processMouseLocation);
    glutSpecialFunc(specialKeyHandler);
    glutSetCursor(GLUT_CURSOR_NONE);

     // A call to glewInit() must be done after glut is initialized!
    glewExperimental = GL_TRUE; //for non-lab machines, this line gives better modern GL support
    GLenum res = glewInit();

    // Check for any errors
    if (res != GLEW_OK) {
      fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
      return 1;
    }

    // Set up your objects and shaders
    init();

    // Begin infinite event loop
    glutMainLoop();
    return 0;
}
