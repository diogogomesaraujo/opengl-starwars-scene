#include "headers/header.h"
#include "headers/Model.h"
#include "headers/Enemy.h"

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

// tilt angle for fighter
float fighterTiltAngle = 0.0f;

// predefined positions
glm::vec3 cameraPos1 = glm::vec3(-2.47806f, 1.00429f, 0.031182f);
glm::vec3 cameraFront1 = glm::vec3(0.994859f, -0.101172f, -0.00446301f);

glm::vec3 cameraPos2 = glm::vec3(-6.3649f, 10.3532f, -0.0311571f);
glm::vec3 cameraFront2 = glm::vec3(0.915654f, -0.401942f, 0.00448226f);

glm::vec3 cameraPos3 = glm::vec3(-5.13154f, 2.62794f, 9.67647f);
glm::vec3 cameraFront3 = glm::vec3(0.927023f, -0.116671f, -0.356394f);

// enemy
int enemyDirection = 1;             // 1 for right, -1 for left
float enemyMoveSpeed = 500.0f;        // Units per second
float enemyMoveDownDistance = 100.0f; // Units to move down when changing direction
float enemyBoundaryLeft = -2000.0f;   // 25 units left of current start
float enemyBoundaryRight = 2000.0f;   // 25 units right of current start
bool shouldMoveDown = false;        // Flag to indicate if enemies should move down

// Function to calculate group boundaries
std::pair<float, float> calculateInitialGroupBoundaries(const std::vector<Enemy>& enemies)
{
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();

    for (const auto& enemy : enemies)
    {
        float x = std::get<0>(enemy.position);
        if (x < minX)
            minX = x;
        if (x > maxX)
            maxX = x;
    }

    return {minX, maxX};
}

void switchCameraPosition(glm::vec3 newPos, glm::vec3 newFront, bool followFighter, const Model &fighter)
{
    if (followFighter)
    {
        // dynamically follow the fighter
        camera.Position = glm::vec3(std::get<0>(fighter.position), std::get<1>(fighter.position), std::get<2>(fighter.position)) + newPos;
        camera.Front = glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f));
    }
    else
    {
        camera.Position = newPos;
        camera.Front = glm::normalize(newFront);
    }

    // recalculate yaw and pitch
    camera.Yaw = glm::atan(camera.Front.z, camera.Front.x);
    camera.Pitch = glm::asin(camera.Front.y);

    camera.Up = glm::normalize(glm::cross(glm::cross(camera.Front, glm::vec3(0.0f, 1.0f, 0.0f)), camera.Front));
    cameraLocked = true;
}

std::vector<Enemy> createEnemyGrid(const std::string &modelPath, std::tuple<float, float, float> centerPosition, int rows, int cols, float rowSpacing, float colSpacing)
{
    std::vector<Enemy> enemies;

    // Calculate the offset to center the grid around the centerPosition
    float xOffset = -((cols - 1) * colSpacing) / 2.0f;
    float zOffset = -((rows - 1) * rowSpacing) / 2.0f;

    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < cols; ++col)
        {
            std::tuple<float, float, float> enemyPosition = std::make_tuple(
                std::get<0>(centerPosition) + xOffset + col * colSpacing,
                std::get<1>(centerPosition),
                std::get<2>(centerPosition) + zOffset + row * rowSpacing);
            enemies.emplace_back(modelPath, enemyPosition);
        }
    }
    return enemies;
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

    // Parameters for the enemy grid
    std::string enemyModelPath = "resources/fighter_1/obj.obj";
    std::tuple<float, float, float> startPosition = std::make_tuple(35.0f, 0.0f, 0.0f);
    int rows = 2;
    int cols = 3;
    float rowSpacing = 7.0f;
    float colSpacing = 12.0f;

    // Generate the enemies without movement parameters
    std::vector<Enemy> enemies = createEnemyGrid(enemyModelPath, startPosition, rows, cols, rowSpacing, colSpacing);

    fighter1.position = make_tuple(4.5f, 0.0f, 0.0f);

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

    std::pair<float, float> boundaries = calculateInitialGroupBoundaries(enemies);
    float groupMinX = boundaries.first;
    float groupMaxX = boundaries.second;

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

        // Update enemy positions using group-based movement
        // Step 1: Check if the group is about to exceed boundaries
        bool boundaryReached = false;

        if ((enemyDirection == 1 && groupMaxX + enemyMoveSpeed * deltaTime > enemyBoundaryRight) ||
            (enemyDirection == -1 && groupMinX - enemyMoveSpeed * deltaTime < enemyBoundaryLeft))
        {
            boundaryReached = true;
        }

        if (boundaryReached)
        {
            // Change direction and move the group down
            enemyDirection *= -1;
            for (auto &enemy : enemies)
            {
                enemy.moveDown(enemyMoveDownDistance);
            }

            // Update group boundaries after moving down
            // (Assuming enemies only move horizontally and down)
        }
        else
        {
            // Move the group horizontally
            float deltaX = enemyDirection * enemyMoveSpeed * deltaTime;
            for (auto &enemy : enemies)
            {
                enemy.moveHorizontally(deltaX);
            }

            // Update group boundaries based on movement
            groupMinX += enemyDirection * enemyMoveSpeed * deltaTime;
            groupMaxX += enemyDirection * enemyMoveSpeed * deltaTime;
        }

        // Render enemies
        for (auto &enemy : enemies)
        {
            glm::mat4 enemyModel = glm::mat4(1.0f);
            enemyModel = glm::translate(enemyModel, glm::vec3(
                                                        std::get<0>(enemy.position),
                                                        std::get<1>(enemy.position),
                                                        std::get<2>(enemy.position)));
            enemyModel = glm::rotate(enemyModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            enemyModel = glm::scale(enemyModel, glm::vec3(0.5f, 0.5f, 0.5f));
            ourShader.setMat4("model", enemyModel);
            enemy.Draw(ourShader);
        }

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
            if (!fighter1.hasReachedTarget(make_tuple(28.506851f, 6.462111f, -66.843697f)))
                fighter1.modelMove(make_tuple(28.506851f, 6.462111f, -66.843697f));
            else
            {
                play3 = true;
                play2 = false;
            }
        }

        if (play3)
        {
            if (!fighter1.hasReachedTarget(make_tuple(-5.970362, 1.991202, -61.859150)))
                fighter1.modelMove(make_tuple(-5.970362, 1.991202, -61.859150));
            else
            {
                play4 = true;
                play3 = false;
            }
        }

        fighter1Model = glm::translate(fighter1Model, glm::vec3(get<0>(fighter1.position), get<1>(fighter1.position), get<2>(fighter1.position)));
        fighter1Model = glm::rotate(fighter1Model, glm::radians(fighterTiltAngle), glm::vec3(1.0f, 0.0f, 0.0f));
        fighter1Model = glm::scale(fighter1Model, glm::vec3(0.3f, 0.3f, 0.3f));
        ourShader.setMat4("model", fighter1Model);
        fighter1.Draw(ourShader);

        // render the hangar model
        glm::mat4 hangarModel = glm::mat4(1.0f);
        hangarModel = glm::translate(hangarModel, glm::vec3(-30.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        hangarModel = glm::scale(hangarModel, glm::vec3(0.1f, 0.1f, 0.1f));       // it's a bit too big for our scene, so scale it down
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
        switchCameraPosition(cameraPos1, cameraFront1, false, fighter1);
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        switchCameraPosition(cameraPos2, cameraFront2, false, fighter1);
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        switchCameraPosition(cameraPos3, cameraFront3, false, fighter1);
    }

    // variables for movement
    static float fighterVelocity = 0.0f;
    static float fighterAcceleration = 10.0f;
    static float fighterMaxSpeed = 5.0f;
    static float fighterDamping = 5.0f;
    static float fighterMinX = -5.0f;
    static float fighterMaxX = 5.0f;

    static float maxTiltAngle = 15.0f;
    static float tiltSpeed = 5.0f;

    // different boundaries for each position
    static float fighterMinXPos1 = -5.0f;
    static float fighterMaxXPos1 = 5.0f;

    static float fighterMinXPos2 = -10.0f;
    static float fighterMaxXPos2 = 10.0f;

    static float fighterMinXPos3 = -7.0f;
    static float fighterMaxXPos3 = 7.0f;

    // movement for the 3 positions
    if (camera.Position == cameraPos1 || camera.Position == cameraPos2 || camera.Position == cameraPos3)
    {
        float targetTiltAngle = 0.0f;
        bool isMoving = false;
        bool atBoundary = false;

        // check for movement and determine target tilt angle and accelerates left/right
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        {
            isMoving = true;
            targetTiltAngle = -maxTiltAngle;
            fighterVelocity -= fighterAcceleration * deltaTime;
        }
        else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        {
            isMoving = true;
            targetTiltAngle = maxTiltAngle;
            fighterVelocity += fighterAcceleration * deltaTime;
        }
        else
        {
            // gradually reduce velocity when no key is pressed
            if (fighterVelocity > 0.0f)
            {
                fighterVelocity -= fighterDamping * deltaTime;
                if (fighterVelocity < 0.0f)
                    fighterVelocity = 0.0f;
            }
            else if (fighterVelocity < 0.0f)
            {
                fighterVelocity += fighterDamping * deltaTime;
                if (fighterVelocity > 0.0f)
                    fighterVelocity = 0.0f;
            }
        }

        // clamp velocity to maximum speed
        fighterVelocity = glm::clamp(fighterVelocity, -fighterMaxSpeed, fighterMaxSpeed);

        // update fighter position
        std::get<2>(fighter1.position) += fighterVelocity * deltaTime;

        // apply different boundaries based on the camera position
        if (camera.Position == cameraPos1)
        {

            if (std::get<2>(fighter1.position) < fighterMinXPos1)
            {
                std::get<2>(fighter1.position) = fighterMinXPos1;
                fighterVelocity = 0.0f;
                atBoundary = true;
            }
            if (std::get<2>(fighter1.position) > fighterMaxXPos1)
            {
                std::get<2>(fighter1.position) = fighterMaxXPos1;
                fighterVelocity = 0.0f;
                atBoundary = true;
            }
        }
        else if (camera.Position == cameraPos2)
        {

            if (std::get<2>(fighter1.position) < fighterMinXPos2)
            {
                std::get<2>(fighter1.position) = fighterMinXPos2;
                fighterVelocity = 0.0f;
                atBoundary = true;
            }
            if (std::get<2>(fighter1.position) > fighterMaxXPos2)
            {
                std::get<2>(fighter1.position) = fighterMaxXPos2;
                fighterVelocity = 0.0f;
                atBoundary = true;
            }
        }
        else if (camera.Position == cameraPos3)
        {
            if (std::get<2>(fighter1.position) < fighterMinXPos3)
            {
                std::get<2>(fighter1.position) = fighterMinXPos3;
                fighterVelocity = 0.0f;
                atBoundary = true;
            }
            if (std::get<2>(fighter1.position) > fighterMaxXPos3)
            {
                std::get<2>(fighter1.position) = fighterMaxXPos3;
                fighterVelocity = 0.0f;
                atBoundary = true;
            }
        }

        // smoothly interpolate the tilt angle
        if (isMoving)
        {
            // apply tilt based on movement
            fighterTiltAngle = glm::mix(fighterTiltAngle, targetTiltAngle, tiltSpeed * deltaTime);
        }
        else if (!atBoundary)
        {
            // reset tilt to 0 only if not at boundary
            fighterTiltAngle = glm::mix(fighterTiltAngle, 0.0f, tiltSpeed * deltaTime);
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

    // print the camera position and front direction for debugging
    std::cout << "Camera Position: (" << camera.Position.x << ", " << camera.Position.y << ", " << camera.Position.z << ") ";
    std::cout << "Camera Front: (" << camera.Front.x << ", " << camera.Front.y << ", " << camera.Front.z << ")" << std::endl;
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
