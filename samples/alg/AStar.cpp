//
// Created by huangkun on 2018/4/24.
//

#include "AStar.h"

using namespace std;
TEST_NODE_IMP_BEGIN

    AStar::AStar() {
        const char *vert = R"(
#version 330 core
layout (location = 0) in vec2 a_position;
out vec2 pos;

void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);
    pos = vec2(a_position);
}
)";

        const char *frag = R"(
#version 330 core
out vec4 FragColor;
in vec2 pos;

layout (std140) uniform Map
{
    vec4 status[1200];
    vec4 times[1200];
//    int status[1200];
//    float times[1200];
};

void main()
{
    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
    int width = 40;
    int height = 30;
    int x = int((pos.x/2 + 0.5) * width);
    int y = int((pos.y/2 + 0.5) * height);
    int index = x + y * width;

    int gap = 20;
    int x1 = int((pos.x/2 + 0.5) * width * gap);
    int y1 = int((pos.y/2 + 0.5) * height * gap);
    if (x1 % gap == 0 || y1 % gap == 0)
    {
        // grid line
        color = vec4(0.5, 0.5, 0.5, 0.5);
    }
    else
    {
        if (times[index].r == -1)
        {
            // from
            color = vec4(1, 0.5, 0.5, 1.0);
        }
        else if (times[index].r == -2)
        {
            // to
            color = vec4(0.5, 1.0, 0.5, 1.0);
        }
        else if (times[index].r == 1)
        {
            // route path
            color = vec4(0.5, 0.5, 0.5, 1.0);
        }
        else if (times[index].r > 0)
        {
            // route history
            color = vec4(0, times[index].r, 0, 1.0);
        }
        else if (status[index].r == 0)
        {
            // empty grid
            color = vec4(0.86, 0.86, 0.86, 1.0);
        }
        else
        {
            color.b = times[index].r;
            color.g = color.b;
            color.r = color.b;
        }
    }

    FragColor = color;
}
)";
        shader.loadStr(vert, frag);

        float vertices[] = {
                1.0f, 1.0f,  // top right
                1.0f, -1.0f, // bottom right
                -1.0f, -1.0f,  // bottom left
                -1.0f, 1.0f,   // bottom top
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

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(0);

        width = 40;
        height = 30;

        shader.use();
        unsigned int bindingPoint = 1;
        unsigned int index = glGetUniformBlockIndex(shader.ID, "Map");
        glUniformBlockBinding(shader.ID, index, bindingPoint);
        int blockSize = 0;
        glGetActiveUniformBlockiv(shader.ID, index, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
        log("blockSize = %d", blockSize);

        glGenBuffers(1, &UBO);
        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        auto mapSize = 16 * width * height;
        auto visitTimesSize = 16 * width * height;
        log("mapSize = %d", mapSize);
        log("visitTimesSize = %d", visitTimesSize);
        glBufferData(GL_UNIFORM_BUFFER, mapSize + visitTimesSize, NULL, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, UBO);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // gen map
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                map.push_back(0);
            }
        }

        from = vec2(10, 10);
        to = vec2(35, 20);

        resetVisitTimes();
        doAStar();
        drawMap();
    }

    void AStar::resetVisitTimes() {
        visitTimes.clear();
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                visitTimes.push_back(0);
            }
        }
    }

    void AStar::doAStar() {
        std::vector<int> path;
        unordered_set<int> open_set;
        unordered_set<int> close_set;
        unordered_map<int, int> came_from;
//        unordered_map<int, int> g_score;
        unordered_map<int, float> h_score;
        unordered_map<int, float> f_score;
        auto manhattanDis = [](vec2 &from, vec2 &to) -> float {
            return abs(from.x - to.x) + abs(from.y - to.y);
        };
        auto indexOf = [&](vec2 &p) -> int {
            return int((p.x - 1) + (p.y - 1) * width);
        };

        open_set.emplace(indexOf(from));
        float dis = manhattanDis(from, to);
        h_score.insert({indexOf(from), dis});
        f_score.insert({indexOf(from), dis});
        while (!open_set.empty()) {
            float dis = INT_MAX;
            vec2 p;
            for (auto iter = f_score.begin(); iter != f_score.end(); iter++) {
                if (iter->second < dis) {
                    dis = iter->second;
                    p.y = iter->first / width + 1;
                    p.x = iter->first % width + 1;
                }
            }
            if (indexOf(p) == indexOf(to)) {
                reconstruct_path(path, came_from, indexOf(to));
                break;
            }
            open_set.erase(indexOf(p));
            close_set.insert(indexOf(p));
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    vec2 y = vec2(p.x + i, p.y + j);
                    auto index = indexOf(y);
                    if (index < 0 || index >= width * height) {
                        continue;
                    }
                    if (close_set.find(index) != close_set.end()) {
                        continue;
                    }
                    visitTimes[index] += 1;
                    bool isBetter = false;
                    if (open_set.find(index) == open_set.end()) {
                        open_set.insert(index);
                        isBetter = true;
                    } else {
                        isBetter = false;
                    }
                    if (isBetter) {
                        came_from.insert({indexOf(y), indexOf(p)});
                        float dis = manhattanDis(y, to);
                        h_score.insert({index, dis});
                        f_score.insert({index, dis});
                    }
                }
            }
        }

        // route path
        for (auto iter = path.rbegin() + 1; iter != path.rend() - 1; iter++) {
            int y = *iter / width + 1;
            int x = *iter % width + 1;
            log("%d,%d", x, y);
            visitTimes[*iter] = 1;
        }
        // from
        visitTimes[(from.x - 1) + (from.y - 1) * width] = -1;
        // to
        visitTimes[(to.x - 1) + (to.y - 1) * width] = -2;
    }

    void AStar::reconstruct_path(std::vector<int> &path, std::unordered_map<int, int> &came_from, int current) {
        path.push_back(current);
        if (came_from.find(current) != came_from.end()) {
            reconstruct_path(path, came_from, came_from.at(current));
        }
    }

    void AStar::drawMap() {
        // std140 require align memory
        float max = 1.0f;
        for (auto iter = visitTimes.begin(); iter != visitTimes.end(); iter++) {
            if (*iter > max) {
                max = *iter;
            }
        }
        log("max times = %.0f", max);
        std::vector<vec4> map2;
        std::vector<vec4> visitTimes2;
        for (int i = 0; i < width * height; i++) {
            map2.push_back(vec4(map[i], 0, 0, 0));
            // normalize
            if (visitTimes[i] > 0) {
                visitTimes2.push_back(vec4(visitTimes[i] / max / 2, 0, 0, 0));
            } else {
                visitTimes2.push_back(vec4(visitTimes[i], 0, 0, 0));
            }
        }
        auto mapSize = 16 * width * height;
        auto visitTimesSize = 16 * width * height;

        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        // define the range of the buffer that links to a uniform binding point
//        glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, mapSize + visitTimesSize);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, mapSize, &map2[0]);
        glBufferSubData(GL_UNIFORM_BUFFER, mapSize, visitTimesSize, &visitTimes2[0]);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        CHECK_GL_ERROR_DEBUG();
    }

    void AStar::draw(const mat4 &transform) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(VAO);
        shader.use();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    AStar::~AStar() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

TEST_NODE_IMP_END