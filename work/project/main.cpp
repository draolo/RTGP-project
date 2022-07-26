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

// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// dimensions of application's window
GLuint screenWidth = 800, screenHeight = 600;

GLfloat frequency = 12.0;
GLfloat power = 4.0;
// number of harmonics (used in the turbulence-based subroutines)
GLfloat harmonics = 4.0;

enum render_passes{RENDER, NOISE};

// callback function for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

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

// rotation angle on Y axis
GLfloat orientationY = 0.0f;
// rotation speed on Y axis
GLfloat spin_speed = 30.0f;
// boolean to start/stop animated rotation on Y angle
GLboolean spinning = GL_TRUE;

// boolean to activate/deactivate wireframe rendering
GLboolean wireframe = GL_FALSE;

// velocity for noise animation effect (for the animated subroutine)
GLfloat speed = 5.0;

// color to be passed as uniform to the shader of the plane
GLfloat planeColor[] = {0.0,0.5,0.0};
GLfloat objectColor[] = {0.5,0.0,0.0};


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

    Shader basic_shader("00_basic.vert", "01_fullcolor.frag");

    // we create the Shader Program used for objects (which presents different subroutines we can switch)
    Shader texture_shader = Shader("framebuffer.vert", "framebuffer.frag");
    // we parse the Shader Program to search for the number and names of the subroutines. 
    // the names are placed in the shaders vector
    SetupShader(texture_shader.Program);
    // we print on console the name of the first subroutine used


    glUniform1i(glGetUniformLocation(texture_shader.Program, "screenTexture"), 0);

    glEnable(GL_DEPTH_TEST);

	// Enables Cull Facing
	glEnable(GL_CULL_FACE);
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

    unsigned int FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// Create Framebuffer Texture
	unsigned int framebufferTexture;
	glGenTextures(1, &framebufferTexture);
	glBindTexture(GL_TEXTURE_2D, framebufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);

	// Create Render Buffer Object
	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    // Bind the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);


    
    // Error checking framebuffer
	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer error: " << fboStatus << std::endl;



    // we load the model(s) (code of Model class is in include/utils/model_v1.h)
    Model cubeModel("../../models/cube.obj");
    Model sphereModel("../../models/sphere.obj");
    Model bunnyModel("../../models/bunny_lp.obj");
    Model planeModel("../../models/plane.obj");

    // Projection matrix: FOV angle, aspect ratio, near and far planes
    glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);
    // View matrix (=camera): position, view direction, camera "up" vector
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 7.0f), glm::vec3(0.0f, 0.0f, -7.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // Model and Normal transformation matrices for the objects in the scene: we set to identity
    glm::mat4 sphereModelMatrix = glm::mat4(1.0f);
    glm::mat3 sphereNormalMatrix = glm::mat3(1.0f);
    glm::mat4 cubeModelMatrix = glm::mat4(1.0f);
    glm::mat3 cubeNormalMatrix = glm::mat3(1.0f);
    glm::mat4 bunnyModelMatrix = glm::mat4(1.0f);
    glm::mat3 bunnyNormalMatrix = glm::mat3(1.0f);
    glm::mat4 planeModelMatrix = glm::mat4(1.0f);

    // Rendering loop: this code is executed at each frame
    while(!glfwWindowShouldClose(window))
    {
       // Bind the custom framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

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

        power-=deltaTime*0.5*power;
        if(power<0.02){
            power=4.0;
        }
        /////////////////// PLANE ////////////////////////////////////////////////
        // We render a plane under the objects. We apply the fullcolor shader to the plane, and we do not apply the rotation applied to the other objects.
        basic_shader.Use();

        // we pass projection and view matrices to the Shader Program of the plane
        glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));

        // we determine the position in the Shader Program of the uniform variables
        GLint planeColorLocation = glGetUniformLocation(basic_shader.Program, "colorIn");
        // we assign the value to the uniform variables
        glUniform3fv(planeColorLocation, 1, planeColor);

        // we create the transformation matrix
        // we reset to identity at each frame
        planeModelMatrix = glm::mat4(1.0f);
        planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(0.0f, -1.0f, 0.0f));
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(10.0f, 1.0f, 10.0f));
        glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));

        // we render the plane
        planeModel.Draw();


        /////////////////// OBJECTS ////////////////////////////////////////////////


        // SPHERE
        /*
          we create the transformation matrix

          N.B.) the last defined is the first applied

          We need also the matrix for normals transformation, which is the inverse of the transpose of the 3x3 submatrix (upper left) of the modelview. We do not consider the 4th column because we do not need translations for normals.
          An explanation (where XT means the transpose of X, etc):
            "Two column vectors X and Y are perpendicular if and only if XT.Y=0. If We're going to transform X by a matrix M, we need to transform Y by some matrix N so that (M.X)T.(N.Y)=0. Using the identity (A.B)T=BT.AT, this becomes (XT.MT).(N.Y)=0 => XT.(MT.N).Y=0. If MT.N is the identity matrix then this reduces to XT.Y=0. And MT.N is the identity matrix if and only if N=(MT)-1, i.e. N is the inverse of the transpose of M.

        */

        // we determine the position in the Shader Program of the uniform variables
        GLint ColorLocation = glGetUniformLocation(basic_shader.Program, "colorIn");
        // we assign the value to the uniform variables
        glUniform3fv(ColorLocation, 1, objectColor);

        // we reset to identity at each frame
        sphereModelMatrix = glm::mat4(1.0f);
        sphereNormalMatrix = glm::mat3(1.0f);
        sphereModelMatrix = glm::translate(sphereModelMatrix, glm::vec3(-3.0f, 0.0f, 0.0f));
        sphereModelMatrix = glm::rotate(sphereModelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
        sphereModelMatrix = glm::scale(sphereModelMatrix, glm::vec3(0.8f, 0.8f, 0.8f));
        // if we cast a mat4 to a mat3, we are automatically considering the upper left 3x3 submatrix
        sphereNormalMatrix = glm::inverseTranspose(glm::mat3(view*sphereModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(sphereModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(basic_shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(sphereNormalMatrix));

        // we render the model
        sphereModel.Draw();

        //CUBE
        // we create the transformation matrix and the normals transformation matrix
        // we reset to identity at each frame
        cubeModelMatrix = glm::mat4(1.0f);
        cubeNormalMatrix = glm::mat3(1.0f);
        cubeModelMatrix = glm::translate(cubeModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        cubeModelMatrix = glm::rotate(cubeModelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
        cubeModelMatrix = glm::scale(cubeModelMatrix, glm::vec3(0.8f, 0.8f, 0.8f));	// It's a bit too big for our scene, so scale it down
        cubeNormalMatrix = glm::inverseTranspose(glm::mat3(view*cubeModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(cubeModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(basic_shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(cubeNormalMatrix));

        // we render the cube
        cubeModel.Draw();

        //BUNNY
        // we create the transformation matrix and the normals transformation matrix
        // we reset to identity at each frame
        bunnyModelMatrix = glm::mat4(1.0f);
        bunnyNormalMatrix = glm::mat3(1.0f);
        bunnyModelMatrix = glm::translate(bunnyModelMatrix, glm::vec3(3.0f, 0.0f, 0.0f));
        bunnyModelMatrix = glm::rotate(bunnyModelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
        bunnyModelMatrix = glm::scale(bunnyModelMatrix, glm::vec3(0.3f, 0.3f, 0.3f));	// It's a bit too big for our scene, so scale it down
        bunnyNormalMatrix = glm::inverseTranspose(glm::mat3(view*bunnyModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(basic_shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(bunnyModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(basic_shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(bunnyNormalMatrix));

        // Swapping back and front buffers
        bunnyModel.Draw();

        // Faccio lo swap tra back e front buffer
        // Bind the default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		texture_shader.Use();
        GLint frequencyLocation = glGetUniformLocation(texture_shader.Program, "frequency");
        GLint powerLocation = glGetUniformLocation(texture_shader.Program, "power");
        GLint harmonicsLocation = glGetUniformLocation(texture_shader.Program, "harmonics");

        // we assign the value to the uniform variable
        glUniform1f(frequencyLocation, frequency);
        glUniform1f(powerLocation, power);
        glUniform1f(harmonicsLocation, harmonics);


		// Draw the framebuffer rectangle
		glBindVertexArray(rectVAO);
		glDisable(GL_DEPTH_TEST); // prevents framebuffer rectangle from being discarded
		glBindTexture(GL_TEXTURE_2D, framebufferTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);


		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);

    }

    // when I exit from the graphics loop, it is because the application is closing
    // we delete the Shader Programs
    basic_shader.Delete();
    texture_shader.Delete();
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
}
