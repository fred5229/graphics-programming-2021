#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <vector>
#include <chrono>

#include "shader.h"
#include "glmutils.h"

#include "primitives.h"

#include <cmath>

// structure to hold render info
// -----------------------------
struct SceneObject{
    unsigned int VAO;
    unsigned int vertexCount;
    void drawSceneObject() const{
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES,  vertexCount, GL_UNSIGNED_INT, 0);
    }
};

struct ParticlesObject{
    unsigned int VAO;
    unsigned int vertexBufferSize;
    void drawSceneObject() const {
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, vertexBufferSize);
        //glDrawElements(GL_POINTS, vertexBufferSize, GL_UNSIGNED_INT, 0);
    }
};

// function declarations
// ---------------------
unsigned int createArrayBuffer(const std::vector<float> &array);
unsigned int createElementArrayBuffer(const std::vector<unsigned int> &array);
unsigned int createVertexArray(const std::vector<float> &positions, const std::vector<float> &colors, const std::vector<unsigned int> &indices);
unsigned int createParticleVertexArray();
void setup();
void drawObjects();

// glfw and input functions
// ------------------------
void cursorInRange(float screenX, float screenY, int screenW, int screenH, float min, float max, float &x, float &y);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void cursor_input_callback(GLFWwindow* window, double posX, double posY);
void drawCube(glm::mat4 model);
void drawRain(glm::mat4 model);

// screen settings
// ---------------
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

// global variables used for rendering
// -----------------------------------
SceneObject cube;
SceneObject floorObj;
Shader* shaderProgram;
Shader* particleShaderProgram;
ParticlesObject particles;

// global variables used for control
// ---------------------------------
float currentTime;
glm::vec3 camForward(.0f, .0f, -1.0f);
glm::vec3 camPosition(.0f, 1.6f, 0.0f);
float linearSpeed = 0.15f, rotationGain = 30.0f;

// Particle stuff
const unsigned int numberOfParticles = 10000;
const int boxSize = 30;
float gravityOffset = 0;
// Rain
const float deltaGravity = 0.1;
// Snow
//const float deltaGravity = 0.05;
float windOffset = 0;
const float windDelta = 0.03;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Exercise 5.2", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_input_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // enable built in variable gl_PointSize in the vertex shader
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    // setup mesh objects
    // ---------------------------------------
    setup();

    // set up the z-buffer
    // Notice that the depth range is now set to glDepthRange(-1,1), that is, a left handed coordinate system.
    // That is because the default openGL's NDC is in a left handed coordinate system (even though the default
    // glm and legacy openGL camera implementations expect the world to be in a right handed coordinate system);
    // so let's conform to that
    glDepthRange(-1,1); // make the NDC a LEFT handed coordinate system, with the camera pointing towards +z
    glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC


    // render loop
    // -----------
    // render every loopInterval seconds
    float loopInterval = 0.02f;
    auto begin = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(window))
    {
        // update current time
        auto frameStart = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> appTime = frameStart - begin;
        currentTime = appTime.count();

        processInput(window);

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

        // notice that we also need to clear the depth buffer (aka z-buffer) every new frame
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawObjects();

        glfwSwapBuffers(window);
        glfwPollEvents();

        // control render loop frequency
        std::chrono::duration<float> elapsed = std::chrono::high_resolution_clock::now()-frameStart;
        while (loopInterval > elapsed.count()) {
            elapsed = std::chrono::high_resolution_clock::now() - frameStart;
        }
    }

    delete shaderProgram;
    delete particleShaderProgram;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void drawObjects(){

    glm::mat4 scale = glm::scale(1.f, 1.f, 1.f);

    // NEW!
    // update the camera pose and projection, and compose the two into the viewProjection with a matrix multiplication
    // projection * view = world_to_view -> view_to_perspective_projection
    // or if we want ot match the multiplication order (projection * view), we could read
    // perspective_projection_from_view <- view_from_world
    glm::mat4 projection = glm::perspectiveFov(70.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, .01f, 100.0f);
    glm::mat4 view = glm::lookAt(camPosition, camPosition + camForward, glm::vec3(0,1,0));
    glm::mat4 viewProjection = projection * view;

    shaderProgram->use();
    // draw floor (the floor was built so that it does not need to be transformed)
    shaderProgram->setMat4("model", viewProjection);
    floorObj.drawSceneObject();

    // draw 1 cube
    drawCube(viewProjection * glm::translate(2.0f, 1.f, 2.0f) * glm::rotateY(glm::half_pi<float>()) * scale);

    particleShaderProgram->use();
    drawRain(viewProjection);
}

void drawRain(glm::mat4 model) {

    gravityOffset += deltaGravity;
    windOffset += windDelta;
    glm::vec3 offsets = glm::vec3(windOffset, -gravityOffset, 0);
    offsets -= camPosition + camForward + glm::vec3(boxSize/2);
    offsets = glm::mod(offsets, glm::vec3(boxSize));

    particleShaderProgram->setVec3("offsets", offsets);
    particleShaderProgram->setFloat("boxSize", boxSize);
    particleShaderProgram->setVec3("cameraPosition", camPosition);
    particleShaderProgram->setVec3("forwardOffset", camForward);
    particleShaderProgram->setMat4("model", model);
    
    particles.drawSceneObject();
}

void drawCube(glm::mat4 model){
    // draw object
    shaderProgram->setMat4("model", model);
    cube.drawSceneObject();
}

// Setting up the particles and distributing them randomly in the initial box
unsigned int createParticleVertexArray() {
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    std::vector<float> particleVertexData(numberOfParticles * 3);
    for (size_t i = 0; i < particleVertexData.size(); i += 3)
    {
        float x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / boxSize));
        float y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / boxSize));
        float z = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / boxSize));
        particleVertexData[i] = x;
        particleVertexData[i + 1] = y;
        particleVertexData[i + 2] = z;
    }

    particles.vertexBufferSize = numberOfParticles;

    glBufferData(GL_ARRAY_BUFFER, numberOfParticles * 3 * 4, &particleVertexData[0], GL_DYNAMIC_DRAW);

    int posAttributeLocation = glGetAttribLocation(particleShaderProgram->ID, "pos");
    glEnableVertexAttribArray(posAttributeLocation);
    glVertexAttribPointer(posAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

    return VAO;
}

void setup(){
    // initialize shaders
    shaderProgram = new Shader("shaders/shader.vert", "shaders/shader.frag");
    particleShaderProgram = new Shader("shaders/rainShader.vert", "shaders/rainShader.frag");

    // load floor mesh into openGL
    floorObj.VAO = createVertexArray(floorVertices, floorColors, floorIndices);
    floorObj.vertexCount = floorIndices.size();

    // load cube mesh into openGL
    cube.VAO = createVertexArray(cubeVertices, cubeColors, cubeIndices);
    cube.vertexCount = cubeIndices.size();

    particles.VAO = createParticleVertexArray();
}

unsigned int createVertexArray(const std::vector<float> &positions, const std::vector<float> &colors, const std::vector<unsigned int> &indices){
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    // bind vertex array object
    glBindVertexArray(VAO);

    // set vertex shader attribute "pos"
    createArrayBuffer(positions); // creates and bind  the VBO
    int posAttributeLocation = glGetAttribLocation(shaderProgram->ID, "pos");
    glEnableVertexAttribArray(posAttributeLocation);
    glVertexAttribPointer(posAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // set vertex shader attribute "color"
    createArrayBuffer(colors); // creates and bind the VBO
    int colorAttributeLocation = glGetAttribLocation(shaderProgram->ID, "color");
    glEnableVertexAttribArray(colorAttributeLocation);
    glVertexAttribPointer(colorAttributeLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // creates and bind the EBO
    createElementArrayBuffer(indices);

    return VAO;
}

unsigned int createArrayBuffer(const std::vector<float> &array){
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, array.size() * sizeof(GLfloat), &array[0], GL_STATIC_DRAW);

    return VBO;
}


unsigned int createElementArrayBuffer(const std::vector<unsigned int> &array){
    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, array.size() * sizeof(unsigned int), &array[0], GL_STATIC_DRAW);

    return EBO;
}

// NEW!
// instead of using the NDC to transform from screen space you can now define the range using the
// min and max parameters
void cursorInRange(float screenX, float screenY, int screenW, int screenH, float min, float max, float &x, float &y){
    float sum = max - min;
    float xInRange = (float) screenX / (float) screenW * sum - sum/2.0f;
    float yInRange = (float) screenY / (float) screenH * sum - sum/2.0f;
    x = xInRange;
    y = -yInRange; // flip screen space y axis
}

void cursor_input_callback(GLFWwindow* window, double posX, double posY){
    // TODO - rotate the camera position based on mouse movements
    //  if you decide to use the lookAt function, make sure that the up vector and the
    //  vector from the camera position to the lookAt target are not collinear

    // get cursor position and scale it down to a smaller range
    int screenW, screenH;
    glfwGetWindowSize(window, &screenW, &screenH);
    glm::vec2 cursorPosition(0.0f);
    cursorInRange(posX, posY, screenW, screenH, -1.0f, 1.0f, cursorPosition.x, cursorPosition.y);

    // initialize with first value so that there is no jump at startup
    static glm::vec2 lastCursorPosition = cursorPosition;

    // compute the cursor position change
    glm::vec2 positionDiff = cursorPosition - lastCursorPosition;

    static float rotationAroundVertical = 0;
    static float rotationAroundLateral = 0;

    // require a minimum threshold to rotate
    if (glm::dot(positionDiff, positionDiff) > 1e-5f){
        camForward = glm::vec3 (0,0,-1);
        // rotate the forward vector around the Y axis, notices that w is set to 0 since it is a vector
        rotationAroundVertical += glm::radians(-positionDiff.x * rotationGain);
        camForward = glm::rotateY(rotationAroundVertical) * glm::vec4(camForward, 0.0f);

        // rotate the forward vector around the lateral axis
        rotationAroundLateral +=  glm::radians(positionDiff.y * rotationGain);
        // we need to clamp the range of the rotation, otherwise forward and Y axes get parallel
        rotationAroundLateral = glm::clamp(rotationAroundLateral, -glm::half_pi<float>() * 0.9f, glm::half_pi<float>() * 0.9f);

        glm::vec3 lateralAxis = glm::cross(camForward, glm::vec3(0, 1,0));

        camForward = glm::rotate(rotationAroundLateral, lateralAxis) * glm::vec4(camForward, 0);

        lastCursorPosition = cursorPosition;
    }

}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // TODO move the camera position based on keys pressed (use either WASD or the arrow keys)
    glm::vec3 forwardInXZ = glm::normalize(glm::vec3(camForward.x, 0, camForward.z));
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        camPosition += forwardInXZ * linearSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        camPosition -= forwardInXZ * linearSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        // vector perpendicular to camera forward and Y-axis
        camPosition -= glm::cross(forwardInXZ, glm::vec3(0, 1, 0)) * linearSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        // vector perpendicular to camera forward and Y-axis
        camPosition += glm::cross(forwardInXZ, glm::vec3(0, 1, 0)) * linearSpeed;
    }

}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}