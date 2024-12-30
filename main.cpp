#include "headers/header.h"
#include "headers/Model.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
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
    // Model fighter2("resources/fighter_1/obj.obj");
    Model hangar("resources/hangar/obj.obj");

    fighter1.position = make_tuple(4.5f, -1.5f, 0.0f);
    // fighter2.position = make_tuple(6.389964f, 2.432045f, 2.700783f);

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
        processInput(window);

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
        
        /*
        // render the fighter2 model
        glm::mat4 fighter2Model = glm::mat4(1.0f);
        if (play4)
        {
            if (!fighter2.hasReachedTarget(make_tuple(28.352026f, 4.128473f, -25.634726f)))
                fighter2.modelMove(make_tuple(28.352026f, 4.128473f, -25.634726f));
            else 
            {
                play5 = true;
                play4 = false;
            }
        }

        if (play5)
        {
            if(!fighter2.hasReachedTarget(make_tuple(28.506851f, 6.462111f, -66.843697f))) 
                fighter2.modelMove(make_tuple(28.506851f, 6.462111f, -66.843697f));
            else 
            {
                play6 = true;
                play5 = false;
            }
        }

        if (play6)
        {
            fighter2.modelMove(make_tuple(5.200547, 2.633695, -62.159782));
        }

        fighter2Model = glm::translate(fighter2Model, glm::vec3(get<0>(fighter2.position), get<1>(fighter2.position), get<2>(fighter2.position)));
        fighter2Model = glm::scale(fighter2Model, glm::vec3(0.3f, 0.3f, 0.3f));


        ourShader.setMat4("model", fighter2Model);
        fighter2.Draw(ourShader);
        */

        // render the hangar model
        glm::mat4 hangarModel = glm::mat4(1.0f);
        hangarModel = glm::translate(hangarModel, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
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
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Toggle camera lock when pressing the "L" key
    static bool lKeyPressed = false; // Tracks whether the "L" key was pressed
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        if (!lKeyPressed)
        {
            cameraLocked = !cameraLocked; // Toggle camera lock
            lKeyPressed = true;
        }
    }
    else
    {
        lKeyPressed = false; // Reset the key press state when "L" is released
    }

    // Process camera movement only if the camera is not locked
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