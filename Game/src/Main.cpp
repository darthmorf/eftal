
#include <cstdio>
#include <chrono> // TODO_THREADS, no std lib
#include <thread> // TODO_THREADS, no std lib

#include <GLFW/glfw3.h>


void glfwErr(int error, const char* description)
{
    // TODO_LOGGING
    fprintf(stderr, "[GLFW] Error %d: %s\n", error, description);
}


int main()
{
    glfwSetErrorCallback(glfwErr);

    if (!glfwInit())
        return -1; // TODO_ASSERT, Asserts when we have them

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // We're using DX12 so we want 'no api'.

    GLFWwindow* window = glfwCreateWindow(640, 480, "Eftal", NULL, NULL);
    if (!window)
       return -1; // TODO_ASSERT, Asserts when we have them

    fprintf(stdout, "[GLFW] Polling");
    while (!glfwWindowShouldClose(window))
    {
        fprintf(stdout, ".");
        glfwPollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
