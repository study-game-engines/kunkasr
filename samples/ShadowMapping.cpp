//
// Created by huangkun on 2018/4/13.
//

#include "ShadowMapping.h"

TEST_NODE_IMP_BEGIN

    unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    ShadowMapping::ShadowMapping() {
        const char *vert = R"(
#version 330 core
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texCoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

void main()
{
    gl_Position = projection * view * model * vec4(a_position, 1.0);
    vs_out.FragPos = vec3(model * vec4(a_position, 1.0));
    vs_out.Normal = mat3(transpose(inverse(model))) * a_normal;
    vs_out.TexCoords = a_texCoord;
}
)";

        const char *frag = R"(
#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

struct Material {
sampler2D diffuse;
};
uniform Material material;

struct Light {
vec3 position;
vec3 ambient;
vec3 diffuse;
vec3 specular;
};
uniform Light light;

uniform vec3 viewPos;
uniform sampler2D ourTexture;
uniform sampler2D depthMap;

void main()
{
    vec3 color = texture(material.diffuse, fs_in.TexCoords).rgb;
    vec3 ambient = light.ambient * color;

    vec3 norm = normalize(fs_in.Normal);
    vec3 lightDir = normalize(light.position - fs_in.FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * color;

    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 16.0);
    vec3 specular = light.specular * spec;

    float depthValue = texture(depthMap, fs_in.TexCoords).r;

    FragColor = vec4(diffuse + ambient + specular, 1.0);
}
)";

        const char *shadow_vert = R"(
#version 330 core
layout (location = 0) in vec3 a_position;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(a_position, 1.0);
}
)";

        const char *depth_frag = R"(
#version 330 core
out vec4 FragColor;

uniform int visual;
uniform sampler2D depthMap;


in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

float near = 0.1;
float far = 10.0; // use a closer dis, the box isn't too far away.
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    if (visual == 1)
    {
        //Visualizing the depth buffer
        float depth = LinearizeDepth(gl_FragCoord.z) / far;
    }
    else if (visual == 2)
    {
        float depth = texture(depthMap, fs_in.TexCoords).r;
//        depth = LinearizeDepth(depth) / far;    // perspective
    if (depth == 0)
{
        FragColor = vec4(vec3(1.0,0,0), 1.0);     // orthographic
}
else
{
        FragColor = vec4(vec3(depth), 1.0);     // orthographic
}
    }
    else
    {
        discard;
    }
}
)";
        const char *shadow_frag = R"(
#version 330 core

void main()
{
}
)";

        shader.loadStr(vert, frag);
        depthShader.loadStr(vert, depth_frag);
        shadowShader.loadStr(shadow_vert, shadow_frag);

        float vertices[] = {
                // positions          // normals           // texture coords
                -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
                0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
                0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
                0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
                -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
                -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

                -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
                0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
                0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
                -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

                -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
                -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

                0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
                0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

                -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
                0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
                0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
                0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
                -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

                -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
                0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
                -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
        };

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        // Plane
        float vertices2[] = {
                // postions       // texture coords
//                0.5f, 0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top right
//                0.5f, -0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom right
//                -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom left
//                -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // bottom top
                0.5f * 10, 0.5f * 10, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f * 10, 1.0f * 10, // top right
                0.5f * 10, -0.5f * 10, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f * 10, 0.0f, // bottom right
                -0.5f * 10, -0.5f * 10, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
                -0.5f * 10, 0.5f * 10, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f * 10, // bottom top
        };
        unsigned int indices[] = { // note that we start from 0!
                0, 1, 3, // first triangle
                1, 2, 3 // second triangle
        };
        glGenVertexArrays(1, &planeVAO);
        glBindVertexArray(planeVAO);

        glGenBuffers(1, &planeVBO);
        glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

        glGenBuffers(1, &planeEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        // depth map fbo
        glGenFramebuffers(1, &depthMapFBO);
        unsigned int depthMap;
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                     NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // visual depth quad
        float quadVertices[] = {
                // positions       // normals(uselsss) // texture Coords
                -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        // Images
        texture = loadTexture("../res/container.jpg");
        texture2 = loadTexture("../res/wood.png");

        shader.use();
        shader.setMat4("projection", projection);
        shader.setInt("material.diffuse", 0);
        shader.setInt("ourTexture", 0);
        shader.setVec3("light.ambient", vec3(0.2f, 0.2f, 0.2f));
        shader.setVec3("light.diffuse", vec3(1.0f, 1.0f, 1.0f));
        shader.setVec3("light.specular", vec3(0.5f, 0.5f, 0.5f));
        shader.setVec3("light.position", vec3(-1.5, 4, -2));

        cameraPos = vec3(0.3f, 4.0f, 5.0f);
        cameraDir = vec3(0.0f, 2.0f, 0.0f) - cameraPos;

        view = glm::lookAt(cameraPos, cameraPos + cameraDir, cameraUp);
        glm::quat quat = glm::quat_cast(view);
        vec3 angles = eulerAngles(quat);
        pitch = -degrees(angles.x);

        glEnable(GL_DEPTH_TEST);
    }

    void ShadowMapping::draw(const mat4 &transform) {
/////////////////////////////// origin scene without shadow //////////////////////
        // render scene
//        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        shader.use();
//        shader.setMat4("projection", projection);
//        view = glm::lookAt(cameraPos, cameraPos + cameraDir, cameraUp);
//        shader.setMat4("view", view);
//        renderScene(shader);
///////////////////////////// depth //////////////////////
        // light matrix
//        glClear(GL_DEPTH_BUFFER_BIT);
//        auto &size = Director::getInstance()->getWinSize();
//        mat4 pj = glm::perspective(glm::radians(60.0f), size.width / size.height, 0.1f, 100.0f);
////        mat4 pj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 10.0f);
//        mat4 vw = glm::lookAt(vec3(-1.5, 4, -2), vec3(0, 0, 0), vec3(0, 1, 0));
//        depthShader.use();
//        depthShader.setMat4("projection", pj);
//        depthShader.setMat4("view", vw);
//        depthShader.setInt("visual", 1);
//        renderScene(depthShader);

        auto &size = Director::getInstance()->getFrameBufferSize();
//        SHADOW_WIDTH = (unsigned int)size.width;
//        SHADOW_HEIGHT = (unsigned int)size.height;
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        // light matrix
        mat4 pj = glm::perspective(glm::radians(60.0f), size.width / size.height, 0.1f, 100.0f);
        mat4 vw = glm::lookAt(vec3(-1.5, 4, -2), vec3(0, 0, 0), vec3(0, 1, 0));
        depthShader.use();
        depthShader.setMat4("projection", pj);
        depthShader.setMat4("view", vw);
        depthShader.setInt("visual", 0);
        renderScene(depthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // visual depth
        glViewport(0, 0, size.width, size.height);
//        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMapFBO);
        depthShader.setInt("visual", 2);
        depthShader.setInt("depthMap", 0);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

/////////////////////////////// scene with shadow //////////////////////
        // render to depth map
//        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
//        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
//        glClear(GL_DEPTH_BUFFER_BIT);
//
//        // light matrix
//        auto &size = Director::getInstance()->getWinSize();
//        mat4 pj = glm::perspective(glm::radians(60.0f), size.width / size.height, 0.1f, 100.0f);
//        mat4 vw = glm::lookAt(vec3(-1.5, 4, -2), vec3(0, 0, 0), vec3(0, 1, 0));
//        depthShader.use();
//        depthShader.setMat4("projection", pj);
//        depthShader.setMat4("view", vw);
//        renderScene(depthShader);
//
//        glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//
//        shader.use();
//        shader.setMat4("projection", projection);
//        view = glm::lookAt(cameraPos, cameraPos + cameraDir, cameraUp);
//        shader.setMat4("view", view);
//        glActiveTexture(GL_TEXTURE1);
//        glBindTexture(GL_TEXTURE_2D, depthMapFBO);
//        shader.setInt("depthMap", 0);
//        renderScene(shader);
    }

    void ShadowMapping::renderScene(Shader &shader) {
        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        model = glm::mat4();
        model = glm::translate(model, glm::vec3(-1.0f, 2.0f, -1.0f));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::mat4();
        model = glm::translate(model, glm::vec3(1.0f, 0.5f, -1.0f));
        model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::mat4();
        model = glm::translate(model, glm::vec3(-1.5f, 0.5f, 0.0f));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // plane
        model = glm::mat4();
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        shader.setMat4("model", model);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture2);
        glBindVertexArray(planeVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    ShadowMapping::~ShadowMapping() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &planeVAO);
        glDeleteBuffers(1, &planeVBO);
        glDeleteBuffers(1, &planeEBO);
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
    }
TEST_NODE_IMP_END