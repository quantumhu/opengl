#define GL_SILENCE_DEPRECATION
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// #include <glm/gtx/string_cast.hpp>  // for glm::to_string

#include <iostream>
#include <vector>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// vertex shader is special because it receives vertex data as its input
// to define how the vertex data is organized, we specify location
// setting the location in GLSL directly saves OpenGL some work of figuring it out with `glGetAttribLocation`
const char *vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aTexCoord;\n"
        "out vec3 ourColor;\n"
        "out vec2 TexCoord;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "void main()\n"
        "{\n"
        " gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        " TexCoord = aTexCoord;\n"
        "}";

const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D texture1;\n"
        "uniform sampler2D texture2;\n"
        "uniform float mixU;\n"
        "void main()\n"
        "{\n"
        "  FragColor = mix(texture(texture1, TexCoord), texture(texture2, vec2(-TexCoord.x, TexCoord.y)), mixU);\n"   // value of 0.2 returns 20% of the second
        "}";

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

namespace Camera
{
    glm::vec3 pos = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 frontVec = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 upVec = glm::vec3(0.0f, 1.0f, 0.0f);
    const float speed = 2.5f;
    float zoom_level = 45.0f;
    const float window_ratio = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

    glm::mat4 projectionMatrix = glm::perspective(glm::radians(zoom_level), window_ratio, 0.1f, 100.0f);

    void mouse_callback(GLFWwindow* window, double xpos, double ypos)
    {
        static double lastX = xpos;
        static double lastY = ypos;
        static float sensitivityY = 0.003f;
        static float sensitivityX = sensitivityY / window_ratio;

        frontVec += glm::vec3(
                              (xpos - lastX) * sensitivityX,
                              (lastY - ypos) * sensitivityY, 
                              0
                             );
        lastX = xpos;
        lastY = ypos;
    }

    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        zoom_level -= (float)yoffset;
        if (zoom_level < 1.0f)
        {
            zoom_level = 1.0f;
        }
        else if (zoom_level > 45.0f)
        {
            zoom_level = 45.0f;
        }
        projectionMatrix = glm::perspective(glm::radians(zoom_level), window_ratio, 0.1f, 100.0f);
    }

    glm::mat4 get_view_matrix()
    {
        return glm::lookAt(pos, pos + frontVec, upVec);
    }

    void W(float deltaTime)
    {
        pos += speed * deltaTime * frontVec;
    }

    void A(float deltaTime)
    {
        pos -= glm::normalize(glm::cross(frontVec, upVec)) * speed * deltaTime;
    }

    void S(float deltaTime)
    {
        pos -= speed * deltaTime * frontVec;
    }

    void D(float deltaTime)
    {
        pos += glm::normalize(glm::cross(frontVec, upVec)) * speed * deltaTime;
    }
};

float mix_percent = 0.2f;

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
        if (mix_percent < 1.0f) mix_percent += 0.01f;
    }
    else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        if (mix_percent > 0.0f) mix_percent -= 0.01f;
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

class ShaderHelper
{
    private:
        unsigned int m_shaderProgram;
        std::vector<unsigned int> m_shaders;

    public:
        ShaderHelper()
        {
            m_shaderProgram = glCreateProgram();
        }

        ~ShaderHelper()
        {
            for (std::vector<unsigned int>::iterator s = m_shaders.begin(); s != m_shaders.end(); ++s)
            {
                glDeleteShader(*s);
            }
            glDeleteProgram(m_shaderProgram);
        }

        /* Compiles a shader and adds it to the helper object. */
        bool add_shader(GLenum type, const char **source)
        {
            if (type != GL_VERTEX_SHADER && type != GL_FRAGMENT_SHADER)
            {
                std::cout << "ERROR! Provided shader is not compatible.\n" << std::endl;
                return false;
            }

            if (source == nullptr)
            {
                std::cout << "ERROR! source string cannot be NULL.\n" << std::endl;
                return false;
            }

            unsigned int shader = glCreateShader(type);
            glShaderSource(shader, 1, source, NULL);
            glCompileShader(shader);

            int success;
            char infoLog[512];
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 512, NULL, infoLog);
                printf("ERROR! Shader compilation failed at %s:%d. Error Log: %s\n", __FILE__, __LINE__, infoLog);
                return false;
            }

            m_shaders.push_back(shader);
            glAttachShader(m_shaderProgram, shader);
            return true;
        }

        /* Links the known shaders to the program */
        bool link_shaders()
        {
            glLinkProgram(m_shaderProgram);

            int success;
            char infoLog[512];
            glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
            if(!success) {
                glGetProgramInfoLog(m_shaderProgram, 512, NULL, infoLog);
                printf("ERROR! Shader linking failed at %s:%d. Error Log: %s\n", __FILE__, __LINE__, infoLog);
                return false;
            }
            return true;
        }

        void use()
        {
            glUseProgram(m_shaderProgram);
        }

        int get_uniform_location(const char *name)
        {
            return glGetUniformLocation(m_shaderProgram, name);
        }

        void set_uniform(const char *name, GLint i)
        {
            glUniform1i(get_uniform_location(name), i);
        }

        void set_uniform(const char *name, GLfloat f)
        {
            glUniform1f(get_uniform_location(name), f);
        }

        void set_uniform_matrix4(const char *name, GLsizei count, GLboolean transpose, const GLfloat *value)
        {
            glUniformMatrix4fv(get_uniform_location(name), count, transpose, value);
        }

        unsigned int load_texture(const char *filename, bool transparent)
        {
            int width, height, nrChannels;
            unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);

            if (data == nullptr)
            {
                std::cout << "Could not read texture image " << filename << "\n" << std::endl;
                return false;
            }

            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, transparent ? GL_RGBA : GL_RGB, width, height, 0, transparent ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(data);
            
            return texture;
        }
};

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

    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f
    };

    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f, 0.0f, 0.0f),
        glm::vec3( 2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f, 2.0f, -2.5f),
        glm::vec3( 1.5f, 0.2f, -1.5f),
        glm::vec3(-1.3f, 1.0f, -1.5f)
    };

    ShaderHelper sh;
    sh.add_shader(GL_VERTEX_SHADER, &vertexShaderSource);
    sh.add_shader(GL_FRAGMENT_SHADER, &fragmentShaderSource);
    sh.link_shaders();

    // textures
    unsigned int texture1 = sh.load_texture("container.jpg", false);
    unsigned int texture2 = sh.load_texture("awesomeface.png", true);

    sh.use();
    sh.set_uniform("texture1", 0);
    sh.set_uniform("texture2", 1);

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    
    // vertex attribute are disabled by default
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glEnable(GL_DEPTH_TEST);

    while(!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sh.set_uniform_matrix4("projection", 1, GL_FALSE, glm::value_ptr(Camera::projectionMatrix));
        sh.set_uniform("mixU", mix_percent);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        sh.use();

        glm::mat4 view = Camera::get_view_matrix();
        sh.set_uniform_matrix4("view", 1, GL_FALSE, glm::value_ptr(view));

        glBindVertexArray(VAO);
        for (unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            sh.set_uniform_matrix4("model", 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();

    return 0;
}