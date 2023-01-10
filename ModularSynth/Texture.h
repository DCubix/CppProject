#pragma once

#include <array>
#include <cstdint>
#include <cassert>

#include "glad/glad.h"

class Texture {
public:
	Texture(GLenum internalFormat = GL_RGBA8, size_t dimensions = 2, GLenum target = GL_TEXTURE_2D);
	Texture(const std::array<uint32_t, 3>& size, GLenum internalFormat = GL_RGBA8, size_t dimensions = 2, GLenum target = GL_TEXTURE_2D);
	virtual ~Texture();

	GLuint id() const { return m_id; }
	GLenum internalFormat() const { return m_internalFormat; }
	GLenum format() const { return m_format; }
	const std::array<uint32_t, 3>& size() const { return m_size; }

	void loadFromMemory(void* data, GLenum format, GLenum type) {
		assert(m_target == GL_TEXTURE_2D);

		glTextureSubImage2D(m_id, 0, 0, 0, m_size[0], m_size[1], format, type, data);
		glGenerateTextureMipmap(m_id);
	}

private:
	GLuint m_id;
	GLenum m_target;
	GLenum m_internalFormat, m_format{ 0 };
	std::array<uint32_t, 3> m_size;

	void init(size_t dimensions);
};
