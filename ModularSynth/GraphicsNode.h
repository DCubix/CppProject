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
	NodeValue& param(const std::string& name) { return m_params[name]; }

	GLuint textureID() const { return m_texture->id(); }

private:
	std::unique_ptr<Texture> m_texture;
	std::unique_ptr<Shader> m_shader;
	
	std::map<std::string, NodeValue> m_params;

	std::string processedSource();
};

// Basic nodes for "testing"
class ColorNode : public GraphicsNode {
public:
	std::string source() {
		return R"(
			return uParamColor * vec4(cUV, 1.0, 1.0);
		)";
	}

	void onCreate() {
		// TODO: How to maintain the same size accross nodes?
		outputWidth = 512;
		outputHeight = 512;
		addParam("Color", NodeValueType::float4);
	}

};
