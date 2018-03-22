#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define ASSERT(x) if(!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
						x;\
						ASSERT(GLLogCall(#x, __FILE__, __LINE__))

static void GLClearError()
{
	while (glGetError != GL_NO_ERROR);
}


static bool GLLogCall(const char* function, const char* file, int line)
{
	while (GLenum error = glGetError())
	{
		std::cout << "[OpenGL Error:]" << error << std::endl;
		std::cout << "In file: " << file << ", line no: " << line << std::endl;

		return false;
	}
	return true;
}

struct ShaderSourceProgram 
{
	std::string VertexShader;
	std::string FragmentShader;
};

static ShaderSourceProgram ParseShader(const std::string& filePath)
{
	ShaderSourceProgram shaderSource;
	shaderSource.VertexShader = "";
	shaderSource.FragmentShader = "";

	//if (!filePath.empty())
	{
		std::ifstream stream(filePath);

		enum class ShaderType
		{
			NONE = -1, VERTEX = 0, FRAGMENT = 1
		};

		std::string line;
		std::stringstream ss[2];

		ShaderType shaderType = ShaderType::NONE;

		while (std::getline(stream, line))
		{
			if (line.find("#shader") != std::string::npos)
			{
				if (line.find("vertex") != std::string::npos)
				{
					//set shader type to vertex
					shaderType = ShaderType::VERTEX;
				}
				else if(line.find("fragment") != std::string::npos)
				{
					//set shader type to fragment
					shaderType = ShaderType::FRAGMENT;
				}
			}
			else
			{
				//put the shader code to the respective stream
				ss[(int)shaderType] << line << std::endl;
			}
		}

		shaderSource.VertexShader = ss[0].str();
		shaderSource.FragmentShader = ss[1].str();

	}

	return shaderSource;
}

static unsigned int CompileShader(unsigned int type,const std::string& source)
{
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		int length = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char *message = (char*)alloca(sizeof(char)*length);
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << "Shader compilation Failed" << std::endl;
		std::cout << message << std::endl;
		result = 0;
	}

	return result;
}

static unsigned int CreateShader(const std::string &vertexShader, const std::string& fragmentShader)
{
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(program,vs);
	glAttachShader(program,fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
		std::cout << "ERROR GLEW" << std::endl;

	std::cout << glGetString(GL_VERSION) << std::endl;

	float positions[] = {
			-0.5f,-0.5f,	//0
			 0.5f, -0.5f,	//1
			 0.5f, 0.5f,	//2
			-0.5f, 0.5f     //3
	};

	unsigned int indices[] = {
			0, 1, 2,
			2, 3, 0
	};

	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW);

	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
	glEnableVertexAttribArray(0);
	 

	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	ShaderSourceProgram shader = ParseShader("res/shaders/Basic.shader");
	//std::cout << "VERTEX" << std::endl;
	//std::cout << shader.VertexShader << std::endl;
	unsigned int shaderProgram = CreateShader(shader.VertexShader, shader.FragmentShader);
	glUseProgram(shaderProgram);

	int location = glGetUniformLocation(shaderProgram, "u_Color");
	ASSERT(location != -1);
	glUniform4f(location, 0.5f, 0.2f, 0.1f, 1.0f);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		//glDrawArrays(GL_TRIANGLES, 0, 6);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);


		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glDeleteProgram(shaderProgram);

	glfwTerminate();
	return 0;
}