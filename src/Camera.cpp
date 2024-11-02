#include "Camera.h"

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

    const char *hudVertexSource = "#version 330 core\n"
        "layout (location = 0) vec2 pos;\n"
        "void main()\n"
        "{\n"
        "  gl_Position = vec4(pos.x, 0.0, pos.y, 1.0);\n"
        "}";
    
    const char *hudFragmentSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "  FragColor = vec4(1.0);\n"
        "}";

    void set_window_ratio(float width, float height)
    {
        windowRatio = width / height;
        projectionMatrix = glm::perspective(glm::radians(zoomLevel), windowRatio, 0.1f, 100.0f);
    }

    void draw_hud()
    {

    }

    void setup_hud()
    {
        // if (hudShader == nullptr)
        // {
        //     hudShader = new ShaderHelper();
        //     hudShader->add_shader(GL_VERTEX_SHADER, &hudVertexSource);
        //     hudShader->add_shader(GL_FRAGMENT_SHADER, &hudFragmentSource);
        //     hudShader->link_shaders();
        // }
    }

    void mouse_callback(GLFWwindow* window, double xpos, double ypos)
    {
        static double lastX = xpos;
        static double lastY = ypos;
        static float sensitivityY = 0.1f;
        static float sensitivityX = sensitivityY / windowRatio;
        
        // 0 degree yaw is 1x, 0z
        // 90 degree is 0x, 1z
        static float yaw = 0.0f;
        static float pitch = 0.0f;
        static bool isYawCalced = false;
        if (!isYawCalced)
        {
            // needs to be unit vector in x or z direction
            bool x_conforms = fabs(frontVec.x) == 1.0f;
            bool z_conforms = fabs(frontVec.z) == 1.0f;
            assert(x_conforms ^ z_conforms);
            isYawCalced = true;

            if (x_conforms)
            {
                if (frontVec.x > 0.0f) yaw = 0.0f;
                else yaw = 180.0f;
            }
            else
            {
                if (frontVec.z > 0.0f) yaw = 90.0f;
                else yaw = 270.0f;
            }
        }
        
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
        // glm::quat q = glm::quatLookAt(glm::normalize(frontVec), upVec);
        // return glm::translate(glm::toMat4(q), -pos);
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