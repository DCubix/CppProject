#include "Shader.h"

Shader::~Shader() {
	if (!m_program) return;

	glDeleteProgram(m_program);
	m_program = 0;
}

void Shader::add(const std::string& src, GLenum type) {
	if (m_program == 0) {
		m_program = glCreateProgram();
	}

	GLuint shader = createShader(src, type);
	if (shader == 0) return;

	glAttachShader(m_program, shader);

	m_shaders.push_back(shader);
}

void Shader::link() {
	glLinkProgram(m_program);

	GLint status;
	glGetProgramiv(m_program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		glDeleteProgram(m_program);
		return;
	}

	for (auto shader : m_shaders) {
		glDetachShader(m_program, shader);
		glDeleteShader(shader);
	}
	m_shaders.clear();
}

GLint Shader::getUniformLocation(const std::string& name) {
	return glGetUniformLocation(m_program, name.c_str());
}

GLint Shader::getAttributeLocation(const std::string& name) {
	return glGetAttribLocation(m_program, name.c_str());
}

GLuint Shader::createShader(const std::string& src, GLenum type) {
	GLuint shader = glCreateShader(type);

	const char* srcRaw[] = { src.c_str() };
	const GLint srcLength[] = { src.size() };
	glShaderSource(shader, 1, srcRaw, srcLength);
	glCompileShader(shader);
	
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		glDeleteShader(shader);
		return 0;
	}

	return shader;
}
