#include "GraphicsNode.h"

#include <format>
#include <ranges>
#include <string_view>
#include <iostream>
#include <fstream>

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

	m_texture = std::unique_ptr<Texture>(new Texture({ outputWidth, outputHeight }, GL_RGBA8));
	m_shader = std::unique_ptr<Shader>(new Shader());
	m_shader->add(processedSource(), GL_COMPUTE_SHADER);
	m_shader->link();

	addOutput("Output", NodeValueType::image);

	std::ofstream of{ "shader.glsl" };
	of << processedSource();
	of.close();
}

static void setUniformNodeValue(Shader* shader, const std::string& name, const NodeValue& nv, size_t index) {
	switch (nv.type) {
		case NodeValueType::float1: shader->uniform<1>(name, { nv.value[0] }); break;
		case NodeValueType::float2: shader->uniform<2>(name, { nv.value[0], nv.value[1] }); break;
		case NodeValueType::float3: shader->uniform<3>(name, { nv.value[0], nv.value[1], nv.value[2] }); break;
		case NodeValueType::float4: shader->uniform<4>(name, nv.value); break;
		case NodeValueType::image: {
			glBindImageTexture(index, GLuint(nv.value[0]), 0, false, 0, GL_READ_ONLY, GL_RGBA8);
		} break;
	}
}

NodeValue GraphicsNode::solve() {
	glBindImageTexture(0, m_texture->id(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA8);
	glUseProgram(m_shader->id());

	size_t binding = 1;
	for (size_t i = 0; i < m_inputs.size(); i++) {
		NodeValue& nv = m_inputs[i];

		m_shader->uniformInt<1>(std::format("uIn{}Connected", toCorrectCase(m_inputNames[i])), { nv.connected ? 1 : 0 });

		std::string name = std::format("uIn{}", toCorrectCase(m_inputNames[i]));
		setUniformNodeValue(m_shader.get(), name, nv, binding++);
	}

	for (auto&& [paramName, nv] : m_params) {
		std::string name = std::format("uParam{}", toCorrectCase(paramName));
		setUniformNodeValue(m_shader.get(), name, nv, binding++);
	}

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glDispatchCompute(m_texture->size()[0]/16, m_texture->size()[1]/16, 1);

	m_solved = true;

	m_outputs[0].value[0] = float(m_texture->id());

	NodeValue ret = {
		.value = m_outputs[0].value,
		.type = NodeValueType::image
	};
	return ret;
}

std::string GraphicsNode::processedSource() {
	std::string images = "";
	std::string src = R"(#version 460
layout (local_size_x=16, local_size_y=16) in;
layout (rgba8, binding=0) uniform image2D bOutput;
	
<images>
)";

	size_t binding = 1;
	for (size_t i = 0; i < m_inputs.size(); i++) {
		NodeValue& nv = m_inputs[i];
		auto name = toCorrectCase(m_inputNames[i]);

		switch (nv.type) {
			case NodeValueType::float1: src += std::format("uniform float uIn{};", name); break;
			case NodeValueType::float2: src += std::format("uniform vec2 uIn{};", name); break;
			case NodeValueType::float3: src += std::format("uniform vec3 uIn{};", name); break;
			case NodeValueType::float4: src += std::format("uniform vec4 uIn{};", name); break;
			case NodeValueType::image:
				images += std::format("layout (rgba8, binding={}) uniform image2D uIn{};\n", binding, name);
			break;
		}
		src += std::format("uniform bool uIn{}Connected;", toCorrectCase(name));

		src += "\n";
		binding++;
	}

	src += "\n";

	for (auto&& [ paramName, nv ] : m_params) {
		auto name = toCorrectCase(paramName);

		switch (nv.type) {
			case NodeValueType::float1: src += std::format("uniform float uParam{};", name); break;
			case NodeValueType::float2: src += std::format("uniform vec2 uParam{};", name); break;
			case NodeValueType::float3: src += std::format("uniform vec3 uParam{};", name); break;
			case NodeValueType::float4: src += std::format("uniform vec4 uParam{};", name); break;
			case NodeValueType::image:
				images += std::format("layout (rgba8, binding={}) uniform image2D uParam{};\n", binding, name);
				break;
		}

		src += "\n";
		binding++;
	}

	src.replace(src.find("<images>"), 8, images);

	src += "\n";

	src += R"(
//vec4 Tex(image2D img, vec2 uv) {
//	vec2 sz = vec2(imageSize(img));
//	ivec2 pc = ivec2(uv * sz);
//	return imageLoad(img, pc);
//}

float fmod(float x, float y) {
	return x - y * trunc(x / y);
}

vec2 fmod(vec2 x, float y) {
	return vec2(fmod(x.x, y), fmod(x.y, y));
}

#define Tex(name, uv) imageLoad(name, ivec2(uv * vec2(imageSize(name).xy)))
#define PI 3.141592654

float rand(float n) { return fract(sin(n) * 43758.5453123); }
float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}
<defs>

vec4 mainFunc(vec2 cUV) {
#line 0
	)";

	src.replace(src.find("<defs>"), 6, definitions());

	src += source();
	src += "\n}";

	src += R"(
void main() {
	ivec2 c__Coords = ivec2(gl_GlobalInvocationID.xy);
	vec2 c__uv = vec2(c__Coords) / vec2(gl_NumWorkGroups.xy * 16);
	vec4 pixel = mainFunc(c__uv);
	imageStore(bOutput, c__Coords, pixel);
})";

	return src;
}
