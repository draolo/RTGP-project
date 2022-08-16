/*
Es02b: Procedural shaders 1 (random patterns)
- procedural shaders for noise patterns, pressing keys from 1 to 7

N.B. 1) 
In this example we use Shaders Subroutines to do shader swapping:
http://www.geeks3d.com/20140701/opengl-4-shader-subroutines-introduction-3d-programming-tutorial/
https://www.lighthouse3d.com/tutorials/glsl-tutorial/subroutines/
https://www.khronos.org/opengl/wiki/Shader_Subroutine

In other cases, an alternative could be to consider Separate Shader Objects:
https://www.informit.com/articles/article.aspx?p=2731929&seqNum=7
https://www.khronos.org/opengl/wiki/Shader_Compilation#Separate_programs
https://riptutorial.com/opengl/example/26979/load-separable-shader-in-cplusplus

N.B. 2) no texturing in this version of the classes

N.B. 3) to test different parameters of the shaders, it is convenient to use some GUI library, like e.g. Dear ImGui (https://github.com/ocornut/imgui)

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2020/2021
Master degree in Computer Science
Universita' degli Studi di Milano
*/

/*
OpenGL coordinate system (right-handed)
positive X axis points right
positive Y axis points up
positive Z axis points "outside" the screen


                              Y
                              |
                              |
                              |________X
                             /
                            /
                           /
                          Z
*/

// Std. Includes
#include <string>
#include <set>
#include <algorithm>

// Loader estensions OpenGL
// http://glad.dav1d.de/
// THIS IS OPTIONAL AND NOT REQUIRED, ONLY USE THIS IF YOU DON'T WANT GLAD TO INCLUDE windows.h
// GLAD will include windows.h for APIENTRY if it was not previously defined.
// Make sure you have the correct definition for APIENTRY for platforms which define _WIN32 but don't use __stdcall
#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

#include <glad/glad.h>

// GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

// another check related to OpenGL loader
// confirm that GLAD didn't include windows.h
#ifdef _WINDOWS_
    #error windows.h was included!
#endif

// classes developed during lab lectures to manage shaders and to load models
#include <utils/shader_v1.h>
#include <utils/model_v1.h>
#include <utils/camera.h>
#include <utils/physics_v1.h>
#include <utils/gameObject.h>
#include <utils/bullet.h>

// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// we include the library for images loading
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#define MAX_HIT 16
#define NUMBER_OF_FBO 2

// dimensions of application's window
GLuint screenWidth = 800, screenHeight = 600;

glm::mat4 view, projection;

// we create a camera. We pass the initial position as a parameter to the constructor. The last boolean tells that we want a camera "anchored" to the ground
Camera camera(glm::vec3(0.0f, 0.0f, 7.0f), GL_TRUE);

GLfloat frequency = 12.0;
GLfloat power = 0.0;
GLfloat hitDamage =4.0;
// number of harmonics (used in the turbulence-based subroutines)
GLfloat harmonics = 4.0;

GLfloat powers[MAX_HIT];
GLfloat hitPoints[2*MAX_HIT];
int hit_index=0;

// we initialize an array of booleans for each keybord key
bool keys[1024];

// callback function for mouse and keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
// if one of the WASD keys is pressed, we call the corresponding method of the Camera class
void apply_camera_movements();

void shoot();
void shootToPlayer(glm::vec3 from);

void checkForCollision();

void update_hits(float deltaTime);

bool add_hit(float normx, float normy);

GLint LoadTextureCube(string path, const string format);

// texture unit for the cube map
GLuint textureCube;

// index of the current shader subroutine (= 0 in the beginning)
GLuint current_subroutine = 0;
// a vector for all the shader subroutines names used and swapped in the application
vector<std::string> shaders;

// the name of the subroutines are searched in the shaders, and placed in the shaders vector (to allow shaders swapping)
void SetupShader(int shader_program);

// print on console the name of current shader subroutine
void PrintCurrentShader(int subroutine);

// parameters for time computation
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// we need to store the previous mouse position to calculate the offset with the current frame
GLfloat lastX, lastY;
// we will use these value to "pass" the cursor position to the keyboard callback, in order to determine the bullet trajectory
double cursorX,cursorY;


// when rendering the first frame, we do not have a "previous state" for the mouse, so we need to manage this situation
bool firstMouse = true;

// rotation angle on Y axis
GLfloat orientationY = 0.0f;
// rotation speed on Y axis
GLfloat spin_speed = 30.0f;
// boolean to start/stop animated rotation on Y angle
GLboolean spinning = GL_TRUE;

glm::vec3 lightPos0 = glm::vec3(5.0f, 10.0f, 10.0f);
//GLfloat lightColor[] = {1.0f,1.0f,1.0f};

// diffusive, specular and ambient components
GLfloat diffuseColor[] = {1.0f,0.0f,0.0f};
GLfloat specularColor[] = {1.0f,1.0f,1.0f};
GLfloat ambientColor[] = {0.1f,0.1f,0.1f};
// weights for the diffusive, specular and ambient components
GLfloat Kd = 0.5f;
GLfloat Ks = 0.4f;
GLfloat Ka = 0.1f;

GLfloat planeMaterial[] = {0.0f,0.5f,0.0f};


// boolean to activate/deactivate wireframe rendering
GLboolean wireframe = GL_FALSE;

// velocity for noise animation effect (for the animated subroutine)
GLfloat speed = 5.0;

// color to be passed as uniform to the shader of the plane
GLfloat planeColor[] = {0.0,0.5,0.0};
GLfloat objectColor[] = {0.5,0.0,0.0};

GLfloat bullet_color[] = {1.0f,1.0f,0.0f};
// dimension of the bullets (global because we need it also in the keyboard callback)
glm::vec3 bullet_size = glm::vec3(0.2f, 0.2f, 0.2f);
Model *bulletModel;

// we set the maximum delta time for the update of the physical simulation
GLfloat maxSecPerFrame = 1.0f / 60.0f;

// instance of the physics class
Physics bulletSimulation;

float rectangleVertices[] =
{
	// Coords    // texCoords
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f,

	 1.0f,  1.0f,  1.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f
};

vector<GameObject*> scene;

/////////////////// MAIN function ///////////////////////
int main()
{
  // Initialization of OpenGL context using GLFW
  glfwInit();
  // We set OpenGL specifications required for this application
  // In this case: 4.1 Core
  // If not supported by your graphics HW, the context will not be created and the application will close
  // N.B.) creating GLAD code to load extensions, try to take into account the specifications and any extensions you want to use,
  // in relation also to the values indicated in these GLFW commands
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  // we set if the window is resizable
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  // we create the application's window
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Project Milanesi 939908", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // we put in relation the window and the callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window,mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_callback);


    // we disable the mouse cursor
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD tries to load the context set by GLFW
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    // we define the viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // we enable Z test
    glEnable(GL_DEPTH_TEST);

    //the "clear" color for the frame buffer
    glClearColor(0.26f, 0.46f, 0.98f, 1.0f);

    // we create the Shader Program used for the plane

    Shader basic_shader("shaders/09_illumination_models.vert", "shaders/10_illumination_models.frag");

    // we create the Shader Program used for objects (which presents different subroutines we can switch)
    Shader horizontal_blur_shader = Shader("shaders/framebuffer.vert", "shaders/hblur.frag");
    Shader vertical_blur_shader = Shader("shaders/framebuffer.vert", "shaders/vblur.frag");
   
    // we create the Shader Program used for the environment map
    Shader skybox_shader("shaders/17_skybox.vert", "shaders/18_skybox.frag");

   
    // we parse the Shader Program to search for the number and names of the subroutines. 
    // the names are placed in the shaders vector
    SetupShader(basic_shader.Program);
    // we print on console the name of the first subroutine used
    PrintCurrentShader(current_subroutine);
    //SetupShader(horizontal_blur_shader.Program);
    // we print on console the name of the first subroutine used


    glUniform1i(glGetUniformLocation(horizontal_blur_shader.Program, "screenTexture"), 0);

    glEnable(GL_DEPTH_TEST);

	// Enables Cull Facing //cull face is not good :D
	//glEnable(GL_CULL_FACE);
	// Keeps front faces
	glCullFace(GL_FRONT);
	// Uses counter clock-wise standard
	glFrontFace(GL_CCW);

    // Specify the color of the background
    
    unsigned int rectVAO, rectVBO;
	glGenVertexArrays(1, &rectVAO);
	glGenBuffers(1, &rectVBO);
	glBindVertexArray(rectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    unsigned int FBO[NUMBER_OF_FBO]; 
	glGenFramebuffers(NUMBER_OF_FBO, FBO);

    // Create Framebuffer Texture
	unsigned int framebufferTexture[NUMBER_OF_FBO];
	glGenTextures(NUMBER_OF_FBO, framebufferTexture);

    // Create Render Buffer Object
	unsigned int RBO[NUMBER_OF_FBO];
	glGenRenderbuffers(NUMBER_OF_FBO, RBO);

    for(int i=0;i<NUMBER_OF_FBO;i++){
	glBindFramebuffer(GL_FRAMEBUFFER, FBO[i]);
	glBindTexture(GL_TEXTURE_2D, framebufferTexture[i]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture[i], 0);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO[i]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO[i]);
   
        // Error checking framebuffer
	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer "<<i<<" error: " << fboStatus << std::endl;
    }
    // Bind the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, FBO[0]);

    
    // we load the cube map (we pass the path to the folder containing the 6 views)
    textureCube = LoadTextureCube("../../textures/cube/Park2/", "png");

// we load the model(s) (code of Model class is in include/utils/model_v1.h)
     // we load the model(s) (code of Model class is in include/utils/model_v1.h)
    Model skyboxModel("../../models/cube.obj"); // used for the environment map

    Model cubeModel("../../models/cube.obj");
    Model sphereModel("../../models/sphere.obj");
    Model bunnyModel("../../models/bunny_lp.obj");

    bulletModel=&sphereModel;

    // Projection matrix: FOV angle, aspect ratio, near and far planes
    projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);
    // View matrix (=camera): position, view direction, camera "up" vector
    view = glm::lookAt(glm::vec3(0.0f, 0.0f, 7.0f), glm::vec3(0.0f, 0.0f, -7.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // Model and Normal transformation matrices for the objects in the scene: we set to identity
    glm::mat4 sphereModelMatrix = glm::mat4(1.0f);
    glm::mat3 sphereNormalMatrix = glm::mat3(1.0f);
    glm::mat4 cubeModelMatrix = glm::mat4(1.0f);
    glm::mat3 cubeNormalMatrix = glm::mat3(1.0f);
    glm::mat4 bunnyModelMatrix = glm::mat4(1.0f);
    glm::mat3 bunnyNormalMatrix = glm::mat3(1.0f);
    glm::mat4 planeModelMatrix = glm::mat4(1.0f);
    glm::mat3 planeNormalMatrix = glm::mat3(1.0f);
    
    glm::vec3 plane_pos = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 plane_size = glm::vec3(200.0f, 0.1f, 200.0f);
    glm::vec3 plane_rot = glm::vec3(0.0f, 0.0f, 0.0f);


    GameObject plane(plane_pos,plane_size, plane_rot, &cubeModel);
    GameObject sphere(glm::vec3(-3.0f, 0.0f, 0.0f),glm::vec3(0.8,0.8,0.8),glm::vec3(0.0,.0,0.0),&sphereModel);
    GameObject cube(glm::vec3(0.0f, 3.0f, 0.0f),glm::vec3(1,1,1),glm::vec3(0.0,.0,0.0),&cubeModel);
    GameObject ind(glm::vec3(2.0f, 2.0f, 2.0f),glm::vec3(.2,.2,.2),glm::vec3(0.0,.0,0.0),&cubeModel);
    GameObject bunny(glm::vec3(3.0f, 0.0f, 0.0f),glm::vec3(0.3,0.3,0.3),glm::vec3(0.0,.0,0.0),&bunnyModel);

    sphere.setColor3(objectColor);
    cube.setColor3(objectColor);
    bunny.setColor3(objectColor);  
    ind.setColor3(objectColor); 
    plane.setColor3(planeColor); 

    plane.addRigidbody(bulletSimulation,BOX,0,0.3,0.3);
    cube.addRigidbody(bulletSimulation,SHAPE,2,0.3,0.3);
    sphere.addRigidbody(bulletSimulation,SPHERE,2,0.3,0.3);
    bunny.addRigidbody(bulletSimulation,SHAPE,0,0.3,0.3);
    camera.addRigidbody(bulletSimulation,SPHERE,0.1,0.,.0);

    //scene.push_back(plane);
    scene.push_back(&sphere);
    scene.push_back(&cube);
    scene.push_back(&bunny);
    scene.push_back(&ind);

    //set up the hit manager
    for(int i=0;i<MAX_HIT;i++){
        powers[i]=0.0f;
    }

    // we set the maximum delta time for the update of the physical simulation
    GLfloat maxSecPerFrame = 1.0f / 60.0f;

    // Rendering loop: this code is executed at each frame
    while(!glfwWindowShouldClose(window))
    {
       // Bind the custom framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, FBO[0]);

        glClearColor(0.26f, 0.46f, 0.98f, 1.0f);
        
		// Clean the back buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Enable depth testing since it's disabled when drawing the framebuffer rectangle
		glEnable(GL_DEPTH_TEST);
        basic_shader.Use();
        // we determine the time passed from the beginning
        // and we calculate the time difference between current frame rendering and the previous one
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check is an I/O event is happening
        glfwPollEvents();

        // we apply FPS camera movements
        apply_camera_movements();
        // View matrix (=camera): position, view direction, camera "up" vector
        view = camera.GetViewMatrix();

        // we "clear" the frame and z buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // we set the rendering mode
        if (wireframe)
            // Draw in wireframe
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // if animated rotation is activated, than we increment the rotation angle using delta time and the rotation speed parameter
        if (spinning)
            orientationY+=(deltaTime*spin_speed);

        update_hits(deltaTime);

        // we update the physics simulation. We must pass the deltatime to be used for the update of the physical state of the scene. The default value for Bullet is 60 Hz, for lesser deltatime the library interpolates and does not calculate the simulation. In this example, we use deltatime from the last rendering: if it is < 1\60 sec, than we use it, otherwise we use the deltatime we have set above
        // we also set the max number of substeps to consider for the simulation (=10)
        bulletSimulation.dynamicsWorld->stepSimulation((deltaTime < maxSecPerFrame ? deltaTime : maxSecPerFrame),10);
        checkForCollision();


        /////////////////// PLANE ////////////////////////////////////////////////
        // We render a plane under the objects. We apply the fullcolor shader to the plane, and we do not apply the rotation applied to the other objects.
        basic_shader.Use();
        // for the plane, we use only Lambert model.
        // Thus, we search inside the Shader Program the name of the subroutine, and we get the numerical index
        GLuint index = glGetSubroutineIndex(basic_shader.Program, GL_FRAGMENT_SHADER, "Lambert");
        // we activate the subroutine using the index
        glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &index);

        // we pass projection and view matrices to the Shader Program of the plane
        glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));

        // we determine the position in the Shader Program of the uniform variables
        GLint pointLightLocation = glGetUniformLocation(basic_shader.Program, "pointLightPosition");
        GLint matDiffuseLocation = glGetUniformLocation(basic_shader.Program, "diffuseColor");
        GLint kdLocation = glGetUniformLocation(basic_shader.Program, "Kd");

        // we assign the value to the uniform variables
        glUniform3fv(pointLightLocation, 1, glm::value_ptr(lightPos0));
        glUniform3fv(matDiffuseLocation, 1, planeMaterial);
        glUniform1f(kdLocation, Kd);


        // we render the plane
        plane.Draw(view,basic_shader);

        /////////////////// OBJECTS ////////////////////////////////////////////////
        // We use the same Shader Program for the objects, but in this case we will do shaders swapping
        // we search inside the Shader Program the name of the subroutine currently selected, and we get the numerical index
        index = glGetSubroutineIndex(basic_shader.Program, GL_FRAGMENT_SHADER, shaders[current_subroutine].c_str());
        // we activate the subroutine using the index (this is where shaders swapping happens)
        glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &index);

        // we determine the position in the Shader Program of the uniform variables
        GLint matAmbientLocation = glGetUniformLocation(basic_shader.Program, "ambientColor");
        GLint matSpecularLocation = glGetUniformLocation(basic_shader.Program, "specularColor");
        GLint kaLocation = glGetUniformLocation(basic_shader.Program, "Ka");
        GLint ksLocation = glGetUniformLocation(basic_shader.Program, "Ks");

        // we assign the value to the uniform variables
        glUniform3fv(matDiffuseLocation, 1, diffuseColor);
        glUniform3fv(matAmbientLocation, 1, ambientColor);
        glUniform3fv(matSpecularLocation, 1, specularColor);
        glUniform1f(kaLocation, Ka);
        glUniform1f(ksLocation, Ks);

        for (auto gameObject : scene) // access by reference to avoid copying
        {  
            gameObject->Draw(view,basic_shader);
        }
        
        /////////////////// SKYBOX ////////////////////////////////////////////////
        // we use the cube to attach the 6 textures of the environment map.
        // we render it after all the other objects, in order to avoid the depth tests as much as possible.
        // we will set, in the vertex shader for the skybox, all the values to the maximum depth. Thus, the environment map is rendered only where there are no other objects in the image (so, only on the background).
        //Thus, we set the depth test to GL_LEQUAL, in order to let the fragments of the background pass the depth test (because they have the maximum depth possible, and the default setting is GL_LESS)
        glDepthFunc(GL_LEQUAL);
        skybox_shader.Use();
        // we activate the cube map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureCube);
         // we pass projection and view matrices to the Shader Program of the skybox
        glUniformMatrix4fv(glGetUniformLocation(skybox_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        // to have the background fixed during camera movements, we have to remove the translations from the view matrix
        // thus, we consider only the top-left submatrix, and we create a new 4x4 matrix
        view = glm::mat4(glm::mat3(view)); // Remove any translation component of the view matrix
        glUniformMatrix4fv(glGetUniformLocation(skybox_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));

        // we determine the position in the Shader Program of the uniform variables
        GLint textureLocation = glGetUniformLocation(skybox_shader.Program, "tCube");
        // we assign the value to the uniform variable
        glUniform1i(textureLocation, 0);

        // we render the cube with the environment map
        skyboxModel.Draw();
        // we set again the depth test to the default operation for the next frame
        glDepthFunc(GL_LESS);

        {
        // Faccio lo swap tra back e front buffer
        // Bind the intermediate framebuffer
    	glBindFramebuffer(GL_FRAMEBUFFER, FBO[1]);

		horizontal_blur_shader.Use();
        GLint frequencyLocation = glGetUniformLocation(horizontal_blur_shader.Program, "frequency");
        GLint hitPointNumberLocation = glGetUniformLocation(horizontal_blur_shader.Program, "contactPointNumber");
        GLint hitPointLocation = glGetUniformLocation(horizontal_blur_shader.Program, "normalizedContactPoints");
        GLint powersLocation = glGetUniformLocation(horizontal_blur_shader.Program, "powers");
        

        GLint harmonicsLocation = glGetUniformLocation(horizontal_blur_shader.Program, "harmonics");

        // we assign the value to the uniform variable
        glUniform1i(hitPointNumberLocation, MAX_HIT);
        glUniform1f(frequencyLocation, frequency);
        glUniform1fv(powersLocation,MAX_HIT, powers);
        glUniform2fv(hitPointLocation,MAX_HIT, hitPoints);
        glUniform1f(harmonicsLocation, harmonics);


		// Draw the framebuffer rectangle
		glBindVertexArray(rectVAO);
		glDisable(GL_DEPTH_TEST); // prevents framebuffer rectangle from being discarded
		glBindTexture(GL_TEXTURE_2D, framebufferTexture[0]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        {
    	glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        vertical_blur_shader.Use();
        GLint frequencyLocation = glGetUniformLocation(vertical_blur_shader.Program, "frequency");
        GLint hitPointNumberLocation = glGetUniformLocation(vertical_blur_shader.Program, "contactPointNumber");
        GLint hitPointLocation = glGetUniformLocation(vertical_blur_shader.Program, "normalizedContactPoints");
        GLint powersLocation = glGetUniformLocation(vertical_blur_shader.Program, "powers");
        

        GLint harmonicsLocation = glGetUniformLocation(vertical_blur_shader.Program, "harmonics");

        // we assign the value to the uniform variable
        glUniform1i(hitPointNumberLocation, MAX_HIT);
        glUniform1f(frequencyLocation, frequency);
        glUniform1fv(powersLocation,MAX_HIT, powers);
        glUniform2fv(hitPointLocation,MAX_HIT, hitPoints);
        glUniform1f(harmonicsLocation, harmonics);


		// Draw the framebuffer rectangle
		glBindVertexArray(rectVAO);
		glDisable(GL_DEPTH_TEST); // prevents framebuffer rectangle from being discarded
		glBindTexture(GL_TEXTURE_2D, framebufferTexture[1]);
		glDrawArrays(GL_TRIANGLES, 0, 6);

        }
		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);

    }

    // when I exit from the graphics loop, it is because the application is closing
    // we delete the Shader Programs
    basic_shader.Delete();
    horizontal_blur_shader.Delete();
    // we close and delete the created context
    glfwTerminate();
    return 0;
}


//////////////////////////////////////////
// The function parses the content of the Shader Program, searches for the Subroutine type names, 
// the subroutines implemented for each type, print the names of the subroutines on the terminal, and add the names of 
// the subroutines to the shaders vector, which is used for the shaders swapping
void SetupShader(int program)
{
    int maxSub,maxSubU,countActiveSU;
    GLchar name[256]; 
    int len, numCompS;
    
    // global parameters about the Subroutines parameters of the system
    glGetIntegerv(GL_MAX_SUBROUTINES, &maxSub);
    glGetIntegerv(GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS, &maxSubU);
    std::cout << "Max Subroutines:" << maxSub << " - Max Subroutine Uniforms:" << maxSubU << std::endl;

    // get the number of Subroutine uniforms (only for the Fragment shader, due to the nature of the exercise)
    // it is possible to add similar calls also for the Vertex shader
    glGetProgramStageiv(program, GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORMS, &countActiveSU);
    
    // print info for every Subroutine uniform
    for (int i = 0; i < countActiveSU; i++) {
        
        // get the name of the Subroutine uniform (in this example, we have only one)
        glGetActiveSubroutineUniformName(program, GL_FRAGMENT_SHADER, i, 256, &len, name);
        // print index and name of the Subroutine uniform
        std::cout << "Subroutine Uniform: " << i << " - name: " << name << std::endl;

        // get the number of subroutines
        glGetActiveSubroutineUniformiv(program, GL_FRAGMENT_SHADER, i, GL_NUM_COMPATIBLE_SUBROUTINES, &numCompS);
        
        // get the indices of the active subroutines info and write into the array s
        int *s =  new int[numCompS];
        glGetActiveSubroutineUniformiv(program, GL_FRAGMENT_SHADER, i, GL_COMPATIBLE_SUBROUTINES, s);
        std::cout << "Compatible Subroutines:" << std::endl;
        
        // for each index, get the name of the subroutines, print info, and save the name in the shaders vector
        for (int j=0; j < numCompS; ++j) {
            glGetActiveSubroutineName(program, GL_FRAGMENT_SHADER, s[j], 256, &len, name);
            std::cout << "\t" << s[j] << " - " << name << "\n";
            shaders.push_back(name);
        }
        std::cout << std::endl;
        
        delete[] s;
    }
}

//////////////////////////////////////////
// we print on console the name of the currently used shader subroutine
void PrintCurrentShader(int subroutine)
{
    std::cout << "Current shader subroutine: " << shaders[subroutine]  << std::endl;
}

//////////////////////////////////////////
// callback for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    GLuint new_subroutine;

    // if ESC is pressed, we close the application
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // if P is pressed, we start/stop the animated rotation of models
    if(key == GLFW_KEY_P && action == GLFW_PRESS)
        spinning=!spinning;

    // if L is pressed, we activate/deactivate wireframe rendering of models
    if(key == GLFW_KEY_L && action == GLFW_PRESS)
        wireframe=!wireframe;
    if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        shootToPlayer(glm::vec3(2.,2.,2.));
    }
    // pressing a key number, we change the shader applied to the models
    // if the key is between 1 and 9, we proceed and check if the pressed key corresponds to
    // a valid subroutine
    if((key >= GLFW_KEY_1 && key <= GLFW_KEY_9) && action == GLFW_PRESS)
    {
        // "1" to "9" -> ASCII codes from 49 to 59
        // we subtract 48 (= ASCII CODE of "0") to have integers from 1 to 9
        // we subtract 1 to have indices from 0 to 8
        new_subroutine = (key-'0'-1);
        // if the new index is valid ( = there is a subroutine with that index in the shaders vector),
        // we change the value of the current_subroutine variable
        // NB: we can just check if the new index is in the range between 0 and the size of the shaders vector,
        // avoiding to use the std::find function on the vector
        if (new_subroutine<shaders.size())
        {
            current_subroutine = new_subroutine;
            PrintCurrentShader(current_subroutine);
        }
    }

    // we keep trace of the pressed keys
    // with this method, we can manage 2 keys pressed at the same time:
    // many I/O managers often consider only 1 key pressed at the time (the first pressed, until it is released)
    // using a boolean array, we can then check and manage all the keys pressed at the same time
    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;
}

//////////////////////////////////////////
// If one of the WASD keys is pressed, the camera is moved accordingly (the code is in utils/camera.h)
void apply_camera_movements()
{
    if(keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
    
    camera.ApllyMovement();
}

//////////////////////////////////////////
// callback for mouse events
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
      // we move the camera view following the mouse cursor
      // we calculate the offset of the mouse cursor from the position in the last frame
      // when rendering the first frame, we do not have a "previous state" for the mouse, so we set the previous state equal to the initial values (thus, the offset will be = 0)

    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    // we save the current cursor position in 2 global variables, in order to use the values in the keyboard callback function
    cursorX = xpos;
    cursorY = ypos;

    // offset of mouse cursor position
    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;

    // the new position will be the previous one for the next frame
    lastX = xpos;
    lastY = ypos;

    // we pass the offset to the Camera class instance in order to update the rendering
    camera.ProcessMouseMovement(xoffset, yoffset);

}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) 
    {
       double xpos, ypos;
       //getting cursor position
       glfwGetCursorPos(window, &xpos, &ypos);
       if(add_hit(xpos/screenWidth,1-(ypos/screenHeight))){
        cout << "Hit Position at (" << xpos << " : " << ypos<<")" << endl;
       }

    }
}


void update_hits(float deltaTime){
    power-=deltaTime*0.5*power;
    if(power<0.02){
            power=0;
    }
    for(int i=0; i<MAX_HIT;i++){
        if(powers[i]>power){
            powers[i]=power;
        }
    }
}

bool add_hit(float normx, float normy){
    if(powers[hit_index]!=0){
        return false;
    }
    hitPoints[2*hit_index]=normx;
    hitPoints[2*hit_index+1]=normy;
    powers[hit_index]=hitDamage;
    power=hitDamage;
    hit_index=(hit_index+1)%MAX_HIT;
    return true;
}

///////////////////////////////////////////
// load one side of the cubemap, passing the name of the file and the side of the corresponding OpenGL cubemap
void LoadTextureCubeSide(string path, string side_image, GLuint side_name)
{
    int w, h;
    unsigned char* image;
    string fullname;

    // full name and path of the side of the cubemap
    fullname = path + side_image;
    // we load the image file
    image = stbi_load(fullname.c_str(), &w, &h, 0, STBI_rgb);
    if (image == nullptr)
        std::cout << "Failed to load texture!" << std::endl;
    // we set the image file as one of the side of the cubemap (passed as a parameter)
    glTexImage2D(side_name, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    // we free the memory once we have created an OpenGL texture
    stbi_image_free(image);
}

GLint LoadTextureCube(string path,const string format= std::string("jpg"))
{
    GLuint textureImage;

    // we create and activate the OpenGL cubemap texture
    glGenTextures(1, &textureImage);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureImage);

    // we load and set the 6 images corresponding to the 6 views of the cubemap
    // we use as convention that the names of the 6 images are "posx, negx, posy, negy, posz, negz", placed at the path passed as parameter
    // we load the images individually and we assign them to the correct sides of the cube map
    LoadTextureCubeSide(path, std::string("posx.")+format, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
    LoadTextureCubeSide(path, std::string("negx.")+format, GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
    LoadTextureCubeSide(path, std::string("posy.")+format, GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
    LoadTextureCubeSide(path, std::string("negy.")+format, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
    LoadTextureCubeSide(path, std::string("posz.")+format, GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
    LoadTextureCubeSide(path, std::string("negz.")+format, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);

    // we set the filtering for minification and magnification
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // we set how to consider the texture coordinates outside [0,1] range
    // in this case we have a cube map, so
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // we set the binding to 0 once we have finished
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureImage;

}

void shoot(){
    
    ///////
    /// BULLET MANAGEMENT (SPACE KEY)
    // if space is pressed, we "shoot" a bullet in the scene
    // the initial trajectory of the bullet is given by a vector from the position of the camera to the mouse cursor position, which must be converted from Viewport Coordinates back to World Coordinate

    btVector3 impulse;
    // we need a initial rotation, even if useless for a sphere
    glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec4 shoot;
    // initial velocity of the bullet
    GLfloat shootInitialSpeed = 15.0f;
    // matrix for the inverse matrix of view and projection
    glm::mat4 unproject;
    // we create a Rigid Body with mass = 1
    GameObject *bullet=new GameObject(camera.Position(),bullet_size,rot,bulletModel);
    bullet->setColor3(bullet_color);
    bullet->addRigidbody(bulletSimulation,SPHERE,.5f,0.3f,0.3f);
    //bullet->rb->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
    bullet->rb->setGravity(btVector3(0.,0.,0.));
    bullet->rb->setUserIndex(1);
    scene.push_back(bullet); 
    // we must retro-project the coordinates of the mouse pointer, in order to have a point in world coordinate to be used to determine a vector from the camera (= direction and orientation of the bullet)
    // we convert the cursor position (taken from the mouse callback) from Viewport Coordinates to Normalized Device Coordinate (= [-1,1] in both coordinates)
    shoot.x = (cursorX/screenWidth) * 2.0f - 1.0f;
    shoot.y = -(cursorY/screenHeight) * 2.0f + 1.0f; // Viewport Y coordinates are from top-left corner to the bottom
    // we need a 3D point, so we set a minimum value to the depth with respect to camera position
    shoot.z = 1.0f;
    // w = 1.0 because we are using homogeneous coordinates
    shoot.w = 1.0f;

    // we determine the inverse matrix for the projection and view transformations
    unproject = glm::inverse(projection * view);

    // we convert the position of the cursor from NDC to world coordinates, and we multiply the vector by the initial speed
    shoot = glm::normalize(unproject * shoot) * shootInitialSpeed;

    // we apply the impulse and shoot the bullet in the scene
    // N.B.) the graphical aspect of the bullet is treated in the rendering loop
    impulse = btVector3(shoot.x, shoot.y, shoot.z);
    bullet->rb->applyCentralImpulse(impulse);
}

void shootToPlayer(glm::vec3 from){
    
    ///////
    /// BULLET MANAGEMENT (SPACE KEY)
    // if space is pressed, we "shoot" a bullet in the scene
    // the initial trajectory of the bullet is given by a vector from the position of the camera to the mouse cursor position, which must be converted from Viewport Coordinates back to World Coordinate

    btVector3 impulse;
    // we need a initial rotation, even if useless for a sphere
    glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 shoot;
    // initial velocity of the bullet
    GLfloat shootInitialSpeed = 15.0f;
    // matrix for the inverse matrix of view and projection
    glm::mat4 unproject;
    // we create a Rigid Body with mass = 1
    Bullet *bullet=new Bullet(from,bullet_size,rot,bulletModel);
    bullet->setColor3(bullet_color);
    bullet->addRigidbody(bulletSimulation,SPHERE,.5f,0.3f,0.3f);
    //bullet->rb->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
    bullet->rb->setGravity(btVector3(0.,0.,0.));
    bullet->rb->setUserIndex(1);
    scene.push_back(bullet); 

    shoot=camera.Position()-from;

    // we convert the position of the cursor from NDC to world coordinates, and we multiply the vector by the initial speed
    shoot = glm::normalize(shoot) * shootInitialSpeed;

    // we apply the impulse and shoot the bullet in the scene
    // N.B.) the graphical aspect of the bullet is treated in the rendering loop
    impulse = btVector3(shoot.x, shoot.y, shoot.z);
    bullet->rb->applyCentralImpulse(impulse);
}

void print2(glm::vec2 v){
    cout<<"("<<v.x<<":"<<v.y<<")"<<endl;
}

void print3(glm::vec3 v){
    cout<<"("<<v.x<<":"<<v.y<<":"<<v.z<<")"<<endl;
}
void print4(glm::vec4 v){
    cout<<"("<<v.x<<":"<<v.y<<":"<<v.z<<":"<<v.w<<")"<<endl;
}

float clamp(float val, float min, float max){
    return std::max(min,std::min(max,val));
}

void processhit(glm::vec3 from_pos){
    glm::vec4 dir= projection*view*glm::vec4(from_pos,1.);
    glm::vec3 ndc = glm::vec3(dir) / dir.w;
    glm::vec2 viewportCoord = glm::vec2(ndc) * 0.5f + 0.5f; //ndc is -1 to 1 in GL. scale for 0 to 1
    viewportCoord.x=clamp(viewportCoord.x,0.,1.);
    viewportCoord.y=clamp(viewportCoord.y,0.,1.);
    add_hit(viewportCoord.x,viewportCoord.y);
    
    /*only border effect
    glm::vec2 dir2d= (glm::normalize(glm::vec2(dir.x,dir.z))*0.5f)+0.5f;
    add_hit(dir2d.x,dir2d.y);
    */
}

void checkForCollision(){
    //Go through collisions
    int numManifolds = bulletSimulation.dispatcher->getNumManifolds();
    set<btCollisionObject*> toRem;
    for (int i = 0; i < numManifolds; i++)
    {
        btPersistentManifold* contactManifold = bulletSimulation.dispatcher->getManifoldByIndexInternal(i);
        btCollisionObject* obA = const_cast<btCollisionObject*>(contactManifold->getBody0());
        btCollisionObject* obB = const_cast<btCollisionObject*>(contactManifold->getBody1());

        GameObject* gameObjA = static_cast<GameObject*>(obA->getUserPointer());
        GameObject* gameObjB = static_cast<GameObject*>(obB->getUserPointer());
        if(obA->getUserIndex()==1){
            toRem.insert(obA);
            if(obB->getUserIndex()==2){
                Bullet* b= static_cast<Bullet*>(gameObjA);
                processhit(b->shoot_pos);
            }
            gameObjA->rb=nullptr;
        }
        if(obB->getUserIndex()==1){
            toRem.insert(obB);
            if(obA->getUserIndex()==2){
                Bullet* b= static_cast<Bullet*>(gameObjB);
                processhit(b->shoot_pos);

            }
            gameObjB->rb=nullptr;
            //bulletSimulation.deleteCollisionObject(obB);
            //delete gameObjB;
        }
    
    }

    for(auto obj:toRem){
 
        bulletSimulation.deleteCollisionObject(obj);
        scene.erase(remove(scene.begin(),scene.end(),(GameObject*)obj->getUserPointer()),scene.end());

    }
}
