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
			return uParamColor;
		)";
	}

	void onCreate() {
		// TODO: How to maintain the same size accross nodes?
		outputWidth = 512;
		outputHeight = 512;
		addParam("Color", NodeValueType::float4);
	}

};

class MixNode : public GraphicsNode {
public:
	std::string source() {
		return R"(
			vec4 va = Sample(uInA, cUV);
			vec4 vb = Sample(uInB, cUV);
			float vf = uParamFactor;
			if (uInFactorConnected) {
				vf *= dot(Sample(uInFactor, cUV).rgb, vec3(0.299, 0.587, 0.114));
			}
			return mix(va, vb, vf);
		)";
	}

	void onCreate() {
		// TODO: How to maintain the same size accross nodes?
		outputWidth = 512;
		outputHeight = 512;
		addInput("A", NodeValueType::image);
		addInput("B", NodeValueType::image);
		addInput("Factor", NodeValueType::image);
		addParam("Factor", NodeValueType::float1);
	}

};
