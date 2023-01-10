#include "GraphicsNode.h"

#include <format>
#include <ranges>
#include <string_view>
#include <iostream>

// From https://helloacm.com/convert-a-string-to-camel-case-format-in-c/#:~:text=How%20to%20Convert%20a%20String%20into%20Camel%20Case%20in%20C%2B%2B%3F&text=function,end()%2C%20data.
static std::string toCorrectCase(const std::string& text) {
	auto rn = text
		| std::ranges::views::split(' ')
		| std::ranges::views::transform([](auto&& str) { return std::string_view(&*str.begin(), std::ranges::distance(str)); });

	std::vector<std::string> words(rn.begin(), rn.end());
	if (words.empty()) return "";

	std::function<std::string(std::string)> stringToLower = [](std::string data) {
		std::transform(data.begin(), data.end(), data.begin(), [](unsigned char c) {
			return std::tolower(c);
			});
		return data;
	};
	std::string ans = "";//stringToLower(words[0]);
	for (int i = 0; i < words.size(); ++i) {
		ans += (char)std::toupper(words[i][0]);
		ans += stringToLower(words[i].substr(1));
	}
	return ans;
}

void GraphicsNode::addParam(const std::string& name, NodeValueType type) {
	m_params[name] = {
		.value = RawNodeValue(),
		.type = type
	};
}

void GraphicsNode::setup() {
	onCreate();

	m_texture = std::unique_ptr<Texture>(new Texture({ outputWidth, outputHeight }, GL_RGBA32F));
	m_shader = std::unique_ptr<Shader>(new Shader());
	m_shader->add(processedSource(), GL_COMPUTE_SHADER);
	m_shader->link();

	std::cout << processedSource() << std::endl;
}

static void setUniformNodeValue(Shader* shader, const std::string& name, const NodeValue& nv) {
	switch (nv.type) {
		case NodeValueType::float1: shader->uniform<1>(name, { nv.value[0] }); break;
		case NodeValueType::float2: shader->uniform<2>(name, { nv.value[0], nv.value[1] }); break;
		case NodeValueType::float3: shader->uniform<3>(name, { nv.value[0], nv.value[1], nv.value[2] }); break;
		case NodeValueType::float4: shader->uniform<4>(name, nv.value); break;
		case NodeValueType::image: shader->uniformInt<1>(name, { int(nv.value[0]) }); break;
	}
}

NodeValue GraphicsNode::solve() {
	glBindImageTexture(0, m_texture->id(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glUseProgram(m_shader->id());

	m_shader->uniformInt<2>("uInternalResolution", { int(m_texture->size()[0]), int(m_texture->size()[1]) });

	for (size_t i = 0; i < m_inputs.size(); i++) {
		NodeValue& nv = m_inputs[i];
		std::string name = std::format("uIn{}", toCorrectCase(m_inputNames[i]));
		setUniformNodeValue(m_shader.get(), name, nv);
	}

	for (auto&& [paramName, nv] : m_params) {
		std::string name = std::format("uParam{}", toCorrectCase(paramName));
		setUniformNodeValue(m_shader.get(), name, nv);
	}

	glDispatchCompute(m_texture->size()[0], m_texture->size()[1], 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	return NodeValue();
}

std::string GraphicsNode::processedSource() {
	std::string src = R"(#version 460
	layout (local_size_x=1, local_size_y=1) in;
	layout (rgba32f, binding=0) uniform image2D bOutput;
	
	uniform ivec2 uInternalResolution;

	)";

	for (size_t i = 0; i < m_inputs.size(); i++) {
		NodeValue& nv = m_inputs[i];
		
		src += "uniform ";

		switch (nv.type) {
			case NodeValueType::float1: src += "float"; break;
			case NodeValueType::float2: src += "vec2"; break;
			case NodeValueType::float3: src += "vec3"; break;
			case NodeValueType::float4: src += "vec4"; break;
			case NodeValueType::image: src += "sampler2D"; break;
		}

		src += " ";
		src += std::format("uIn{};\n", toCorrectCase(m_inputNames[i]));
	}

	src += "\n";

	for (auto&& [ paramName, nv ] : m_params) {
		src += "uniform ";

		switch (nv.type) {
			case NodeValueType::float1: src += "float"; break;
			case NodeValueType::float2: src += "vec2"; break;
			case NodeValueType::float3: src += "vec3"; break;
			case NodeValueType::float4: src += "vec4"; break;
			case NodeValueType::image: src += "sampler2D"; break;
		}

		src += " ";
		src += std::format("uParam{};\n", toCorrectCase(paramName));
	}

	src += "\n";

	src += R"(vec4 mainFunc(vec2 cUV) {
	#line 1
	)";

	src += source();
	src += "\n}";

	src += R"(
	void main() {
		ivec2 c__Coords = ivec2(gl_GlobalInvocationID.xy);
		vec2 c__uv = vec2(c__Coords) / vec2(uInternalResolution);
		vec4 pixel = mainFunc(c__uv);
		imageStore(bOutput, c__Coords, pixel);
	})";

	return src;
}
