#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <stack>


int width = 1000;
int height = 1000;
int pixelSize = 1;
float pixelPerUnit = 2 / (float)height;

int* borderMap;

glm::ivec3 lineColor(0,0,0);

void setBorder(int x, int y)
{
    borderMap[(height / 2 - y) * width + (x + width / 2)] = 1;
}
void setFill(int x, int y)
{
    borderMap[(height / 2 - y) * width + (x + width / 2)] = 2;
}
int getBorderMap(int x, int y)
{
    return borderMap[(height / 2 - y) * width + (x + width / 2)];
}

void midPointLine(glm::ivec2 p1, glm::ivec2 p2)
{
    glColor4ub(lineColor.x, lineColor.y, lineColor.z, 255);
    glBegin(GL_POINTS);
    
    int dx = abs(p2.x - p1.x);
    int dy = abs(p2.y - p1.y);
    int sx = p1.x < p2.x ? 1 : -1;
    int sy = p1.y < p2.y ? 1 : -1;
    int p;
    int x = p1.x, y = p1.y;

    if (dx > dy) {
        p = 2 * -dy + dx;
        while (x != p2.x + sx && y != p2.y + sy) {
            setBorder(x, y);
            glVertex2f((x + 0.5) * pixelSize * pixelPerUnit, (y + 0.5) * pixelSize * pixelPerUnit);
            x += sx;
            if (p <= 0) { p += 2 * (-dy + dx); y += sy; }
            else { p += 2 * -dy; }
        }
    }
    else {
        p = 2 * -dx + dy;
        while (x != p2.x + sx && y != p2.y + sy) {
            setBorder(x, y);
            glVertex2f((x + 0.5) * pixelSize * pixelPerUnit, (y + 0.5) * pixelSize * pixelPerUnit);
            y += sy;
            if (p <= 0) { p += 2 * (-dx + dy); x += sx; }
            else { p += 2 * -dx; }
        }
    }
    glEnd();
}

void DDALine(glm::ivec2 p1, glm::ivec2 p2)
{
    glColor4ub(lineColor.x, lineColor.y, lineColor.z, 255);
    glBegin(GL_POINTS);
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
    dx = dx / (float)steps;
    dy = dy / (float)steps;
    float x = p1.x, y = p1.y;

    setBorder(x, y);
    glVertex2f((x + 0.5) * pixelSize * pixelPerUnit, (y + 0.5) * pixelSize * pixelPerUnit);
    for (int i = 0; i < steps; i++) {
        x += dx;
        y += dy;

        setBorder(x, y);
        glVertex2f(((int)x + 0.5) * pixelSize * pixelPerUnit, ((int)y + 0.5) * pixelSize * pixelPerUnit);
    }
    glEnd();
}

void seedFill(int x, int y)
{
    static std::stack<glm::ivec2> stack;
    stack.push(glm::ivec2(x, y));
    while (!stack.empty()) {
        glm::ivec2 pixel = stack.top();
        stack.pop();

        if (getBorderMap(pixel.x, pixel.y) == 0) {
            setFill(pixel.x, pixel.y);
            glColor4ub(0, 0, 0, 1);
            glBegin(GL_POINTS);
            glVertex2f((pixel.x + 0.5) * pixelSize * pixelPerUnit, (pixel.y + 0.5) * pixelSize * pixelPerUnit);
            glEnd();

            if (getBorderMap(pixel.x, pixel.y + 1) == 0) stack.push(glm::ivec2(pixel.x, pixel.y + 1));
            if (getBorderMap(pixel.x, pixel.y - 1) == 0) stack.push(glm::ivec2(pixel.x, pixel.y - 1));
            if (getBorderMap(pixel.x - 1, pixel.y) == 0) stack.push(glm::ivec2(pixel.x - 1, pixel.y));
            if (getBorderMap(pixel.x + 1, pixel.y) == 0) stack.push(glm::ivec2(pixel.x + 1, pixel.y));
        }
    }
}

int fillLineRight(int x, int y)
{
    while (getBorderMap(x, y) == 0) {
        setFill(x, y);
        glColor4ub(0, 0, 0, 1);
        glBegin(GL_POINTS);
        glVertex2f((x + 0.5) * pixelSize * pixelPerUnit, (y + 0.5) * pixelSize * pixelPerUnit);
        glEnd();
        x++;
    }
    return x - 1;
}
int fillLineLeft(int x, int y)
{
    while (getBorderMap(x, y) == 0) {
        setFill(x, y);
        glColor4ub(0, 0, 0, 1);
        glBegin(GL_POINTS);
        glVertex2f((x + 0.5) * pixelSize * pixelPerUnit, (y + 0.5) * pixelSize * pixelPerUnit);
        glEnd();
        x--;
    }
    return x + 1;
}
void newLineSeed(std::stack<glm::ivec2>& stack, int xl, int xr, int y)
{
    int x = xl;
    while (x <= xr) {
        int f = 0;
        while (getBorderMap(x, y) == 0 && x < xr) {
            if (f == 0) f = 1;
            x++;
        }
        if (f == 1) {
            if (x == xr && getBorderMap(x, y) == 0)
                stack.push(glm::ivec2(x, y));
            else stack.push(glm::ivec2(x - 1, y));
            
            f = 0;
        }
        int c = x;
        while (getBorderMap(x, y) != 0 && x < xr) x++;
        if (x == c) x++;
    }
}
void scanLineFill(int sx, int sy)
{
    static std::stack<glm::ivec2> stack;
    stack.push(glm::ivec2(sx, sy));
    while (!stack.empty()) {
        glm::ivec2 pixel = stack.top();
        stack.pop();

        int r = fillLineRight(pixel.x, pixel.y);
        int l = fillLineLeft(pixel.x - 1, pixel.y);
        
        newLineSeed(stack, l, r, pixel.y - 1);
        newLineSeed(stack, l, r, pixel.y + 1);
    }
}

struct LineSet
{
    std::vector<glm::ivec2> points;
    std::vector<glm::ivec2> indices;
};
LineSet parse(const std::string file)
{
    std::ifstream infile(file);
    if (!infile.is_open()) return {};

    LineSet lines;
    std::string str;
    bool t = false;
    while (getline(infile, str)) {
        if (str == "/") {
            t = true; continue;
        }
        if (t == 0) {
            std::istringstream ss(str);
            int x, y;
            ss >> x >> y;
            x = x - width / 2;
            y = height / 2 - y;
            lines.points.push_back(glm::ivec2(x, y));
        }
        else {
            std::istringstream ss(str);
            int p1, p2;
            ss >> p1 >> p2;
            lines.indices.push_back(glm::ivec2(p1, p2));
        }
    }
    return lines;
}

int main()
{
    if (!glfwInit())
        return 0;

    GLFWwindow* window = glfwCreateWindow(width, height, "Hello World", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    if (glewInit() == GLEW_OK)
        std::cout << "glew init ok" << std::endl;    

    LineSet lines = parse("./icon1.txt");

    for (auto l : lines.indices) {
        std::cout << lines.points[l.x].x << "," << lines.points[l.x].y << "  " <<
            lines.points[l.y].x << "," << lines.points[l.y].y << std::endl;
    }

    borderMap = new int[width * height]();

    while (!glfwWindowShouldClose(window)) {
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        memset(borderMap, 0, sizeof(int) * width * height);
        glPointSize(pixelSize/2);
        for (auto l : lines.indices) {
            midPointLine(lines.points[l.x], lines.points[l.y]);
        }

        /*scanLineFill(340 - 500, 500 - 220);
        scanLineFill(560 - 500, 500 - 220);
        scanLineFill(400 - 500, 500 - 310);
        scanLineFill(650 - 500, 500 - 610);
        scanLineFill(440 - 500, 500 - 910);*/


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete[] borderMap;
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}