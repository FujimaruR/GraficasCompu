#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos, 1.0);\n"
"}\0";
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(0.262f, 0.615f, 0.305f, 1.0f);\n"
"}\n\0";
const char* vertexShaderSourceWithColor =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"

"out vec3 ourColor;"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos, 1.0);\n"
"   ourColor = aColor;"
"}\0";
const char* fragmentShaderSourceWithColor = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 ourColor;"
"void main()\n"
"{\n"
"   FragColor = vec4(ourColor.x, ourColor.y, ourColor.z, 1.0f);\n"
"}\n\0";


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Tangram", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSourceWithColor, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSourceWithColor, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
         0.25f,  0.5f, 0.0f,  // top right
         0.25f, -0.5f, 0.0f,  // bottom right
        -0.25f, -0.5f, 0.0f,  // bottom left


        -0.25f,  0.5f, 0.0f,   // top left 
        -0.7f,  -0.5f, 0.0f,  // top right
        -0.7f,   0.5f, 0.0f,  // bottom right
    };

    float verticesTriangulosColores[] = {
        -0.5f,0.5f,0.0f,    0.976, 1, 0.101,  // 0
        0.0f, 0.5f, 0.0f,   0.976, 1, 0.101,  // 1
        -0.5f, -0.0f, 0.0f, 0.976, 1, 0.101,  // 2 

        0.0f, 0.5f, 0.0f,   0.290, 0.643, 0.254,  // 3
        0.0f, 0.0f, 0.0f,   0.290, 0.643, 0.254,  // 4
        -0.5f,  0.0f, 0.0f, 0.290, 0.643, 0.254,   // 5

        0.25f,  0.5f, 0.0f, 1, 0, 0,        // 6
        0.0f,  0.5f, 0.0f,  1, 0, 0,        // 7
        0.25f,  0.0f, 0.0f, 1, 0, 0,        // 8

        0.0f,  0.0f, 0.0f,  0.474, 0.207, 0.207,
        0.0f,  0.5f, 0.0f,  0.474, 0.207, 0.207,
        0.25f, 0.0f, 0.0f,  0.474, 0.207, 0.207,

    };

    float VerticesTrangram[] = {
        //Triangulo de la izquierda
        //0.5f, 0.5f, 0.0f,    0.890, 0.509, 0.670, //0
        //0.0f, 0.0f, 0.0f,    0.886, 0.196, 0.674, //1
        //-0.5f, -0.5f, 0.0f,    0.886, 0.196, 0.674, //0.890, 0.509, 0.670 //2

        //Triangulo de abajo
        -0.5f, -0.5f, 0.0f,    0.886, 0.490, 0.196, //3
        0.0f, 0.0f, 0.0f,    0.886, 0.490, 0.196, //4
        0.5f, -0.5f, 0.0f,    0.886, 0.490, 0.196, //5

        //Triangulo de arriba de la derecha
        0.5f, 0.5f, 0.0f,    1, 0.890, 0.219, //6
        0.5f, 0.0f, 0.0f,    1, 0.890, 0.219, //7
        0.0f, 0.5f, 0.0f,    1, 0.890, 0.219, //8

        //arriba en medio 
        0.25f, 0.25f, 0.0f,    0.235, 0, 0.341, //9
        0.0f, 0.0f, 0.0f,    0.235, 0, 0.341, //10
        -0.25f, 0.25f, 0.0f,    0.235, 0, 0.341, //11

        //arriba en medio segundo
        0.25f, 0.25f, 0.0f,    0.980, 0, 0.094, //12
        -0.25f, 0.25f, 0.0f,    0.980, 0, 0.094, //13
        0.0f, 0.5f, 0.0f,    0.980, 0, 0.094, //14

        //arriba
        -0.5f, 0.5f, 0.0f,    0.980, 0, 0.094, //15
        -0.25f, 0.25f, 0.0f,    0.980, 0, 0.094, //16
        0.0f, 0.5f, 0.0f,    0.980, 0, 0.094, //17

        //cuadrado1
         0.25f, 0.25f, 0.0f,    0, 0.062, 0.541, //18
        0.5f, 0.0f, 0.0f,    0, 0.062, 0.541, //19
        0.0f, 0.0f, 0.0f,    0, 0.062, 0.541, //20

        //cuadrado2
        0.25f, -0.25f, 0.0f,    0, 0.062, 0.541, //21
        0.5f, 0.0f, 0.0f,    0, 0.062, 0.541, //22
        0.0f, 0.0f, 0.0f,    0, 0.062, 0.541, //23

        //abajo a la derecha
        0.5f, -0.5f, 0.0f,    0, 0.541, 0.023, //24
        0.25f, -0.25f, 0.0f,    0, 0.541, 0.023, //25
        0.5f, 0.0f, 0.0f,    0, 0.541, 0.023, //26

         -0.5f, 0.5f, 0.0f,    0.796, 0.396, 0.886, //0
        0.0f, 0.0f, 0.0f,    0.796, 0.396, 0.886, //1
        -0.5f, -0.5f, 0.0f,    0.796, 0.396, 0.886, //2
    };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        3, 2, 1,   // second Triangle

        3, 2, 5,   // third Triangle
        5, 4, 2   // fourth Triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);


    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VerticesTrangram), VerticesTrangram, GL_STATIC_DRAW);

    /*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);*/

    //2 ATRIBUTOS

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); //Column Major
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //UN ATRIBUTO

    /*glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);*/

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);


    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(shaderProgram);
        glLineWidth(20.0);

        glPointSize(20.0);

        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        glDrawArrays(GL_TRIANGLES, 0, 27); //GL_LINE_LOOP
        /*glDrawElements(GL_TRIANGLES, 15, GL_UNSIGNED_INT, 0);*/ //Solo se puede esta o arrays a la vez
        // glBindVertexArray(0); // no need to unbind it every time 

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}