#include "input.h"
#include "platform.h"
#include "TouchScreen.h"
#include "FileLoader.h"
#include "stb_image.h"
#include <GLFW/glfw3.h>

extern GLFWwindow* myMainWindow;

void cursorPositionCallback(GLFWwindow* window, double xPos, double yPos) {
    TouchScreen::addEvent(MOVE_CURSOR, xPos, yPos);
    //mylog("mouse pos: %d x %d\n",(int)xPos,(int)yPos);
}

void cursorEnterCallback(GLFWwindow* window, int entered) {
    if (entered)
        TouchScreen::addEvent(SCREEN_IN); //mylog("Entered window\n");
    else
        TouchScreen::addEvent(SCREEN_OUT); //mylog("Left window\n");
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int modS) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        TouchScreen::addEvent(LEFT_BUTTON_DOWN); //mylog("Left button pressed\n");
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        TouchScreen::addEvent(LEFT_BUTTON_UP); // mylog("Left button released\n");
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        TouchScreen::addEvent(RIGHT_BUTTON_DOWN); //mylog("Right button pressed\n");
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        TouchScreen::addEvent(RIGHT_BUTTON_UP); //mylog("Right button released\n");
}

void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    TouchScreen::addEvent(SCROLL, xOffset, yOffset); //mylog("scroll: %d x %d\n",(int)xOffset,(int)yOffset);
}
GLFWcursor* loadCursor(std::string path, int xhot, int yhot) {
    std::string filePath = FileLoader::getFullPath(path);
    GLFWimage img;
    int nrChannels;
    img.pixels = stbi_load(filePath.c_str(),
        &img.width, &img.height, &nrChannels, 4); //"4"-convert to 4 channels -RGBA
    GLFWcursor* pCursor = glfwCreateCursor(&img, xhot, yhot);
    stbi_image_free(img.pixels);
    return pCursor;
}
GLFWcursor* cursor_hand = NULL;
GLFWcursor* cursor_palm = NULL;
GLFWcursor* cursor_fist = NULL;
GLFWcursor* cursor_finger = NULL;
GLFWcursor* cursor_fingerdown = NULL;
GLFWcursor* cursor_arrow = NULL;
std::string currentCursorName="";

void setCursor(std::string cursorName) {
    if (currentCursorName.compare(cursorName) == 0)
        return;
    currentCursorName.assign(cursorName);

    //return glfwSetCursor(myMainWindow, cursor_fingerdown);


    if (cursorName.compare("arrow")==0)
        return glfwSetCursor(myMainWindow, cursor_arrow);
    if (cursorName.compare("hand") == 0)
        return glfwSetCursor(myMainWindow, cursor_hand);
    if (cursorName.compare("palm") == 0)
        return glfwSetCursor(myMainWindow, cursor_palm);
    if (cursorName.compare("fist") == 0)
        return glfwSetCursor(myMainWindow, cursor_fist);
    if (cursorName.compare("finger") == 0)
        return glfwSetCursor(myMainWindow, cursor_finger);
    if (cursorName.compare("fingerdown") == 0)
        return glfwSetCursor(myMainWindow, cursor_fingerdown);

    return glfwSetCursor(myMainWindow, cursor_arrow);
}

void initMouseInput() {
    GLFWwindow* window = myMainWindow;
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetCursorEnterCallback(window, cursorEnterCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);    // DISABLED, HIDDEN, NORMAL
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1);
    //load cursors
    cursor_finger = loadCursor("/dt/img/cursor/finger24.png",9,3);
    cursor_fingerdown = loadCursor("/dt/img/cursor/fingerdown24.png", 9, 5);
    cursor_palm = loadCursor("/dt/img/cursor/palm24.png", 14, 12);
    cursor_hand = loadCursor("/dt/img/cursor/hand24.png", 14, 12);
    cursor_fist = loadCursor("/dt/img/cursor/fist24.png", 14, 14);
    cursor_arrow = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    setCursor("arrow");
}

