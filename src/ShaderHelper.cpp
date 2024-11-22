#include "ShaderHelper.h"

ShaderHelper::ShaderHelper()
{
    m_shaderProgram = glCreateProgram();
    m_needsLinking = false;
}

ShaderHelper::~ShaderHelper()
{
    for (std::vector<unsigned int>::iterator s = m_shaders.begin(); s != m_shaders.end(); ++s)
    {
        glDeleteShader(*s);
    }
    glDeleteProgram(m_shaderProgram);
}

/* Compiles a shader and adds it to the helper object. */
bool ShaderHelper::add_shader(GLenum type, const char **source)
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
        printf("SHADER CONTENTS:\n");
        printf("%s\n\n", *source);
        assert(false);
        return false;
    }

    m_shaders.push_back(shader);
    glAttachShader(m_shaderProgram, shader);
    m_needsLinking = true;
    return true;
}

/* Links the known shaders to the program */
bool ShaderHelper::link_shaders()
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
    m_needsLinking = false;
    return true;
}

void ShaderHelper::use()
{
    if (m_needsLinking)
    {
        std::cout << "ERROR! Newly added shader(s) were not linked yet!" << std::endl;
        assert(false);
    }
    glUseProgram(m_shaderProgram);
}

int ShaderHelper::get_uniform_location(const char *name)
{
    return glGetUniformLocation(m_shaderProgram, name);
}

void ShaderHelper::set_uniform(const char *name, GLint i)
{
    use();
    glUniform1i(get_uniform_location(name), i);
}

void ShaderHelper::set_uniform(const char *name, GLfloat f)
{
    use();
    glUniform1f(get_uniform_location(name), f);
}

void ShaderHelper::set_uniform(const char *name, GLfloat f1, GLfloat f2, GLfloat f3)
{
    use();
    glUniform3f(get_uniform_location(name), f1, f2, f3);
}

void ShaderHelper::set_uniform(const char *name, glm::vec3 vec3)
{
    set_uniform(name, vec3.x, vec3.y, vec3.z);
}

void ShaderHelper::set_uniform_matrix4(const char *name, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    use();
    glUniformMatrix4fv(get_uniform_location(name), count, transpose, value);
}

unsigned int ShaderHelper::load_texture(const char *filename, bool transparent)
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