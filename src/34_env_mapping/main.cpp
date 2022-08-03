#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <map>

#include <tool/shader.h>
#include <tool/camera.h>
#include <geometry/BoxGeometry.h>
#include <geometry/PlaneGeometry.h>
#include <geometry/SphereGeometry.h>

#define STB_IMAGE_IMPLEMENTATION
#include <tool/stb_image.h>

#include <tool/gui.h>
#include <tool/mesh.h>
#include <tool/model.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void processInput(GLFWwindow *window);
unsigned int loadTexture(char const *path);
unsigned int loadCubemap(vector<std::string> faces);
void drawSkyBox(Shader shader, BoxGeometry geometry, unsigned int cubeMap);

std::string Shader::dirName;

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;

// camera value
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// delta time
float deltaTime = 0.0f;
float lastTime = 0.0f;

float lastX = SCREEN_WIDTH / 2.0f; // 鼠标上一帧的位置
float lastY = SCREEN_HEIGHT / 2.0f;

Camera camera(glm::vec3(0.0, 1.0, 6.0));

using namespace std;

int main(int argc, char *argv[])
{
    Shader::dirName = argv[1];
   
    
    glfwInit();
    // 设置主要和次要版本
    const char *glsl_version = "#version 330";

    // 片段着色器将作用域每一个采样点（采用4倍抗锯齿，则每个像素有4个片段（四个采样点））
    // glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 窗口对象
    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 设置视口
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    // glEnable(GL_PROGRAM_POINT_SIZE);
    
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // 鼠标键盘事件
    // 1.注册窗口变化监听
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // 2.鼠标事件
    glfwSetCursorPosCallback(window, mouse_callback);
    PlaneGeometry planeGeometry(10.0, 10.0, 10.0, 10.0);
    BoxGeometry boxGeometry(1.0, 1.0, 1.0);
    PlaneGeometry grassGeometry(1.0, 1.0);               // 草丛
    SphereGeometry sphereGeometry(0.04, 10.0, 10.0);
    BoxGeometry skyboxGeometry(1.0, 1.0, 1.0);           // 天空盒
    ImVec4 clear_color = ImVec4(25.0 / 255.0, 25.0 / 255.0, 25.0 / 255.0, 1.0); // 25, 25, 25
    
    Shader skyboxShader("./shader/cube_map_vert.glsl", "./shader/cube_map_frag.glsl");
    Shader reflectShader("./shader/reflect_object_vert.glsl", "./shader/reflect_object_frag.glsl");
    Shader refractShader("./shader/refract_object_vert.glsl", "./shader/refract_object_frag.glsl");


    unsigned int woodMap = loadTexture("./static/texture/wood.png");           // 地面
    unsigned int brickMap = loadTexture("./static/texture/brick_diffuse.jpg"); // 砖块
    unsigned int grassMap = loadTexture("./static/texture/blending_transparent_window.png"); // 草丛
    // 天空盒贴图
    vector<string> faces{
        "./static/texture/Park3Med/px.jpg",
        "./static/texture/Park3Med/nx.jpg",
        "./static/texture/Park3Med/py.jpg",
        "./static/texture/Park3Med/ny.jpg",
        "./static/texture/Park3Med/pz.jpg",
        "./static/texture/Park3Med/nz.jpg"};
    unsigned int cubemapTexture = loadCubemap(faces);
    
    Model ourModel("./static/model/walt/WaltHead.obj");

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastTime;
        lastTime = currentFrame;

        // 渲染指令
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 绘制天空盒
        drawSkyBox(skyboxShader, skyboxGeometry, cubemapTexture);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0, 0.0, 0.0));
        model = glm::scale(model, glm::vec3(0.03f, 0.03f, 0.03f));
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

        refractShader.use();
        refractShader.setMat4("model", model);
        refractShader.setMat4("view", view);
        refractShader.setMat4("projection", projection);
        refractShader.setVec3("cameraPos", camera.Position);
        refractShader.setVec3("objectColor", glm::vec3(0.0, 0.0, 0.0));

        ourModel.Draw(refractShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-3.0, 0.0, 0.0));
        model = glm::scale(model, glm::vec3(0.03f, 0.03f, 0.03f));

        refractShader.setMat4("model", model);

        refractShader.setVec3("objectColor", glm::vec3(0.1, 0.0, 0.1));
        ourModel.Draw(refractShader);

        reflectShader.use();
        reflectShader.setMat4("view", view);
        reflectShader.setMat4("projection", projection);
        reflectShader.setVec3("cameraPos", camera.Position);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.0, 0.0, 0.0));
        model = glm::scale(model, glm::vec3(0.03f, 0.03f, 0.03f));
        reflectShader.setMat4("model", model);
        reflectShader.setVec3("objectColor", glm::vec3(0.1, 0.1, 0.0));
        ourModel.Draw(reflectShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    boxGeometry.dispose();
    planeGeometry.dispose();
    sphereGeometry.dispose();
    skyboxGeometry.dispose();
    glfwTerminate();

    return 0;
}

// 窗口变动监听
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// 从窗口输入 ESC 退出窗口, 按 WSAD 进行平移
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    float cameraSpeed = 2.5f * deltaTime; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

// 鼠标移动监听
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// 加载纹理贴图
unsigned int loadTexture(char const *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    // 图像y轴翻转
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// 加载立方体贴图
unsigned int loadCubemap(vector<std::string> faces)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  // 此处需要将y轴旋转关闭，若之前调用过loadTexture
  stbi_set_flip_vertically_on_load(false);

  int width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); i++)
  {
    unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                   0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    }
    else
    {
      std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
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


// *******************method*********************
void drawSkyBox(Shader shader, BoxGeometry geometry, unsigned int cubeMap)
{
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_DEPTH_TEST);

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

    shader.use();
    view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // 移除平移分量

    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
    glBindVertexArray(geometry.VAO);
    glDrawElements(GL_TRIANGLES, geometry.indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    view = camera.GetViewMatrix();
}