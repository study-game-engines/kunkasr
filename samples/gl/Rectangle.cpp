//
// Created by Administrator on 2018/4/1 0001.
//

#include "Rectangle.h"

TEST_NODE_IMP_BEGIN

    Rectangle::Rectangle() {
        const char *vert = R"(
#version 330 core
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;
out vec4 outColor;

void main()
{
    gl_Position = vec4(a_position, 1.0);
    outColor = vec4(a_color);
}
)";

        const char *frag = R"(
#version 330 core
in vec4 outColor;
out vec4 FragColor;

void main()
{
    FragColor = outColor;
}
)";
        shader.loadStr(vert, frag);

        float vertices[] = {
                0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // top right
                0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // bottom right
                -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // bottom left
                -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,  // bottom top
        };
        unsigned int indices[] = { // note that we start from 0!
                0, 1, 3, // first triangle
                1, 2, 3 // second triangle
        };

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *) (3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // unbind
        glBindVertexArray(0);
    }

    void Rectangle::draw(const mat4 &transform) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        glBindVertexArray(VAO);
//        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframemode
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    Rectangle::~Rectangle() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
TEST_NODE_IMP_END