#include <iostream>
#include <GL/glew.h>
#include <SFML/Graphics.hpp>

GLuint ProgramBase;
GLuint ProgramUniform;
GLuint ProgramGradient;

GLint Attrib_vertex_base;

GLint Attrib_vertex_uniform;
GLint Uni_vertex;

GLint Attrib_vertex_gradient;
GLint Attrib_color;

GLuint VBO;
GLuint VBO_colors;


enum class Figure
{
	quadrilateral,
	fan,
	pentagon //#HAcK
};

enum class ColorFill {
	base,
	uniform,
	gradient
};

Figure fig = Figure::quadrilateral;
ColorFill colorFillType = ColorFill::base;

bool is_gradient = false;


//����������� ���������
const char* VertexShaderSourceForGradient = R"( 
#version 330 core
in vec2 coord;
in vec3 colorV;
out vec4 color;
void main() {
gl_Position = vec4(coord, 0.0, 1.0);
color=vec4(colorV,0.0);
}
)";

// ����������� �����������
const char* FragShaderGradient = R"(
#version 330 core
in vec4 color;
void main() {
gl_FragColor = color;
}
)";

//��������� ������ ������� � uniform
const char* VertexShaderSourceUniformBase = R"( 
#version 330 core
in vec2 coord;
void main() {
gl_Position = vec4(coord, 0.0, 1.0);
}
)";

//����������� ������ �������
const char* FragShaderBase = R"(
#version 330 core
void main() {
gl_FragColor = vec4(1.0,0,0,0);
}
)";


// ��� ������������ ������� � uniform-����������
const char* FragShaderUniform = R"(
#version 330 core
uniform vec4 uni_color;
void main() {
gl_FragColor = uni_color;
}
)";

struct Vertex {
	GLfloat x;
	GLfloat y;
};

void ShaderLog(unsigned int shader);

void Draw();

void ReleaseShaders();

void ReleaseVBO();

void Release();

void Init();

void InitVBO();

void InitShaderBase();
void InitShaderUniform();
void InitShaderGradient();

void checkOpenGLerror();

void InitVBOQuadrilateral();
void InitVBOPentagon();
void InitVBOFan();

void MyDraw();


/*����������� ������
1 - ���������������
2 - ����
3 - ���������� ������������

g - ����������
u - uniform
b - ��� ��������
*/
void buttonCheck(sf::Event event);

int main() {
	sf::Window window(sf::VideoMode(600, 600), "My OpenGL window", sf::Style::Default, sf::ContextSettings(24));
	window.setVerticalSyncEnabled(true);
	window.setActive(true);
	glewInit();
	Init();
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::Resized:
				glViewport(0, 0, event.size.width, event.size.height);
				break;
			case sf::Event::KeyReleased:
				buttonCheck(event);
			}
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Draw();
		window.display();
		//std::cout << "draw worked" << std::endl;
	}
	Release();
	return 0;
}

void buttonCheck(sf::Event event)
{
	Figure newFig = fig;
	ColorFill newColorFillType = colorFillType;
	switch (event.key.scancode)
	{
	case sf::Keyboard::Scan::G:
		std::cout << "G pressed" << std::endl;
		newColorFillType = ColorFill::gradient;
		break;
	case sf::Keyboard::Scan::U:
		std::cout << "U pressed" << std::endl;
		newColorFillType = ColorFill::uniform;

		break;
	case sf::Keyboard::Scan::B:
		std::cout << "B pressed" << std::endl;
		newColorFillType = ColorFill::base;
		break;
	case sf::Keyboard::Scan::Numpad1:
	case sf::Keyboard::Scan::Num1:
		std::cout << "1 pressed" << std::endl;
		newFig = Figure::quadrilateral;
		break;
	case sf::Keyboard::Scan::Numpad2:
	case sf::Keyboard::Scan::Num2:
		std::cout << "2 pressed" << std::endl;
		newFig = Figure::fan;
		break;
	case sf::Keyboard::Scan::Numpad3:
	case sf::Keyboard::Scan::Num3:
		std::cout << "3 pressed" << std::endl;
		newFig = Figure::pentagon;
		break;
	default:
		break;
	}

	if (newFig != fig)
	{
		switch (newFig)
		{
		case Figure::quadrilateral:
			InitVBOQuadrilateral();
			break;
		case Figure::fan:
			InitVBOFan();

			break;
		case Figure::pentagon:
			InitVBOPentagon();
			break;
		}
	}
	if (newColorFillType != colorFillType)
	{
		//������ �������� ��� ����������� � ����� ������ ���������
	}
	fig = newFig;
	colorFillType = newColorFillType;
}

void ShaderLog(unsigned int shader)
{
	int infologLen = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
	if (infologLen > 1)
	{
		int charsWritten = 0;
		std::vector<char> infoLog(infologLen);
		glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog.data());
		std::cout << "InfoLog: " << infoLog.data() << std::endl;
	}
}

void Draw() {

	GLint Attrib_vertex = 0;

	// ������������� ��������� ��������� �������
	switch (colorFillType)
	{
	case ColorFill::base:
		glUseProgram(ProgramBase);
		Attrib_vertex = Attrib_vertex_base;
		break;
	case ColorFill::uniform:
		glUseProgram(ProgramUniform);
		glUniform4f(Uni_vertex, 1, 0.3, 0.3, 1);
		Attrib_vertex = Attrib_vertex_uniform;
		break;
	case ColorFill::gradient:
		glUseProgram(ProgramGradient);
		glEnableVertexAttribArray(Attrib_color);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_colors);
		glVertexAttribPointer(Attrib_color, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
		Attrib_vertex = Attrib_vertex_gradient;
		break;
	}



	glEnableVertexAttribArray(Attrib_vertex); // �������� ������ ���������

	glBindBuffer(GL_ARRAY_BUFFER, VBO); // ���������� VBO

	GLsizei size = 0;
	switch (fig)
	{
	case Figure::quadrilateral:
		size = 4;
		break;
	case Figure::fan:
		size = 6;
		break;
	case Figure::pentagon:
		size = 5;
		break;
	}
	glVertexAttribPointer(Attrib_vertex, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);


	glBindBuffer(GL_ARRAY_BUFFER, 0); // ��������� VBO

	glDrawArrays(GL_TRIANGLE_FAN, 0, size);
	glDisableVertexAttribArray(Attrib_vertex); // ��������� ������ ���������

	if (colorFillType == ColorFill::gradient)
		glDisableVertexAttribArray(Attrib_color); // ��������� ������ ���������


	glUseProgram(0); // ��������� ��������� ���������
	checkOpenGLerror();
}

void MyDraw() {

	glUseProgram(ProgramGradient); // ������������� ��������� ��������� �������
	// �������� ������� ���������
	glEnableVertexAttribArray(Attrib_color);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_colors);
	glVertexAttribPointer(Attrib_color, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

	glEnableVertexAttribArray(Attrib_vertex_gradient);
	// ���������� VBO_position
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(Attrib_vertex_gradient, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
	// ���������� VBO_color
	glBindBuffer(GL_ARRAY_BUFFER, 0); // ��������� VBO
	glDrawArrays(GL_TRIANGLES, 0, 6); // �������� ������ �� ����������(������)
	// ��������� ������� ���������
	glDisableVertexAttribArray(Attrib_vertex_gradient);
	glDisableVertexAttribArray(Attrib_color);
	glUseProgram(0); checkOpenGLerror();
}


void Release() {
	// �������
	ReleaseShaders();
	// ��������� �����
	ReleaseVBO();
}

void ReleaseVBO() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO);
}

void ReleaseShaders() {
	// ��������� ����, �� ��������� ��������� ���������
	glUseProgram(0);
	// ������� ��������� ���������
	glDeleteProgram(ProgramBase);
	glDeleteProgram(ProgramUniform);
	glDeleteProgram(ProgramGradient);
}

void Init() {
	// �������
	InitShaderBase();
	InitShaderUniform();
	InitShaderGradient();
	// ��������� �����
	InitVBO();
}

void InitVBOQuadrilateral()
{
	float data[4][2] = { {-1,-1},{-1,1}, {1,1} ,{1,-1} };
	float color[6][3] = { {1.0,0.0,0.0}, {1.0,1.0,0.0},{1.0,1.0,0.0},{0,1.0,0.0} };

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_colors);
	glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
	checkOpenGLerror();
}

void InitVBOFan()
{
	float data[6][2] = { {0,-1},{1,0},{0.9,0.8},{0.7,0.9},{0.1,1},{-0.5,0.5} };
	float color[6][3] = { {1.0,0.0,0.0}, {1.0,1.0,0.0},{1.0,1.0,0.0},{1.0,1.0,0.0},{0.0,1.0,0.0},{1.0,0.0,1.0} };

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_colors);
	glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
	checkOpenGLerror();
}

void InitVBOPentagon()
{
	float data[6][2] = { {0,1},{0.951,0.309},{0.587,-0.809},{-0.587,-0.809}, {-0.951,0.309} };
	float color[6][3] = { {1.0,0.0,0.0}, {1.0,1.0,0.0},{1.0,0.5,0.5},{1.0,0.0,1.0},{0.0,1.0,0.0} };

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_colors);
	glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
	checkOpenGLerror();
}

void InitVBO() {
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &VBO_colors);
	InitVBOQuadrilateral();
}

void InitShaderBase() {
	// ������� ��������� ������
	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	// �������� �������� ���
	glShaderSource(vShader, 1, &VertexShaderSourceUniformBase, NULL); //���������
	// ����������� ������
	glCompileShader(vShader);
	std::cout << "vertex shader \n";
	// ������� ������ ���� �������
	ShaderLog(vShader); //������ ������� ���� � ������������
	// ������� ����������� ������
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
	// �������� �������� ���
	glShaderSource(fShader, 1, &FragShaderBase, NULL); //�����������
	// ����������� ������
	glCompileShader(fShader);
	std::cout << "fragment shader \n";
	// ������� ������ ���� �������
	ShaderLog(fShader);
	// ������� ��������� � ����������� ������� � ���
	ProgramBase = glCreateProgram();
	glAttachShader(ProgramBase, vShader);
	glAttachShader(ProgramBase, fShader);
	// ������� ��������� ���������
	glLinkProgram(ProgramBase);
	// ��������� ������ ������
	int link_ok;
	glGetProgramiv(ProgramBase, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		std::cout << "error attach shaders \n";
		return;
	}
	// ���������� ID �������� �� ��������� ���������
	const char* attr_name_coord = "coord"; //��� � �������
	Attrib_vertex_base = glGetAttribLocation(ProgramBase, attr_name_coord);
	if (Attrib_vertex_base == -1) {
		std::cout << "could not bind attrib " << attr_name_coord << std::endl;
		return;
	}
	checkOpenGLerror();
}

void InitShaderUniform() {
	// ������� ��������� ������
	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	// �������� �������� ���
	glShaderSource(vShader, 1, &VertexShaderSourceUniformBase, NULL);
	// ����������� ������
	glCompileShader(vShader);
	std::cout << "vertex shader \n";
	// ������� ������ ���� �������
	ShaderLog(vShader); //������ ������� ���� � ������������
	// ������� ����������� ������
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
	// �������� �������� ���
	glShaderSource(fShader, 1, &FragShaderUniform, NULL);
	// ����������� ������
	glCompileShader(fShader);
	std::cout << "fragment shader \n";
	// ������� ������ ���� �������
	ShaderLog(fShader);// ������� ��������� � ����������� ������� � ���
	ProgramUniform = glCreateProgram();
	glAttachShader(ProgramUniform, vShader);
	glAttachShader(ProgramUniform, fShader);
	// ������� ��������� ���������
	glLinkProgram(ProgramUniform);
	// ��������� ������ ������
	int link_ok;
	glGetProgramiv(ProgramUniform, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		std::cout << "error attach shaders \n";
		return;
	}

	//UNIFORM
	const char* uni_name = "uni_color"; //��� � �������
	Uni_vertex = glGetUniformLocation(ProgramUniform, uni_name);
	if (Uni_vertex == -1)
	{
		std::cout << "could not bind uniform " << uni_name << std::endl;
		return;
	}


	// ���������� ID �������� �� ��������� ���������
	const char* attr_name = "coord"; //��� � �������
	Attrib_vertex_uniform = glGetAttribLocation(ProgramUniform, attr_name);
	if (Attrib_vertex_uniform == -1) {
		std::cout << "could not bind attrib " << attr_name << std::endl;
		return;
	}
	checkOpenGLerror();
}

void InitShaderGradient() {
	// ������� ��������� ������
	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	// �������� �������� ���
	glShaderSource(vShader, 1, &VertexShaderSourceForGradient, NULL); //���������
	// ����������� ������
	glCompileShader(vShader);
	std::cout << "vertex shader \n";
	// ������� ������ ���� �������
	ShaderLog(vShader); //������ ������� ���� � ������������
	// ������� ����������� ������
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
	// �������� �������� ���
	glShaderSource(fShader, 1, &FragShaderGradient, NULL); //�����������
	// ����������� ������
	glCompileShader(fShader);
	std::cout << "fragment shader \n";
	// ������� ������ ���� �������
	ShaderLog(fShader);
	// ������� ��������� � ����������� ������� � ���
	ProgramGradient = glCreateProgram();
	glAttachShader(ProgramGradient, vShader);
	glAttachShader(ProgramGradient, fShader);
	// ������� ��������� ���������
	glLinkProgram(ProgramGradient);
	// ��������� ������ ������
	int link_ok;
	glGetProgramiv(ProgramGradient, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		std::cout << "error attach shaders \n";
		return;
	}
	// ���������� ID �������� �� ��������� ���������
	const char* attr_name_coord = "coord"; //��� � �������
	Attrib_vertex_gradient = glGetAttribLocation(ProgramGradient, attr_name_coord);
	if (Attrib_vertex_gradient == -1) {
		std::cout << "could not bind attrib " << attr_name_coord << std::endl;
		return;
	}

	const char* attr_name_color = "colorV"; //��� � �������
	Attrib_color = glGetAttribLocation(ProgramGradient, attr_name_color);
	if (Attrib_color == -1) {
		std::cout << "could not bind attrib " << attr_name_color << std::endl;
		return;
	}
	checkOpenGLerror();
}

void checkOpenGLerror()
{
	GLenum err;
	int count = 0;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		count++;
		if (count > 10)
			return;
		std::cout << "InfoLog: " << err << std::endl;
	}
}