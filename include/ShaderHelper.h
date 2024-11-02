#ifndef SHADER_HELPER_H
#define SHADER_HELPER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "stb_image.h"

#include <iostream>
#include <vector>
#include <cassert>

class ShaderHelper {
    private:
        unsigned int m_shaderProgram;
        std::vector<unsigned int> m_shaders;
        bool m_needsLinking;

    public:
        ShaderHelper();
        ~ShaderHelper();

        bool add_shader(GLenum type, const char **source);
        bool link_shaders();
        void use();

        int get_uniform_location(const char *name);
        void set_uniform(const char *name, GLint i);
        void set_uniform(const char *name, GLfloat f);
        void set_uniform(const char *name, GLfloat f1, GLfloat f2, GLfloat f3);
        void set_uniform(const char *name, glm::vec3 vec3);
        void set_uniform_matrix4(const char *name, GLsizei count, GLboolean transpose, const GLfloat *value);
        
        unsigned int load_texture(const char *filename, bool transparent);
};

#endif // SHADER_HELPER_H