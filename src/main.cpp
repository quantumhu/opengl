#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>

#include "ShaderHelper.h"
#include "Camera.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Globals
float g_mix_percent = 0.2f;
glm::vec3 g_lightPos = glm::vec3(1.2f, 1.0f, 2.0f);

const char *vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aNormal;\n"
        "layout (location = 2) in vec2 aTexCoord;\n"
        "out vec3 Normal;\n"
        "out vec3 FragPos;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "void main()\n"
        "{\n"
        "  gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "  Normal = aNormal;\n"
        "  FragPos = vec3(model * vec4(aPos, 1.0));\n"
        "}";

const char *fragment2ShaderSource = "#version 330 core\n"
        "in vec3 Normal;\n"
        "in vec3 FragPos;\n"
        "out vec4 FragColor;\n"
        "uniform vec3 objectColor;\n"
        "uniform vec3 lightColor;\n"
        "uniform vec3 lightPos;\n"
        "uniform vec3 viewPos;\n"
        "void main()\n"
        "{\n"
        "  float specularStr = 0.5;\n"
        "  float ambientStrength = 0.1;\n"
        "  vec3 ambient = ambientStrength * lightColor;\n"
        "  vec3 norm    = normalize(Normal);\n"
        "  vec3 lightDir = normalize(lightPos - FragPos);\n"
        "  float diff = max(dot(norm, lightDir), 0.0);\n"
        "  vec3 diffuse = diff * lightColor;\n"
        "  vec3 viewDir = normalize(viewPos - FragPos);\n"
        "  vec3 reflectDir = reflect(-lightDir, norm);\n"   // reflect arg 1 vector FROM light TO fragment
        "  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);\n"
        "  vec3 specular = specularStr * spec * lightColor;\n"
        "  vec3 result  = (ambient + diffuse + specular) * objectColor;\n"
        "  FragColor = vec4(result, 1.0);\n"
        "}";

const char *lightSourceVertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "void main()\n"
        "{\n"
        "  gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "}";

const char *lightSourceFragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "  FragColor = vec4(1.0);\n"
        "}";

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    Camera::set_window_ratio((float)width, (float)height);
}

void processInput(GLFWwindow *window)
{
    static float deltaTime = 0.0f;
    static float lastFrame = 0.0f;
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ||  glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        if (g_mix_percent < 1.0f) g_mix_percent += 0.01f;
    }
    else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        if (g_mix_percent > 0.0f) g_mix_percent -= 0.01f;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        Camera::W(deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        Camera::S(deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        Camera::A(deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        Camera::D(deltaTime);
    }
}

GLFWwindow* window_setup()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // needed for macos

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "My OpenGL Window", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // camera setup
    Camera::set_window_ratio((float)WINDOW_WIDTH, (float)WINDOW_HEIGHT);
    glfwSetCursorPosCallback(window, Camera::mouse_callback);
    glfwSetScrollCallback(window, Camera::scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return NULL;
    }

    stbi_set_flip_vertically_on_load(true);

    return window;
}

int main()
{
    GLFWwindow *window = window_setup();
    if (window == nullptr) return 1;

    // Camera::toggle_fps_movement(true);

    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    ShaderHelper sh;
    sh.add_shader(GL_VERTEX_SHADER, &vertexShaderSource);
    sh.add_shader(GL_FRAGMENT_SHADER, &fragment2ShaderSource);
    sh.link_shaders();

    // textures
    unsigned int texture1 = sh.load_texture("assets/container.jpg", false);
    unsigned int texture2 = sh.load_texture("assets/awesomeface.png", true);

    sh.use();
    sh.set_uniform("texture1", 0);
    sh.set_uniform("texture2", 1);
    sh.set_uniform("objectColor", 1.0f, 0.5f, 0.31f);
    sh.set_uniform("lightColor", 1.0f, 1.0f, 1.0f);
    sh.set_uniform("lightPos", g_lightPos);

    // things bound when VAO is bound are attached to that object
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO;
    // create one VBO. the function returns a "number" that represent internally where the VBO would go
    glGenBuffers(1, &VBO);
    // easy way to reference VBO, by binding it to the keyword and using that keyword instead
    // only one of each type can be binding to its associated keyword
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // copy our data into a buffer for OpenGL
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // vertex attribute is an attribute unique to each vector
    // first arg is the # of the vertex attribute 
    // second arg is the size of the vertex attribute, related to third arg (datatype)
    // fourth arg is about whether or not to normalize to 0/-1 and 1
    // fifth arg is distance between each vertex attribute. It would be the width of each vertex attribute
    //   if we know it is tightly packed we can pass 0 to let opengl figure out the stride
    // sixth arg is the offset of where the vertex attribute data begins in the buffer
    // THE VBO BOUND TO GL_ARRAY_BUFFER IS THE ONE OPENGL uses for vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    
    // vertex attribute are disabled by default
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    // glEnableVertexAttribArray(2);

    // lighting
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    // we only need to bind to the VBO, the container’s VBO’s data
    // already contains the data.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    ShaderHelper lightsh;
    lightsh.add_shader(GL_VERTEX_SHADER, &lightSourceVertexShaderSource);
    lightsh.add_shader(GL_FRAGMENT_SHADER, &lightSourceFragmentShaderSource);
    lightsh.link_shaders();

    Camera::setup_hud(g_lightPos, glm::vec3(1.0f));

    glEnable(GL_DEPTH_TEST);

    while(!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sh.use();

        sh.set_uniform_matrix4("projection", 1, GL_FALSE, glm::value_ptr(Camera::projectionMatrix));
        sh.set_uniform("mixU", g_mix_percent);
        sh.set_uniform("viewPos", Camera::pos);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        glm::mat4 view = Camera::get_view_matrix();
        sh.set_uniform_matrix4("view", 1, GL_FALSE, glm::value_ptr(view));

        glBindVertexArray(VAO);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f));
        sh.set_uniform_matrix4("model", 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        lightsh.use();
        lightsh.set_uniform_matrix4("projection", 1, GL_FALSE, glm::value_ptr(Camera::projectionMatrix));
        lightsh.set_uniform_matrix4("view", 1, GL_FALSE, glm::value_ptr(view));

        glBindVertexArray(lightVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, g_lightPos);
        model = glm::scale(model, glm::vec3(0.2f));
        lightsh.set_uniform_matrix4("model", 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        Camera::draw_hud();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}