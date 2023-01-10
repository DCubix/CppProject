#pragma once

#include "glad/glad.h"

#include <vector>
#include <array>
#include <string>
#include <functional>
#include <cassert>

class Shader {
public:
	Shader() = default;
	virtual ~Shader();

	void add(const std::string& src, GLenum type);
	void link();

	GLint getUniformLocation(const std::string& name);
	GLint getAttributeLocation(const std::string& name);

	template <size_t S>
	void uniform(const std::string& name, const std::array<float, S>& v) {
		static_assert((S >= 1 && S <= 4) || S == 16 || S == 9);

		GLint loc = getUniformLocation(name);
		switch (S) {
			default: break;
			case 1: glUniform1f(loc, v[0]); break;
			case 2: glUniform2fv(loc, 1, &v[0]); break;
			case 3: glUniform3fv(loc, 1, &v[0]); break;
			case 4: glUniform4fv(loc, 1, &v[0]); break;
			case 9: glUniformMatrix3fv(loc, 1, false, v.data()); break;
			case 16: glUniformMatrix4fv(loc, 1, false, v.data()); break;
		}
	}

	template <size_t S>
	void uniformInt(const std::string& name, const std::array<int, S>& v) {
		static_assert(S >= 1 && S <= 4);

		GLint loc = getUniformLocation(name);
		switch (S) {
			default: break;
			case 1: glUniform1i(loc, v[0]); break;
			case 2: glUniform2iv(loc, 1, &v[0]); break;
			case 3: glUniform3iv(loc, 1, &v[0]); break;
			case 4: glUniform4iv(loc, 1, &v[0]); break;
		}
	}

	GLuint id() const { return m_program; }

private:
	GLuint m_program;
	std::vector<GLuint> m_shaders;

	GLuint createShader(const std::string& src, GLenum type);
};
