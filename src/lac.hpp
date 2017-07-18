#ifndef __LAC_HPP__
#define __LAC_HPP__
#include <vector>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "stb_image.h"
struct lacVec2 {
	int x;
	int y;
};
typedef void(*fnGetClientRect)(int&, int&);
template<typename T>
class _lac {
public:
	static _lac& instance() {
		static _lac<T> inst;
		return inst;
	}
	void init(fnGetClientRect gcr) {
		this->gcr = gcr;
	}
	int getClientRect(int& w, int& h) {
		if (gcr) {
			gcr(w, h);
			return 0;
		}
		else {
			return -1;
		}
	}
private:
	fnGetClientRect gcr;
};
typedef _lac<void> lac;
// Shaders


template<typename T>
class _lacProgram {
public:
	enum loadType {
		loadType_file,
		loadType_buffer,
	};
public:
	_lacProgram() {
		program = glCreateProgram();
	}
	GLint addShader(const GLchar* buf, GLenum type, loadType lt = loadType_buffer) {
		if (lt == loadType_buffer)
		{
			GLuint shader = glCreateShader(type);
			glShaderSource(shader, 1, &buf, NULL);
			glCompileShader(shader);
			// Check for compile time errors
			GLchar infoLog[512];
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
				return -1;
			}
			else {
				vec_shader.push_back(shader);
				return 0;
			}
		}
		return -1;
	}

	GLint linkAll() {
		GLchar infoLog[512];
		for (std::vector<GLuint>::iterator iter = vec_shader.begin(); iter != vec_shader.end(); iter++)
		{
			glAttachShader(program, *iter);
		}
		glLinkProgram(program);
		// Check for linking errors
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(program, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
			return -1;
		}
		for (std::vector<GLuint>::iterator iter = vec_shader.begin(); iter != vec_shader.end(); iter++)
		{
			glDeleteShader(*iter);
		}
		vec_shader.clear();
		return 0;
	}
	void use() {
		glUseProgram(program);
	}
	GLint location(const char* name) {
		return glGetUniformLocation(program, name);
	}
	void bindAttrLoaction(GLint idx, const char* name) {
		glBindAttribLocation(program, idx, name);
	}
private:
	GLuint program;
	std::vector<GLuint> vec_shader;
	GLint success;
};
typedef _lacProgram<void> lacProgram;

template<typename T>
class _lacGL {
public:
	_lacGL() {}
	static _lacGL& instance() {
		static _lacGL<T> inst;
		return inst;
	}
	int setViewpoint(int x, int y, int w, int h) {
		glViewport(x, y, w, h);
		return 0;
	}
	int setScreen(int x, int y, int w, int h, int sw, int sh) {
		int ry = sh - y - h;
		glViewport(x, ry, w, h);
		return 0;
	}
};
typedef _lacGL<void> lacGL;

template<typename T>
class _lacShape {
public:
	_lacShape() :
		index_flag(0),
		ebo_size(0),
		vbo_size(0),
		polygon_mode(GL_FILL),
		polygon_face(GL_FRONT_AND_BACK),
		vecdata(0),
		vecidx(0){
		lacProgram &prog=getProg();
		const GLchar* vertexShaderSource = "attribute vec3 position;\n"
			"void main()\n"
			"{\n"
			"gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
			"}\0";
		const GLchar* fragmentShaderSource = "out vec4 color;\n"
			"void main()\n"
			"{\n"
			"color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
			"}\n\0";
		prog.addShader(vertexShaderSource, GL_VERTEX_SHADER);
		prog.addShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
		prog.linkAll();
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		idx_size = 0;
		GLfloat vertices[] = {
			1.0f, 1.0f, 0.0f,   // 右上角
			1.0f, -1.0f, 0.0f,  // 右下角
			-1.0f, -1.0f, 0.0f, // 左下角
			-1.0f, 1.0f, 0.0f   // 左上角

		};
		GLuint indices[] = { // 注意索引从0开始! 
			0, 1, 2, 3,
		};
		data(vertices, sizeof(vertices));
		index(indices, sizeof(indices));
	}
	void data(GLfloat buf[], GLsizeiptr size) {
		if (vecdata) { delete vecdata; vecdata = 0; }
		vecdata = new GLfloat[size/sizeof(GLfloat)];
		memcpy(vecdata, buf, size);
		vbo_size = (GLuint)size;
	}
	void index(GLuint buf[], GLsizeiptr size) {
		if (vecidx) { delete vecidx; vecidx = 0; }
		idx_size = size / sizeof(GLint);
		vecidx = new GLint[idx_size];
		memcpy(vecidx, buf, size);
		index_flag = 1;
		ebo_size = (GLuint)size;
	}
	void draw(GLenum mode, GLuint count = -1, GLint frist = 0) {
		
		int width, height;
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vbo_size, vecdata, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ebo_size, vecidx, GL_STATIC_DRAW);
		
		getProg().use();
		glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs)
		glPolygonMode(polygon_face, polygon_mode);
		glLineWidth(10.0f);
		lac::instance().getClientRect(width, height);
		lacGL::instance().setScreen(client_pos.x, client_pos.y, client_size.x, client_size.y, width, height);
		if (index_flag) {
			glBindVertexArray(VAO);
			glDrawElements(mode, count<idx_size ? count : idx_size, GL_UNSIGNED_INT, (const void*)frist);
			glBindVertexArray(0);
		}
		else {
			glBindVertexArray(VAO);
			glDrawArrays(mode, frist, count<vbo_size ? count : vbo_size);
			glBindVertexArray(0);
		}
	}
	void mode(GLenum mode, GLenum face = GL_FRONT_AND_BACK) {
		polygon_mode = mode;
		polygon_face = face;
	}
	lacVec2 location() {
		return client_pos;
	}
	lacVec2 size() {
		return client_size;
	}
	void location(int x, int y) {
		client_pos.x = x;
		client_pos.y = y;
	}
	void size(int w, int h) {
		client_size.x = w;
		client_size.y = h;
	}
private:
	static lacProgram& getProg() {
		static lacProgram inst;
		return inst;
	}
private:
	GLuint VBO, VAO, EBO;
	int index_flag;
	GLuint vbo_size, ebo_size,idx_size;
	GLenum polygon_mode, polygon_face;
	lacVec2 client_pos;
	lacVec2 client_size;
	GLfloat *vecdata;
	GLint *vecidx;
};
typedef _lacShape<void> lacShape;
template<typename T>
class _lacImage {
public:
	_lacImage() :
		index_flag(0),
		ebo_size(0),
		vbo_size(0),
		polygon_mode(GL_FILL),
		polygon_face(GL_FRONT_AND_BACK),
		vecdata(0),
		vecidx(0) {
		lacProgram &prog = getProg();
		const GLchar* vertexShaderSource = 
			"#version 330\n"
			"layout(location = 0) in vec4 vposition;\n"
			"layout(location = 1) in vec2 vtexcoord;\n"
			"out vec2 ftexcoord;\n"
			"void main() {\n"
			"   ftexcoord = vec2(vtexcoord.x,1.0f-vtexcoord.y);\n"
			"   gl_Position = vposition;\n"
			"}\n";
		const GLchar* fragmentShaderSource = 
			"#version 330\n"
			"uniform sampler2D tex;\n" // texture uniform
			"in vec2 ftexcoord;\n"
			"layout(location = 0) out vec4 FragColor;\n"
			"void main() {\n"
			"   FragColor = texture(tex, ftexcoord);\n"
			"}\n";
		prog.addShader(vertexShaderSource, GL_VERTEX_SHADER);
		prog.addShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
		prog.linkAll();
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		client_pos = { 0 };
		client_size = {0,0};
		GLfloat vertices[] = {
			//  X     Y     Z           U     V     
			1.0f, 1.0f, 0.0f,       1.0f, 1.0f, // vertex 0
			-1.0f, 1.0f, 0.0f,       0.0f, 1.0f, // vertex 1
			1.0f,-1.0f, 0.0f,       1.0f, 0.0f, // vertex 2
			-1.0f,-1.0f, 0.0f,       0.0f, 0.0f, // vertex 3
		}; // 4 vertices with 5 components (floats) each
		GLuint indices[] = {
			0,1,2, // first triangle
			2,1,3, // second triangle
		};
		data(vertices, sizeof(vertices));
		index(indices, sizeof(indices));
	}
	int load(const char*path, int type) {
		
		stbi_uc *imagedata = stbi_load(path, &width, &height, &comp, type);
		client_size.x = width;
		client_size.y = height;
		glGenTextures(1, &texture1);
		glBindTexture(GL_TEXTURE_2D, texture1); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
												// Set our texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Set texture filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Load, create texture and generate mipmaps
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imagedata);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
		STBI_FREE(imagedata);
		return 0;
	}
	void data(GLfloat buf[], GLsizeiptr size) {
		if (vecdata) { delete vecdata; vecdata = 0; }
		vecdata = new GLfloat[size / sizeof(GLint)];
		memcpy(vecdata, buf, size); 
		vbo_size = (GLuint)size;
	}
	void index(GLuint buf[], GLsizeiptr size) {
		if (vecidx) { delete vecidx; vecidx = 0; }
		vecidx = new GLint[size / sizeof(GLint)];
		memcpy(vecidx, buf, size);
		index_flag = 1;
		ebo_size = (GLuint)size;
	}
	void draw(GLenum mode, GLuint count = -1, GLint frist = 0) {

		int cwidth, cheight;
		glBindVertexArray(VAO);
		
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vbo_size, vecdata, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (char*)0 + 0 * sizeof(GLfloat));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (char*)0 + 3 * sizeof(GLfloat));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ebo_size, vecidx, GL_STATIC_DRAW);
		getProg().use();

		lac::instance().getClientRect(cwidth, cheight);
		lacGL::instance().setScreen(client_pos.x, client_pos.y, client_size.x, client_size.y, cwidth, cheight);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glUniform1i(getProg().location("tex"), 0);
		if (index_flag) {
			glBindVertexArray(VAO);
			glDrawElements(mode, count<ebo_size ? count : ebo_size, GL_UNSIGNED_INT, (const void*)frist);
			glBindVertexArray(0);
		}
		else {
			glBindVertexArray(VAO);
			glDrawArrays(mode, frist, count<vbo_size ? count : vbo_size);
			glBindVertexArray(0);
		}
	}
	void mode(GLenum mode, GLenum face = GL_FRONT_AND_BACK) {
		polygon_mode = mode;
		polygon_face = face;
	}
	lacVec2 location() {
		return client_pos;
	}
	lacVec2 size() {
		return client_size;
	}
	void location(int x, int y) {
		client_pos.x = x;
		client_pos.y = y;
	}
	void size(int w, int h) {
		client_size.x = w;
		client_size.y = h;
	}
private:
	static lacProgram& getProg() {
		static lacProgram inst;
		return inst;
	}
private:
	GLuint VBO, VAO, EBO;
	int index_flag;
	GLuint vbo_size, ebo_size;
	GLenum polygon_mode, polygon_face;
	lacVec2 client_pos;
	lacVec2 client_size;
	GLfloat *vecdata;
	GLint *vecidx;
	int width, height, comp;
	GLuint texture1;
};
typedef _lacImage<void> lacImage;

template<typename T>
class _lacYuv {
public:
	_lacYuv():
		index_flag(0),
		ebo_size(0),
		vbo_size(0),
		vecdata(0),
		vecidx(0) {
		lacProgram &prog = getProg();
		const GLchar* vertexShaderSource =
			"attribute vec4 vertexIn;"
			"attribute vec2 textureIn;"
			"varying vec2 textureOut;"
			"void main(void)"
			"{"
			"gl_Position = vertexIn;"
			"textureOut = textureIn;"
			"}";
		const GLchar* fragmentShaderSource =
			"varying vec2 textureOut;"
			"uniform sampler2D tex_y;"
			"uniform sampler2D tex_u;"
			"uniform sampler2D tex_v;"
			"void main(void)"
			"{"
			"	vec3 yuv;"
			"	vec3 rgb;"
			"	yuv.x = texture2D(tex_y, textureOut).r;"
			"	yuv.y = texture2D(tex_u, textureOut).r - 0.5;"
			"	yuv.z = texture2D(tex_v, textureOut).r - 0.5;"
			"	rgb = mat3(1, 1, 1,"
			"		0, -0.39465, 2.03211,"
			"		1.13983, -0.58060, 0) * yuv;"
			"	gl_FragColor = vec4(rgb, 1);"
			"}";
		prog.addShader(vertexShaderSource, GL_VERTEX_SHADER);
		prog.addShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
		prog.linkAll();
		
		textureUniformY = prog.location("tex_y");
		textureUniformU = prog.location("tex_u");
		textureUniformV = prog.location("tex_v");

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		client_pos = { 0 };
		client_size = { 0,0 };
		GLfloat vertices[] = {
			//  X     Y     Z           U     V     
			-1.0f,-1.0f, 0.0f,       0.0f, 1.0f, // vertex 0
			1.0f,-1.0f, 0.0f,       1.0f, 1.0f, // vertex 1
			-1.0f, 1.0f, 0.0f,       0.0f, 0.0f, // vertex 2
			1.0f, 1.0f, 0.0f,       1.0f, 0.0f, // vertex 3
		}; // 4 vertices with 5 components (floats) each
		//GLuint indices[] = {
		//	0,1,2, // first triangle
		//	2,3,1, // second triangle
		//};
		data(vertices, sizeof(vertices));
		//index(indices, sizeof(indices));
		glGenTextures(1, &id_y);
		glBindTexture(GL_TEXTURE_2D, id_y);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &id_u);
		glBindTexture(GL_TEXTURE_2D, id_u);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &id_v);
		glBindTexture(GL_TEXTURE_2D, id_v);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	void data(GLfloat buf[], GLsizeiptr size) {
		if (vecdata) { delete vecdata; vecdata = 0; }
		vecdata = new GLfloat[size / sizeof(GLint)];
		memcpy(vecdata, buf, size);
		vbo_size = (GLuint)size;
	}
	void index(GLuint buf[], GLsizeiptr size) {
		if (vecidx) { delete vecidx; vecidx = 0; }
		vecidx = new GLint[size / sizeof(GLint)];
		memcpy(vecidx, buf, size);
		index_flag = 1;
		ebo_size = (GLuint)size;
	}
	void draw(const unsigned char* buf, int pixel_w,int pixel_h,GLint frist=0) {

		/*static const GLfloat textureVertices[] = {
			0.0f,  1.0f,
			1.0f,  1.0f,
			0.0f,  0.0f,
			1.0f,  0.0f,
		};
		static const GLfloat vertexVertices[] = {
			-1.0f, -1.0f,
			1.0f, -1.0f,
			-1.0f,  1.0f,
			1.0f,  1.0f,
		};*/
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vbo_size, vecdata, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (char*)0 + 0 * sizeof(GLfloat));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (char*)0 + 3 * sizeof(GLfloat));

		/*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ebo_size, vecidx, GL_STATIC_DRAW);*/
		getProg().use();
		//Clear
		int cwidth, cheight;
		plane[0] = buf;
		plane[1] = plane[0] + pixel_w*pixel_h;
		plane[2] = plane[1] + pixel_w*pixel_h / 4;

		lac::instance().getClientRect(cwidth, cheight);
		lacGL::instance().setScreen(client_pos.x, client_pos.y, pixel_w, pixel_h, cwidth, cheight);

		//Y  
		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, id_y);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, pixel_w, pixel_h, 0, GL_RED, GL_UNSIGNED_BYTE, plane[0]);

		glUniform1i(textureUniformY, 0);
		//U  
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, id_u);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, pixel_w / 2, pixel_h / 2, 0, GL_RED, GL_UNSIGNED_BYTE, plane[1]);
		glUniform1i(textureUniformU, 1);
		//V  
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, id_v);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, pixel_w / 2, pixel_h / 2, 0, GL_RED, GL_UNSIGNED_BYTE, plane[2]);
		glUniform1i(textureUniformV, 2);

		if (index_flag) {
			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLE_STRIP, ebo_size , GL_UNSIGNED_INT, (const void*)frist);
			glBindVertexArray(0);
		}
		else {
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLE_STRIP, frist, 4);
			glBindVertexArray(0);
		}

	}
	
	lacVec2 location() {
		return client_pos;
	}
	lacVec2 size() {
		return client_size;
	}
	void location(int x, int y) {
		client_pos.x = x;
		client_pos.y = y;
	}
	void size(int w, int h) {
		client_size.x = w;
		client_size.y = h;
	}
private:
	static lacProgram& getProg() {
		static lacProgram inst;
		return inst;
	}
private:
	int index_flag;
	GLuint VBO, VAO, EBO;
	GLuint vbo_size, ebo_size;
	lacVec2 client_pos;
	lacVec2 client_size;
	GLfloat *vecdata;
	GLint *vecidx;
	GLuint id_y, id_u, id_v;
	GLuint textureUniformY, textureUniformU, textureUniformV;
	const unsigned char *plane[3];
	
};
typedef _lacYuv<void> lacYuv;

#endif // !__LAC_HPP__
