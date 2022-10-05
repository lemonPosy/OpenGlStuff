#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>


void processInput(GLFWwindow* window);

//must state version number for vertex shader. 
const char* vertexShaderSource = "#version 450 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"void main()\n"
	"{\n"
	"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
	"}\0";

const char* fragmentShaderSource = "#version 450 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n" //frag color currently set to opaque orange
"}\n\0";

int main(void)
{
	GLFWwindow* window;

	/* Initialize the GLFW library */
	if (!glfwInit())
		return -1;

	// before creating the glfw window, set the glfw version to 4.5 core.
	//the key hint are GLFW_OPENGL_CORE_PROFILE , GLFW_CONTEXT_VERSION_MAJOR and GLFW_CONTEXT_VERSION_MINOR.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // configure GLFW. major 4
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5); // configure GLFW. minor 5 -> 4.5
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // configure GLFW. core Note: Telling GLFW we want to use the core-profile means we'll get access to a smaller subset of OpenGL features without backwards-compatible features we no longer need.

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(800, 600, "Hello World", NULL, NULL); //create a window object. This window object holds all the windowing data and is required by most of GLFW's other functions.
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	//glad: We pass GLAD the function to load the address of the OpenGL function pointers which is OS-specific. GLFW gives us glfwGetProcAddress that defines the correct function based on which OS we're compiling for
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))  //we want to initialize GLAD before we call any OpenGL function
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, 800, 600); // We have to tell OpenGL the size of the rendering window so OpenGL knows how we want to display the data and coordinates with respect to the window.

	//note: the minimum shaders needed by user is a vertex and fragment shader. below :)

	//VERTEX BUFFER OBJECT
	float vertices[] = {
	-0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 0.0f,  0.5f, 0.0f
	};

	unsigned int VBO; //vertex buffer object where the data for the geometry is stored.
	glGenBuffers(1, &VBO); //generate a single buffer object since we passed it 1. if we were passing it multiple vertex buffer object (say an array of 5), we would have to pass it 5 and we'd generate 5 buffer objects.
	glBindBuffer(GL_ARRAY_BUFFER, VBO); //opengl has many buffer types. in this case, a vertex buffer object is a GL_ARRAY_BUFFER. 
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //copy vertices information into the vertex buffer object.

	//shaders note: -> shaders are little programs that rest on the GPU. These programs are run for each specific section of the graphics pipeline. 
	// In a basic sense, shaders are nothing more than programs transforming inputs to outputs. 
	//Shaders are also very isolated programs in that they're not allowed to communicate with each other; the only communication they have is via their inputs and outputs.

	//VERTEX SHADER
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER); //type of shader we want to create is a vertex shader.

	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); //bind the vertex shader source to the vertex shader
	glCompileShader(vertexShader); //compile shader

	//check to see if the shader was compiled correctly. error loggigng
	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	//FRAGMENT SHADER -> The fragment shader is all about calculating the color output of your pixels
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);//error checking
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	//SHADER PROGRAM - > A shader program object is the final linked version of multiple shaders combined. he activated shader program's shaders will be used when we issue render calls
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glUseProgram(shaderProgram); //Every shader and rendering call after glUseProgram will now use this program object (and thus the shaders).
	glDeleteShader(vertexShader); //don't forget to delete the shader objects once we've linked them into the program object; we no longer need them anymore
	glDeleteShader(fragmentShader);

	//TILL NOW -> Right now we sent the input vertex data to the GPU and instructed the GPU how it should process the vertex data within a vertex and fragment shader
	//NEXT STEP -> OpenGL does not yet know how it should interpret the vertex data in memory and how it should connect the vertex data to the vertex shader's attributes.

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

 
	//Vertex Array Object -> VAO that stores our vertex attribute configuration and which VBO to use. 
	// Usually when you have multiple objects you want to draw, you first generate/configure all the VAOs (and thus the required VBO and attribute pointers) and store those for later use. 
	//The moment we want to draw one of our objects, we take the corresponding VAO, bind it, then draw the object and unbind the VAO again.
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);

	// 1. bind Vertex Array Object
	glBindVertexArray(VAO);
	// 2. copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// 3. then set our vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{

		processInput(window);

		/* Render here */
		glClearColor(255.0f, 255.0f, 0.0f, 1.0f); //sets the color to be set when using glClear (for the whole buffer)
		glClear(GL_COLOR_BUFFER_BIT);

		//draw triangle
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		/* Swap front and back buffers */
		glfwSwapBuffers(window);
		/* Poll for and process events */
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);


	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}
