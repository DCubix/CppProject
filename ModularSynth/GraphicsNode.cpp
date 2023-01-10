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

	addOutput("Output", NodeValueType::image);

	std::cout << processedSource() << std::endl;
}

static void setUniformNodeValue(Shader* shader, const std::string& name, const NodeValue& nv, size_t index) {
	switch (nv.type) {
		case NodeValueType::float1: shader->uniform<1>(name, { nv.value[0] }); break;
		case NodeValueType::float2: shader->uniform<2>(name, { nv.value[0], nv.value[1] }); break;
		case NodeValueType::float3: shader->uniform<3>(name, { nv.value[0], nv.value[1], nv.value[2] }); break;
		case NodeValueType::float4: shader->uniform<4>(name, nv.value); break;
		case NodeValueType::image: {
			glBindImageTexture(index, GLuint(nv.value[0]), 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
			shader->uniformInt<1>(name, { int(index) });
		} break;
	}
}

NodeValue GraphicsNode::solve() {
	glBindImageTexture(0, m_texture->id(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glUseProgram(m_shader->id());

	for (size_t i = 0; i < m_inputs.size(); i++) {
		NodeValue& nv = m_inputs[i];
		std::string name = std::format("uIn{}", toCorrectCase(m_inputNames[i]));
		setUniformNodeValue(m_shader.get(), name, nv, i+2);
	}

	size_t i = 0;
	for (auto&& [paramName, nv] : m_params) {
		std::string name = std::format("uParam{}", toCorrectCase(paramName));
		setUniformNodeValue(m_shader.get(), name, nv, i+2);
		i++;
	}

	glDispatchCompute(m_texture->size()[0], m_texture->size()[1], 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

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
	layout (local_size_x=1, local_size_y=1) in;
	layout (rgba32f, binding=0) uniform image2D bOutput;
	
	<images>

	)";

	for (size_t i = 0; i < m_inputs.size(); i++) {
		NodeValue& nv = m_inputs[i];
		auto name = toCorrectCase(m_inputNames[i]);

		switch (nv.type) {
			case NodeValueType::float1: src += std::format("uniform float uIn{}", name); break;
			case NodeValueType::float2: src += std::format("uniform vec2 uIn{}", name); break;
			case NodeValueType::float3: src += std::format("uniform vec3 uIn{}", name); break;
			case NodeValueType::float4: src += std::format("uniform vec4 uIn{}", name); break;
			case NodeValueType::image:
				images += std::format("layout (rgba32f, binding={}) uniform image2D uIn{};", i+2, name);
				src += std::format("uniform bool uIn{}Connected;\n", toCorrectCase(name));
			break;
		}

		src += "\n";
	}

	src += "\n";

	size_t i = 0;
	for (auto&& [ paramName, nv ] : m_params) {
		auto name = toCorrectCase(paramName);

		switch (nv.type) {
			case NodeValueType::float1: src += std::format("uniform float uParam{}", name); break;
			case NodeValueType::float2: src += std::format("uniform vec2 uParam{}", name); break;
			case NodeValueType::float3: src += std::format("uniform vec3 uParam{}", name); break;
			case NodeValueType::float4: src += std::format("uniform vec4 uParam{}", name); break;
			case NodeValueType::image:
				images += std::format("layout (rgba32f, binding={}) uniform image2D uParam{};", i + 2, name);
				src += std::format("uniform bool uParam{}Connected;\n", toCorrectCase(name));
				break;
		}

		src += "\n";
		i++;
	}

	src.replace(src.find("<images>"), 8, images);

	src += "\n";

	src += R"(
	vec4 Sample(image2D img, vec2 uv) {
		return imageLoad(img, ivec2(uv * imageSize(img)));
	}

	vec4 mainFunc(vec2 cUV) {
	#line 1
	)";

	src += source();
	src += "\n}";

	src += R"(
	void main() {
		ivec2 c__Coords = ivec2(gl_GlobalInvocationID.xy);
		vec2 c__uv = vec2(c__Coords) / vec2(gl_NumWorkGroups.xy);
		vec4 pixel = mainFunc(c__uv);
		imageStore(bOutput, c__Coords, pixel);
	})";

	return src;
}
