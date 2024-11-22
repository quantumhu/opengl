#include "Camera.h"

#include "arrow_v4.h"

namespace Camera
{
    glm::vec3 pos = glm::vec3(0.5f, 0.5f, 6.0f);
    glm::vec3 frontVec = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 upVec = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::vec3 direction = glm::vec3(0.0f);

    const float speed = 3.0f;
    float zoomLevel = 45.0f;
    bool fpsMovement = false;

    float windowRatio;
    glm::mat4 projectionMatrix;

    ShaderHelper *hudShader = nullptr;
    unsigned int hudVAO;
    unsigned int hudVBO;
    unsigned int hudVEO;

    // 0 degree yaw is 1x, 0z
    // 90 degree is 0x, 1z
    float yaw = 0.0f;
    float pitch = 0.0f;
    float offsetYaw = 0.0f;
    bool isStartYawCalced = false;

    const char *hudVertexSource = "#version 330 core\n"
        "layout (location = 0) in vec3 pos;\n"
        "layout (location = 1) in float colIntensity;\n"
        "uniform mat4 model;\n"
        "uniform mat4 rotation;\n"
        "out float ColIntensity;\n"
        "out vec3 FragPos;\n"
        "void main()\n"
        "{\n"
        "  gl_Position = rotation * model * vec4(pos, 1.0);\n"
        "  FragPos = vec3(model * vec4(pos, 1.0));\n"
        "  ColIntensity = colIntensity;\n"
        "}";
    
    // Colour intensity is pre-calculated so the inside of the arrow tip is dark
    const char *hudFragmentSource = "#version 330 core\n"
        "in vec3 FragPos;\n"
        "in float ColIntensity;\n"
        "uniform vec3 objectColour;\n"
        "uniform vec3 lightColour;\n"
        "uniform vec3 lightPos;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "  vec3 colour = lightColour * objectColour * ColIntensity;\n"
        "  FragColor = vec4(colour, 1.0);\n"
        "}";

    void set_window_ratio(float width, float height)
    {
        windowRatio = width / height;
        projectionMatrix = glm::perspective(glm::radians(zoomLevel), windowRatio, 0.1f, 100.0f);
    }

    void setup_hud(glm::vec3 lightPos, glm::vec3 lightColour)
    {
        if (hudShader == nullptr)
        {
            hudShader = new ShaderHelper();
            hudShader->add_shader(GL_VERTEX_SHADER, &hudVertexSource);
            hudShader->add_shader(GL_FRAGMENT_SHADER, &hudFragmentSource);
            hudShader->link_shaders();

            hudShader->set_uniform("lightColour", lightColour);
            hudShader->set_uniform("lightPos", lightPos);

            glGenVertexArrays(1, &hudVAO);
            glGenBuffers(1, &hudVBO);
            glGenBuffers(1, &hudVEO);

            glBindVertexArray(hudVAO);
            glBindBuffer(GL_ARRAY_BUFFER, hudVBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hudVEO);
            
            glBufferData(GL_ARRAY_BUFFER, sizeof(arrow_v4_buffer_data), arrow_v4_buffer_data, GL_STATIC_DRAW);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(arrow_v4_elements_data), arrow_v4_elements_data, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, arrow_v4_buffer_data_stride * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, arrow_v4_buffer_data_stride * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glBindVertexArray(0);

            if (!isStartYawCalced)
            {
                // needs to be unit vector in x or z direction
                bool x_conforms = fabs(frontVec.x) == 1.0f;
                bool z_conforms = fabs(frontVec.z) == 1.0f;
                assert(x_conforms ^ z_conforms);
                isStartYawCalced = true;

                if (x_conforms)
                {
                    if (frontVec.x > 0.0f) yaw = 0.0f;
                    else yaw = offsetYaw = 180.0f;
                }
                else
                {
                    if (frontVec.z > 0.0f) yaw = offsetYaw = 90.0f;
                    else yaw = offsetYaw = 270.0f;
                }
            }
        }
    }

    void draw_hud()
    {
        // Arrow model faces vec3(0, 1, 0) positive y-axis by default
        // Hud arrows DO NOT follow OpenGL axis directions. This X-axis is flipped compared to OpenGL
        // Hud arrows X-axis faces where the camera starts looking

        static glm::vec3 rotateToFaceX = glm::vec3(1.0f, 0.0f, 0.0f);
        static glm::vec3 rotateToFaceZ = glm::vec3(0.0f, 0.0f, 1.0f);

        static int arrow_v4_elements_count = sizeof(arrow_v4_elements_data)/sizeof(arrow_v4_elements_data[0]);

        static glm::mat4 *baseTransform = nullptr;
        if (baseTransform == nullptr)
        {
            baseTransform = new glm::mat4(1.0f);
            *baseTransform = glm::translate(*baseTransform, glm::vec3(0.7f, -0.8f, 0.0f));
            *baseTransform = glm::scale(*baseTransform, glm::vec3(0.015f));
        }

        glm::mat4 rotation = glm::mat4(*baseTransform);
        rotation = glm::rotate(rotation, glm::radians(offsetYaw - yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        rotation = glm::rotate(rotation, glm::radians(-pitch), glm::vec3(1.0f, 0.0f, 0.0f));

        hudShader->set_uniform_matrix4("rotation", 1, GL_FALSE, glm::value_ptr(rotation));
        
        glm::mat4 model;
        // std::cout << "yaw: " << yaw << " axis-yaw: " << (-yaw + offsetYaw) << std::endl;

        glBindVertexArray(hudVAO);

        // Draw X axis arrow, red
        hudShader->set_uniform("objectColour", 0.8f, 0.0f, 0.2f);
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(90.0f), rotateToFaceX);
        hudShader->set_uniform_matrix4("model", 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, arrow_v4_elements_count, GL_UNSIGNED_INT, 0);

        // Draw Y axis arrow, blue
        hudShader->set_uniform("objectColour", 0.0f, 0.3f, 0.8f);
        model = glm::mat4(1.0f);
        hudShader->set_uniform_matrix4("model", 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, arrow_v4_elements_count, GL_UNSIGNED_INT, 0);

        // Draw Z axis arrow, green
        hudShader->set_uniform("objectColour", 0.2f, 0.8f, 0.0f);
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-90.0f), rotateToFaceZ);
        hudShader->set_uniform_matrix4("model", 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, arrow_v4_elements_count, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }

    void mouse_callback(GLFWwindow* window, double xpos, double ypos)
    {
        static double lastX = xpos;
        static double lastY = ypos;
        static float sensitivityY = 0.1f;
        static float sensitivityX = sensitivityY / windowRatio;
        
        yaw += (xpos - lastX) * sensitivityX;
        pitch += (lastY - ypos) * sensitivityY;
        
        pitch = glm::clamp(pitch, -89.9f, 89.9f);

        // std::cout << "Yaw: " << yaw << ", Pitch: " << pitch << std::endl;

        frontVec = glm::vec3(
                             cos(glm::radians(yaw)),
                             sin(glm::radians(pitch)),
                             sin(glm::radians(yaw))
                            );

        // std::cout << "frontVec    " << glm::to_string(frontVec) << std::endl;

        lastX = xpos;
        lastY = ypos;
    }

    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        zoomLevel -= (float)yoffset;
        if (zoomLevel < 1.0f)
        {
            zoomLevel = 1.0f;
        }
        else if (zoomLevel > 45.0f)
        {
            zoomLevel = 45.0f;
        }
        projectionMatrix = glm::perspective(glm::radians(zoomLevel), windowRatio, 0.1f, 100.0f);
    }

    glm::mat4 get_view_matrix()
    {
        return glm::lookAt(pos, pos + frontVec, upVec);
    }

    void toggle_fps_movement(bool enabled)
    {
        fpsMovement = enabled;
    }

    void W(float deltaTime)
    {
        glm::vec3 moveVec;
        if (!fpsMovement)
        {
            moveVec = glm::normalize(frontVec);
        }
        else
        {
            moveVec = glm::normalize(glm::vec3(frontVec.x, 0, frontVec.z));
        }
        // std::cout << "frontVec   " << glm::to_string(moveVec) << std::endl;
        pos += speed * deltaTime * moveVec;
        // std::cout << "pos   " << glm::to_string(pos) << std::endl;
    }

    void A(float deltaTime)
    {
        pos -= glm::normalize(glm::cross(frontVec, upVec)) * speed * deltaTime;
    }

    void S(float deltaTime)
    {
        glm::vec3 moveVec;
        if (!fpsMovement)
        {
            moveVec = glm::normalize(frontVec);
        }
        else
        {
            moveVec = glm::normalize(glm::vec3(frontVec.x, 0, frontVec.z));
        }
        pos -= speed * deltaTime * moveVec;
    }

    void D(float deltaTime)
    {
        pos += glm::normalize(glm::cross(frontVec, upVec)) * speed * deltaTime;
    }
};