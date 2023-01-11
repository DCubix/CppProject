#pragma once

#include "NodeGraph.h"
#include "Shader.h"
#include "Texture.h"

#include <memory>
#include <map>

class GraphicsNode : public Node {
public:
	virtual std::string source() = 0;
	virtual void onCreate() = 0;

	void setup() override final;
	NodeValue solve();

	uint32_t outputWidth, outputHeight;

	void addParam(const std::string& name, NodeValueType type);

	const NodeValue& param(const std::string& name) { return m_params[name]; }

	void setParam(const std::string& name, const RawNodeValue& value) { m_params[name].value = value; m_changed = true; }
	void setParam(const std::string& name, float v) { m_params[name].value[0] = v; m_changed = true; }
	void setParam(const std::string& name, float x, float y) {
		m_params[name].value[0] = x;
		m_params[name].value[1] = y;
		m_changed = true;
	}
	void setParam(const std::string& name, float x, float y, float z) {
		m_params[name].value[0] = x;
		m_params[name].value[1] = y;
		m_params[name].value[2] = z;
		m_changed = true;
	}
	void setParam(const std::string& name, float x, float y, float z, float w) {
		m_params[name].value[0] = x;
		m_params[name].value[1] = y;
		m_params[name].value[2] = z;
		m_params[name].value[3] = w;
		m_changed = true;
	}
	void setParam(const std::string& name, size_t index, float v) { m_params[name].value[index] = v; m_changed = true; }

	GLuint textureID() const { return m_texture->id(); }

	const std::map<std::string, NodeValue>& params() { return m_params; }

private:
	std::unique_ptr<Texture> m_texture;
	std::unique_ptr<Shader> m_shader;
	
	std::map<std::string, NodeValue> m_params;

	std::string processedSource();
};
