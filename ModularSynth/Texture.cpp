#include "Texture.h"

Texture::Texture(GLenum internalFormat, size_t dimensions, GLenum target) {
	m_target = target;
	m_internalFormat = internalFormat;
	m_size = { 1, 1, 1 };
	init(dimensions);
}

Texture::Texture(
	const std::array<uint32_t, 3>& size,
	GLenum internalFormat,
	size_t dimensions,
	GLenum target
) : Texture(internalFormat, dimensions, target)
{
	m_size = size;
	init(dimensions);
}

Texture::~Texture() {
	glDeleteTextures(1, &m_id);
	m_id = 0;
}

void Texture::init(size_t dimensions) {
	glCreateTextures(m_target, 1, &m_id);

	switch (dimensions) {
		default: break;
		case 1: glTextureStorage1D(m_id, 1, m_internalFormat, m_size[0]); break;
		case 2: glTextureStorage2D(m_id, 1, m_internalFormat, m_size[0], m_size[1]); break;
		case 3: glTextureStorage3D(m_id, 1, m_internalFormat, m_size[0], m_size[1], m_size[2]); break;
	}

	if (dimensions >= 1) {
		glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	}
	else if (dimensions >= 2) {
		glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else if (dimensions >= 3) {
		glTextureParameteri(m_id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	// TODO: Set filter
	glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}
