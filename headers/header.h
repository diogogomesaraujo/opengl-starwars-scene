#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <shader_m.h>
#include <camera.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"


#include <iostream>

#include <string>
#include <vector>

using namespace std;