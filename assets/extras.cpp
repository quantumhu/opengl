    /*
    // green over time
    float timeValue;
    float greenValue;
    timeValue = glfwGetTime();
    greenValue = (sin(timeValue) / 2.0f) + 0.5f;
    
    glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
    */

       // wireframe mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

(float)abs((sin(glfwGetTime()) * 45.0))

        // second arg is count, third arg is whether to transpose
        // fourth arg is because glm and opengl have different data formats so we must transform
        glUniformMatrix4fv(sh.get_uniform_location("transform"), 1, GL_FALSE, glm::value_ptr(trans));
        glUniform1f(sh.get_uniform_location("mixU"), mix_percent);


    // unsigned int indices[] = {
    //     0, 1, 3,
    //     1, 2, 3,
    // };

        // unsigned int EBO;
    // glGenBuffers(1, &EBO);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // When you unbind EBO, make sure to unbind VAO first so you don't unbind EBO from the VAO



    // don't need to useProgram when getting uniform location, but we do need it when updating
    // int vertexColorLocation = sh.get_uniform_location("ourColor");


            // frontVec += (glm::vec3(
        //                       cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        //                       sin(glm::radians(pitch)),
        //                       sin(glm::radians(yaw)) * cos(glm::radians(pitch))
        //                      ));