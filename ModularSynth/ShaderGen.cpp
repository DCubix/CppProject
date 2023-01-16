#include "ShaderGen.h"

#include <cctype>
#include <format>
#include <iostream>

#include "Window.h"

// HEAVILY INSPIRED BY https://github.com/UPBGE/upbge/blob/upbge0.2.5/source/blender/gpu/intern/gpu_codegen.c#L701

void ShaderGen::loadLib(const std::string& src) {
	const std::regex identifierName("[a-zA-Z0-9_]");
	StringScanner ss{ src };

	while (ss.peek()) {
		size_t pos = ss.position();
		char c = ss.scan();

		if (std::isalpha(c)) { // check for functions
			std::string retType = c + ss.scanWhile(identifierName);
			ss.skipSpaces();

			std::string identifier = ss.scanWhile(identifierName);
			ss.skipSpaces();

			if (ss.peek() != '(') { // expect function signature, otherwise it's not a function.
				continue;
			}

			ss.scan(); // remove (
			ss.skipSpaces();

			ShaderFunction func{};
			func.name = identifier;
			func.stringIndex = pos;

			// read parameters
			while (ss.peek() != ')') {

				// read param
				std::vector<std::string> paramStr;
				while (ss.peek() != ',' && ss.peek() != ')') {
					paramStr.push_back(ss.scanWhile(identifierName));
					ss.skipSpaces();
				}
				if (ss.peek() == ',') ss.scan(); // remove ,
				ss.skipSpaces();

				ShaderFunctionParam param;

				switch (paramStr.size()) {
					default: continue; break;
					case 2: {
						param.qualifier = ShaderFunctionParam::none;
						for (size_t i = 0; i < std::size(typeStr); i++) {
							if (typeStr[i] == paramStr[0]) {
								param.type = ValueType(i);
								break;
							}
						}
						param.name = paramStr[1];
					} break;
					case 3: { // has qualifier
						param.qualifier = ShaderFunctionParam::none;
						if (paramStr[0] == "in") param.qualifier = ShaderFunctionParam::in;
						else if (paramStr[0] == "out") param.qualifier = ShaderFunctionParam::out;

						for (size_t i = 0; i < std::size(typeStr); i++) {
							if (typeStr[i] == paramStr[1]) {
								param.type = ValueType(i);
								break;
							}
						}
						param.name = paramStr[2];
					} break;
				}

				func.parameters[param.name] = param;
				func.parameterOrder.push_back(param.name);
			}

			ss.skipSpaces();
			ss.scan(); // remove )

			ss.skipSpaces();

			// skip function body
			int braceCount = 0;
			if (ss.scan() == '{') {
				braceCount++;

				while (true) {
					char bc = ss.scan();
					if (bc == '{') braceCount++;
					else if (bc == '}') braceCount--;

					if (braceCount <= 0) break;
				}

				if (ss.peek() == '}') ss.scan();
			}

			func.stringLength = ss.position() - func.stringIndex;
			m_shaderLib[func.name] = func;

			ss.skipSpaces();
		}
	}
}

void ShaderGen::pasteFunction(const std::string& funcName, const std::string& shaderCode) {
	if (std::find(m_pasted.begin(), m_pasted.end(), funcName) != m_pasted.end()) {
		return;
	}

	if (m_shaderLib.find(funcName) == m_shaderLib.end()) { // not found? bleh
		return;
	}

	auto func = m_shaderLib[funcName];
	auto src = shaderCode.substr(func.stringIndex, func.stringLength);

	// check for dependent functions
	const std::regex identifierName("[a-zA-Z0-9_]");
	StringScanner ss{ src };

	while (ss.peek()) {
		size_t pos = ss.position();
		char c = ss.scan();

		if (std::isalpha(c)) { // check for functions
			std::string identifier = c + ss.scanWhile(identifierName);
			ss.skipSpaces();

			if (ss.peek() != '(') { // expect function signature, otherwise it's not a function.
				continue;
			}

			while (ss.peek() != ')' && ss.peek() != 0) ss.scan();
			if (ss.peek() == ')') ss.scan();
			ss.skipSpaces();

			if (ss.peek() == ';') {
				pasteFunction(identifier, shaderCode);
			}
		}
	}

	m_defs += src;
	m_defs += '\n\n';

	m_pasted.push_back(funcName);
}

void ShaderGen::convertType(ValueType from, ValueType to, const std::string& varName) {
	if (from == to) {
		m_body += varName;
	}
	else if (to == ValueType::scalar) {
		switch (from) {
			case ValueType::vec2: m_body += std::format("{}.r", varName); break;
			case ValueType::vec3: m_body += std::format("rgb_to_float({})", varName); break;
			case ValueType::vec4: m_body += std::format("rgba_to_float({})", varName); break;
		}
	}
	else if (to == ValueType::vec2) {
		switch (from) {
			case ValueType::scalar: m_body += std::format("vec2({}, 1.0)", varName, varName); break;
			case ValueType::vec3: m_body += std::format("vec2(rgb_to_float({}), 1.0)", varName, varName); break;
			case ValueType::vec4: m_body += std::format("vec2(rgb_to_float({}), {}.a)", varName, varName); break;
		}
	}
	else if (to == ValueType::vec3) {
		switch (from) {
			case ValueType::scalar: m_body += std::format("vec3({})", varName); break;
			case ValueType::vec2: m_body += std::format("vec3({}.r)", varName); break;
			case ValueType::vec4: m_body += std::format("{}.rgb", varName, varName); break;
		}
	}
	else { // vec4
		switch (from) {
			case ValueType::scalar: m_body += std::format("vec4(vec3({}), 1.0)", varName); break;
			case ValueType::vec2: m_body += std::format("vec4(vec3({}.r), {}.g)", varName, varName); break;
			case ValueType::vec3: m_body += std::format("vec4({}, 1.0)", varName); break;
		}
	}
}

std::string ShaderGen::appendUniform(ValueType type, const std::string& name, size_t binding) {
	if (type == ValueType::image) {
		m_uniforms += std::format("layout (rgba8, binding={}) uniform image2D {};", binding, name);
	}
	else {
		m_uniforms += std::format("uniform {} {};", typeStr[size_t(type)], name);
	}
	m_uniforms += '\n';
	return name;
}

std::string ShaderGen::appendVariable(ValueType type, const std::string& name) {
	if (type == ValueType::image) {
		type = ValueType::vec4;
	}
	m_body += std::format("{} {}", typeStr[size_t(type)], name);
	return name;
}

void ShaderGen::appendUniform(const std::string& str) {
	m_uniforms += str;
}

void ShaderGen::append(const std::string& str) {
	m_body += str;
}

std::string ShaderGen::generate() {
	std::string src = shaderTemplate;
	src.replace(src.find("<uniforms>"), 10, m_uniforms);
	src.replace(src.find("<defs>"), 6, m_defs);
	src.replace(src.find("<body>"), 6, m_body);
	return src;
}
