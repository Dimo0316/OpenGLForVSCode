#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#include <tool/shader.h>
#include <tool/camera.h>
#include <geometry/BoxGeometry.h>
#include <geometry/PlaneGeometry.h>
#include <geometry/SphereGeometry.h>

#define STB_IMAGE_IMPLEMENTATION
#include <tool/stb_image.h>

#include <tool/gui.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void processInput(GLFWwindow *window);
unsigned int loadTexture(char const *path);

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
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // 鼠标键盘事件
    // 1.注册窗口变化监听
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // 2.鼠标事件
    glfwSetCursorPosCallback(window, mouse_callback);

    Shader sceneShader("./shader/scene_vert.glsl", "./shader/scene_frag.glsl");
    Shader lightObjectShader("./shader/light_object_vert.glsl", "./shader/light_object_frag.glsl");

    PlaneGeometry planeGeometry(10.0, 10.0, 10.0, 10.0);
    BoxGeometry boxGeometry(1.0, 1.0, 1.0);
    SphereGeometry sphereGeometry(0.04, 10.0, 10.0);

    unsigned int woodMap = loadTexture("./static/texture/wood.png");           // 地面
    unsigned int brickMap = loadTexture("./static/texture/brick_diffuse.jpg"); // 砖块

    sceneShader.use();

    float fov = 45.0f; // 视锥体的角度
    ImVec4 clear_color = ImVec4(25.0 / 255.0, 25.0 / 255.0, 25.0 / 255.0, 1.0); // 25, 25, 25

    glm::vec3 lightPosition = glm::vec3(1.0, 2.5, 2.0); // 光照位置

    // 传递材质属性
    sceneShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
    sceneShader.setFloat("material.shininess", 32.0f);

    // 设置平行光光照属性
    sceneShader.setVec3("directionLight.ambient", 0.01f, 0.01f, 0.01f);
    sceneShader.setVec3("directionLight.diffuse", 0.2f, 0.2f, 0.2f); // 将光照调暗了一些以搭配场景
    sceneShader.setVec3("directionLight.specular", 1.0f, 1.0f, 1.0f);

    // 设置衰减
    sceneShader.setFloat("light.constant", 1.0f);
    sceneShader.setFloat("light.linear", 0.09f);
    sceneShader.setFloat("light.quadratic", 0.032f);

    // 点光源的位置
    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f, 1.0f, 1.5f),
        glm::vec3(2.3f, 3.0f, -4.0f),
        glm::vec3(-4.0f, 2.0f, 1.0f),
        glm::vec3(1.4f, 2.0f, 0.3f)};
    // 点光源颜色
    glm::vec3 pointLightColors[] = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)};

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastTime;
        lastTime = currentFrame;

        // 渲染指令
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sceneShader.use();

        glBindTexture(GL_TEXTURE_2D, woodMap);

        float radius = 5.0f;
        float camX = sin(glfwGetTime() * 0.5) * radius;
        float camZ = cos(glfwGetTime() * 0.5) * radius;

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

        glm::vec3 lightPos = glm::vec3(lightPosition.x * glm::sin(glfwGetTime()) * 2.0, lightPosition.y, lightPosition.z);

        sceneShader.setMat4("view", view);
        sceneShader.setMat4("projection", projection);

        sceneShader.setVec3("directionLight.direction", lightPos); // 光源位置
        sceneShader.setVec3("viewPos", camera.Position);

        pointLightPositions[0].z = camZ;
        pointLightPositions[0].x = camX;

        for (unsigned int i = 0; i < 4; i++)
        {
            // 设置点光源属性
            sceneShader.setVec3("pointLights[" + std::to_string(i) + "].position", pointLightPositions[i]);
            sceneShader.setVec3("pointLights[" + std::to_string(i) + "].ambient", 0.01f, 0.01f, 0.01f);
            sceneShader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", pointLightColors[i]);
            sceneShader.setVec3("pointLights[" + std::to_string(i) + "].specular", 1.0f, 1.0f, 1.0f);

            // 设置衰减
            sceneShader.setFloat("pointLights[" + std::to_string(i) + "].constant", 1.0f);
            sceneShader.setFloat("pointLights[" + std::to_string(i) + "].linear", 0.09f);
            sceneShader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.032f);
        }

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));

        sceneShader.setFloat("uvScale", 4.0f);
        sceneShader.setMat4("model", model);

        glBindVertexArray(planeGeometry.VAO);
        glDrawElements(GL_TRIANGLES, planeGeometry.indices.size(), GL_UNSIGNED_INT, 0);

        // 绘制砖块
        glBindTexture(GL_TEXTURE_2D, brickMap);
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(2.0, 2.0, 2.0));
        model = glm::translate(model, glm::vec3(1.0, 0.5, -1.0));

        sceneShader.setFloat("uvScale", 1.0f);
        sceneShader.setMat4("model", model);

        glBindVertexArray(boxGeometry.VAO);
        glDrawElements(GL_TRIANGLES, boxGeometry.indices.size(), GL_UNSIGNED_INT, 0);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0, 0.5, 2.0));
        sceneShader.setMat4("model", model);

        glBindVertexArray(boxGeometry.VAO);
        glDrawElements(GL_TRIANGLES, boxGeometry.indices.size(), GL_UNSIGNED_INT, 0);

        // 绘制灯光物体
        lightObjectShader.use();
        lightObjectShader.setMat4("view", view);
        lightObjectShader.setMat4("projection", projection);

        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);

        lightObjectShader.setMat4("model", model);
        lightObjectShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));

        glBindVertexArray(sphereGeometry.VAO);
        glDrawElements(GL_TRIANGLES, sphereGeometry.indices.size(), GL_UNSIGNED_INT, 0);

        for (unsigned int i = 0; i < 4; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);

            lightObjectShader.setMat4("model", model);
            lightObjectShader.setVec3("lightColor", pointLightColors[i]);

            glBindVertexArray(sphereGeometry.VAO);
            glDrawElements(GL_TRIANGLES, sphereGeometry.indices.size(), GL_UNSIGNED_INT, 0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    boxGeometry.dispose();
    planeGeometry.dispose();
    sphereGeometry.dispose();
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