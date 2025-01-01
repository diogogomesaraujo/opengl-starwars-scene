#include "headers/header.h"
#include "headers/Model.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window, Model &fighter1);
unsigned int loadCubemap(vector<std::string> faces);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool cameraLocked = false;

// lighting
glm::vec3 lightPos(0.0f, 0.0f, 15.0f);

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// pressed
bool play1 = false, play2 = false, play3 = false, play4 = false, play5 = false, play6 = false;

// predefined Camera Positions
glm::vec3 cameraPos1 = glm::vec3(-2.47806f, 1.00429f, 0.031182f); 
glm::vec3 cameraPos2 = glm::vec3(15.0f, 5.0f, 20.0f); 
glm::vec3 cameraPos3 = glm::vec3(-15.0f, 5.0f, 20.0f); 

// predefined Camera Orientations
glm::vec3 cameraFront1 = glm::vec3(0.994859f, -0.101172f, -0.00446301f); 
glm::vec3 cameraFront2 = glm::vec3(1.0f, 0.0f, 0.0f); 
glm::vec3 cameraFront3 = glm::vec3(-1.0f, 0.0f, 0.0f); 

void switchCameraPosition(glm::vec3 newPos, glm::vec3 newFront) {
    // update the camera position and front vector
    camera.Position = newPos;
    camera.Front = glm::normalize(newFront); 

    // recalculate yaw and pitch from the new front vector
    camera.Yaw = glm::atan(camera.Front.z, camera.Front.x);  
    camera.Pitch = glm::asin(camera.Front.y);                

    // ensure the up vector is perpendicular to the front direction
    camera.Up = glm::normalize(glm::cross(glm::cross(camera.Front, glm::vec3(0.0f, 1.0f, 0.0f)), camera.Front));

    cameraLocked = true;
}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Star Wars Scene", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader("shaders/lighting.vs", "shaders/lighting.fs");
    Shader skyboxShader("shaders/skybox.vs", "shaders/skybox.fs");

    // load models
    // -----------
    Model fighter1("resources/fighter_1/obj.obj");
    Model hangar("resources/hangar/obj.obj");

    fighter1.position = make_tuple(4.5f, -1.5f, 0.0f);

    stbi_set_flip_vertically_on_load(false);

    vector<std::string> faces{
        "resources/skybox 2/right.png",
        "resources/skybox 2/left.png",
        "resources/skybox 2/top.png",
        "resources/skybox 2/bottom.png",
        "resources/skybox 2/front.png",
        "resources/skybox 2/back.png"};
    unsigned int cubemapTexture = loadCubemap(faces);

    float skyboxVertices[] = {
        // positions
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f};

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    // draw in wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Set material properties (if applicable)
        ourShader.setFloat("material.shininess", 32.0f); // Adjust shininess for the material

        // Pass camera position to the shader
        ourShader.setVec3("viewPos", camera.Position);

        skyboxShader.use();
        skyboxShader.setInt("skybox", 0);

        // input
        // -----
        processInput(window, fighter1);

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
            play1 = true;

        ourShader.use();

        // lighting settings
        lightPos = camera.Position;
        ourShader.setVec3("globalAmbientLight", glm::vec3(0.8f, 0.7f, 0.2f)); // Moody yellow light
        ourShader.setVec3("lightPos", lightPos);                              // Pass light position in world space
        ourShader.setVec3("viewPos", camera.Position);                        // Pass camera position in world space

        ourShader.setVec3("light.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
        ourShader.setVec3("light.diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
        ourShader.setVec3("light.specular", glm::vec3(0.1f, 0.1f, 0.1f));

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        ourShader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // render the fighter1 model
        glm::mat4 fighter1Model = glm::mat4(1.0f);
        if (play1)
        {
            if (!fighter1.hasReachedTarget(make_tuple(28.352026f, 4.128473f, -25.634726f)))
                fighter1.modelMove(make_tuple(28.352026f, 4.128473f, -25.634726f));
            else 
            {
                play2 = true;
                play1 = false;
            }
        }

        if (play2)
        {
            if(!fighter1.hasReachedTarget(make_tuple(28.506851f, 6.462111f, -66.843697f))) 
                fighter1.modelMove(make_tuple(28.506851f, 6.462111f, -66.843697f));
            else 
            {
                play3 = true;
                play2 = false;
            }
        }

        if (play3)
        {
            if(!fighter1.hasReachedTarget(make_tuple(-5.970362, 1.991202, -61.859150))) 
                fighter1.modelMove(make_tuple(-5.970362, 1.991202, -61.859150));
            else 
            {
                play4 = true;
                play3 = false;
            }
        }

        fighter1Model = glm::translate(fighter1Model, glm::vec3(get<0>(fighter1.position), get<1>(fighter1.position), get<2>(fighter1.position)));
        fighter1Model = glm::scale(fighter1Model, glm::vec3(0.3f, 0.3f, 0.3f));
        ourShader.setMat4("model", fighter1Model);
        fighter1.Draw(ourShader);

        // render the hangar model
        glm::mat4 hangarModel = glm::mat4(1.0f);
        hangarModel = glm::translate(hangarModel, glm::vec3(-30.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        hangarModel = glm::scale(hangarModel, glm::vec3(0.1f, 0.1f, 0.1f));     // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", hangarModel);
        hangar.Draw(ourShader);

        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS); // Set depth function back to default

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, Model &fighter1)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // toggle camera lock when pressing the "L" key
    static bool lKeyPressed = false; 
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        if (!lKeyPressed)
        {
            cameraLocked = !cameraLocked; 
            lKeyPressed = true;
        }
    }
    else
    {
        lKeyPressed = false; 
    }

    // switch camera positions based on key input
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        switchCameraPosition(cameraPos1, cameraFront1); 
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        switchCameraPosition(cameraPos2, cameraFront2); 
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        switchCameraPosition(cameraPos3, cameraFront3); 
    }

    // adjust these variables for faster movement
    static float fighterVelocity = 0.0f; 
    static float fighterAcceleration = 10.0f; 
    static float fighterMaxSpeed = 5.0f; 
    static float fighterDamping = 5.0f;
    static float fighterMinX = -5.0f;
    static float fighterMaxX = 5.0f;  

    if (camera.Position == cameraPos1)
    {
        // accelerate left
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        {
            fighterVelocity -= fighterAcceleration * deltaTime;
        }
        // accelerate right
        else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        {
            fighterVelocity += fighterAcceleration * deltaTime;
        }
        else
        {
            // apply damping when no key is pressed - clamp to zero
            if (fighterVelocity > 0.0f)
            {
                fighterVelocity -= fighterDamping * deltaTime;
                if (fighterVelocity < 0.0f) fighterVelocity = 0.0f; 
            }
            else if (fighterVelocity < 0.0f)
            {
                fighterVelocity += fighterDamping * deltaTime;
                if (fighterVelocity > 0.0f) fighterVelocity = 0.0f; 
            }
        }

        // clamp velocity to maximum speed
        if (fighterVelocity > fighterMaxSpeed) fighterVelocity = fighterMaxSpeed;
        if (fighterVelocity < -fighterMaxSpeed) fighterVelocity = -fighterMaxSpeed;

        // update fighter position based on velocity
        std::get<2>(fighter1.position) += fighterVelocity * deltaTime;

        // clamp position to boundaries - stop movement if hitting boundary
        if (std::get<2>(fighter1.position) < fighterMinX)
        {
            std::get<2>(fighter1.position) = fighterMinX;
            fighterVelocity = 0.0f; 
        }
        if (std::get<2>(fighter1.position) > fighterMaxX)
        {
            std::get<2>(fighter1.position) = fighterMaxX;
            fighterVelocity = 0.0f; 
        }
    }

    // process camera movement only if the camera is not locked
    if (!cameraLocked)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.ProcessKeyboard(UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera.ProcessKeyboard(DOWN, deltaTime);
    }
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    // If the camera is locked, do not process mouse movement
    if (cameraLocked)
        return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    // If the camera is locked, do not process mouse scroll
    if (cameraLocked)
        return;

    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
