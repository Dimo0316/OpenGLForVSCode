#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <geometry/BoxGeometry.h>
#include <geometry/SphereGeometry.h>

#define STB_IMAGE_IMPLEMENTATION
#include <tool/stb_image.h>
#include <tool/camera.h>
#include <tool/shader.h>
#include <tool/gui.h>
std::string Shader::dirName;
using namespace std;

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;

float deltaTime = 0.0f; // 当前帧与上一帧的时间差
float lastFrame = 0.0f; // 上一帧的时间
float lastX = SCREEN_WIDTH/2.0f, lastY = SCREEN_HEIGHT/2.0f;
bool firstMouse = true;

Camera camera(glm::vec3(0.0, 1.0, 5.0));


void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
unsigned int loadTexture(char const * path);


int main(int argc, char *argv[])
{   
    const char *glsl_version = "#version 330";
    Shader::dirName = argv[1];
    glfwInit();
    // 设置主要和次要版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口对象
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

    // 注册窗口变化监听
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);

    Shader ourShader("./shader/vertex.glsl", "./shader/fragment.glsl");
    Shader lightShader("./shader/light_object_vert.glsl", "./shader/light_object_frag.glsl");
    unsigned int diffuseMap = loadTexture("./static/texture/container2.png");
    unsigned int specularMap = loadTexture("./static/texture/container2_specular.png");
    unsigned int specularColorMap = loadTexture("./static/texture/lighting_maps_specular_color.png");
    unsigned int emissionMap = loadTexture("./static/texture/matrix.jpg");

    BoxGeometry boxGeometry(1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
    SphereGeometry sphereGeometry(0.3, 10.0, 10.0);

    // float factor = 0.0;

    glm::vec3 view_translate = glm::vec3(0.0f, 0.0f, -5.0f);
    glm::vec3 lightPosition = glm::vec3(1.0, 1.5, 0.0); // 光照位置
    glm::vec4 clear_color = glm::vec4(25.0 / 255.0, 25.0 / 255.0, 25.0 / 255.0, 1.0);
    // glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
 
 // 要先启用这个 shader类 然后再对类内的 uniform 进行赋值
    // 设置光源属性
    ourShader.use();
    ourShader.setVec3("light.ambient",  0.4f, 0.4f, 0.4f);
    ourShader.setVec3("light.diffuse",  0.8f, 0.8f, 0.8f); // 将光照调暗了一些以搭配场景
    ourShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

    // 设置材质属性
    ourShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);  // 镜面反射
    ourShader.setFloat("material.shininess", 32.0f);
    ourShader.setInt("material.diffuse", 0);        
    ourShader.setInt("material.specular", 1);
    ourShader.setInt("material.specularColor", 2);
    ourShader.setInt("material.emission", 3);
    ourShader.setInt("factor", glfwGetTime());  

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);

        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();

        
        ourShader.setInt("factor", glfwGetTime());  
        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1); 
        glBindTexture(GL_TEXTURE_2D, specularMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, specularColorMap);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, emissionMap);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::mat4(1.0f);
        view = glm::translate(view, view_translate);
        glm::vec3 lightPos = glm::vec3(lightPosition.x * glm::sin(glfwGetTime()), lightPosition.y, lightPosition.z);
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1f, 100.0f);
        

        glm::mat4 model = glm::mat4(1.0f);
        float rotate = glfwGetTime() * 0.2f;
        glm::qua<float> qu = glm::qua<float>(glm::vec3(rotate, rotate, rotate));
        model = glm::mat4_cast(qu);

        ourShader.setMat4("model", model);
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        ourShader.setVec3("lightPos", lightPos);
        ourShader.setVec3("viewPos", camera.Position);
    
        glBindVertexArray(boxGeometry.VAO);

        glDrawElements(GL_TRIANGLES, boxGeometry.indices.size(), GL_UNSIGNED_INT, 0);

        // 绘制圆形光源
        lightShader.use();
        
        model = glm::translate(glm::mat4(1.0f), lightPos);

        
        lightShader.setMat4("model", model);
        lightShader.setMat4("view", view);
        lightShader.setMat4("projection", projection);

        glBindVertexArray(sphereGeometry.VAO);
        glDrawElements(GL_TRIANGLES, sphereGeometry.indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    boxGeometry.dispose();
    sphereGeometry.dispose();
    glfwTerminate();

    return 0;
}


// 窗口渲染回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
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


// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
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