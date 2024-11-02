#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // for perspective, lookAt
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>      // for to_string

#include "ShaderHelper.h"

namespace Camera {
    extern glm::vec3 pos;
    extern glm::mat4 projectionMatrix;

    extern void set_window_ratio(float width, float height);
    extern void draw_hud();
    extern void setup_hud();
    extern void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    extern void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    extern glm::mat4 get_view_matrix();
    extern void toggle_fps_movement(bool enabled);
    extern void W(float deltaTime);
    extern void A(float deltaTime);
    extern void S(float deltaTime);
    extern void D(float deltaTime);
};

#endif // CAMERA_H