#pragma once

#include <string>
#include <unordered_map>
#include <regex>
#include <stack>

#include "NodeGraph.h"

const std::string typeStr[] = {
	"", "float", "vec2", "vec3", "vec4", "image2D"
};

const std::string shaderTemplate = R"(#version 460
layout (local_size_x=16, local_size_y=16) in;

<uniforms>

float fmod(float x, float y) {
	return x - y * trunc(x / y);
}

vec2 fmod(vec2 x, float y) {
	return vec2(fmod(x.x, y), fmod(x.y, y));
}

vec3 hash3(vec2 p) {
	vec3 q = vec3(dot(p,vec2(127.1,311.7)), 
					dot(p,vec2(269.5,183.3)), 
					dot(p,vec2(419.2,371.9)));
	return fract(sin(q)*43758.5453);
}

#define Tex(name, uv) imageLoad(name, ivec2(uv * vec2(imageSize(name).xy)))
#define TexP(name, uv, ox, oy) imageLoad(name, ivec2(uv * vec2(imageSize(name).xy)) + ivec2(ox, oy))
#define PI 3.141592654

float rand(float n) { return fract(sin(n) * 43758.5453123); }
float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float rgb_to_float(vec3 color) {
	return color.r * 0.2126 + color.g * 0.7152 + color.b * 0.0722;
}

float rgba_to_float(vec4 color) {
	return rgb_to_float(color.rgb) * color.a;
}

<defs>

void main() {
	ivec2 cCoords = ivec2(gl_GlobalInvocationID.xy);
	vec2 cUV = vec2(cCoords) / vec2(gl_NumWorkGroups.xy * 16);
	
<body>
}
)";

struct ShaderFunctionParam {
	ValueType type;
	enum {
		none = 0,
		in,
		out
	} qualifier;
	std::string name;
};

struct ShaderFunction {
	std::string name;
	std::unordered_map<std::string, ShaderFunctionParam> parameters;
	std::vector<std::string> parameterOrder;
	size_t stringIndex{ 0 }, stringLength{ 0 };
};

class ShaderGen {
public:
	enum class Target {
		body = 0,
		definitions,
		uniforms
	};

	void loadLib(const std::string& src);

	void beginCodeBlock();
	void endCodeBlock(Target target);
	
	void beginFunctionBlock(const std::string& signature);
	void endFunctionBlock(Target target);

	void pasteFunction(const std::string& funcName, const std::string& shaderCode);
	std::string appendUniform(ValueType type, const std::string& name, size_t binding = 0);

	void append(const std::string& str);
	std::string appendVariable(ValueType type, const std::string& name);
	void convertType(ValueType from, ValueType to, const std::string& varName);
	void indent();

	std::string generate();

	const ShaderFunction& getFunction(const std::string& name) { return m_shaderLib[name]; }
	std::string& target(Target target) { return m_targets[target]; }

protected:
	std::unordered_map<Target, std::string> m_targets;
	std::stack<std::string> m_userCodeBlocks;

	size_t m_tmpIndex{ 0 };

	std::unordered_map<std::string, ShaderFunction> m_shaderLib;
	std::vector<std::string> m_pasted;


};

template <typename Char = char>
class StringScanner {
public:
	using Str = std::basic_string<Char>;
	using RegExp = std::basic_regex<Char>;

	StringScanner() = default;
	StringScanner(const Str& input) {
		m_data = std::vector<Char>(input.begin(), input.end());
	}

	Char peek(size_t offset = 0) const { return m_data.empty() ? '\0' : m_data[offset]; }
	Char scan() {
		if (m_data.empty()) return '\0';
		Char tmp = m_data.front();
		m_data.erase(m_data.begin());
		m_position++;
		return tmp;
	}

	std::string scanWhile(RegExp re) {
		std::string ret = "";
		while (peek() != '\0' && std::regex_match(Str(1, peek()), re)) {
			ret += scan();
		}
		return ret;
	}

	std::string peekWhile(RegExp re) {
		std::string ret = "";
		size_t offset = 0;
		while (peek(offset) != '\0' && std::regex_match(Str(1, peek(offset)), re)) {
			ret += peek(offset++);
		}
		return ret;
	}

	void skipSpaces() {
		scanWhile(std::basic_regex<Char>("\\s"));
	}

	size_t position() const { return m_position; }

private:
	std::vector<Char> m_data;
	size_t m_position{ 0 };
};
