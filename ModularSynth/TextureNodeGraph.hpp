#pragma once

#include "NodeGraph.h"
#include "GraphicsNode.h"
#include "ShaderGen.h"
#include "Shader.h"
#include "Texture.h"

#include <format>
#include <fstream>
#include <stack>

class TextureNodeGraph : public NodeGraph {
public:
	void solve() override {
		if (m_nodePath.empty()) buildNodePath();

		// TODO: Generate the shader here
		ShaderGen gen{};
		std::string lib = "";

		size_t imgId = 1; // 0 is the final output
		for (size_t i = 0; i < m_nodePath.size(); i++) {
			auto node = dynamic_cast<GraphicsNode*>(get(m_nodePath[i]));

			// load libraries
			lib += node->library();
			lib += "\n";

			// do the same for params
			for (auto& [ paramName, nv ] : node->params()) {
				// UNIFORM
				auto uniName = std::format("param_{}_{}", node->id(), toCamelCase(paramName));
				gen.appendUniform(nv.type, uniName, nv.type == ValueType::image ? (imgId++) : 0);

				// BODY
				/*if (nv.type == ValueType::image) {
					auto varName = std::format("pix_{}_{}", node->id(), toCamelCase(paramName));
					gen.append("\t");
					gen.appendVariable(ValueType::vec4, varName);
					gen.append(std::format(" = Tex({}, cUV);\n", uniName));
				}*/
			}

			// declare outputs
			for (size_t i = 0; i < node->outputCount(); i++) {
				auto& nv = node->output(i);

				auto varName = std::format("out_{}_{}", node->id(), i);

				gen.append("\t");
				gen.appendVariable(nv.type, varName);
				gen.append(";\n");
			}
		}

		gen.loadLib(lib);

		imgId = 1;
		
		/*
		* The nodes are already ordered by execution priority, that is the "node path"
		* While the node stack is not empty:
		*	a. Output the function name to the shader source
		*	b. For each parameter in the function (fetched from the function library for the correct order)
		*		i. Get the input/param name from the parameter map that the node provides
		*		ii. If the parameter is a node input
		*			- is it connected? get the value from the output of the node connected to this input and emit a converted value
		*			- is it not connected? continue to step (iii)
		*		iii. If the parameter is a node param
		*			- emit a converted value
		*		iv.  Otherwise
		*			- check for builtins
		*			- emit a default value otherwise
		*	c. Emit the output parameters
		*/

		// call functions
		std::stack<Node*> nodes;
		for (size_t i = m_nodePath.size(); i-- > 0;) {
			nodes.push(get(m_nodePath[i]));
		}

		Node* lastNode = nullptr;

		// solve nodes
		while (!nodes.empty()) {
			auto node = static_cast<GraphicsNode*>(nodes.top()); nodes.pop();
			if (nodes.size() == 1) {
				lastNode = nodes.top();
			}
			
			// a
			gen.pasteFunction(node->functionName(), lib);
			gen.append(std::format("\t{}(", node->functionName()));

			// b
			auto nodeParams = node->parameters();
			auto fn = gen.getFunction(node->functionName());

			for (auto&& param : fn.parameterOrder) {
				auto paramOb = fn.parameters[param];
				if (paramOb.qualifier == ShaderFunctionParam::out) continue;

				// i
				auto [ inputParamName, sType ] = nodeParams[param];
				bool appendComma = false;

				// ii
				if (node->hasInput(inputParamName)) {
					auto conns = getConnectionsToInput(node, node->inputIndex(inputParamName));
					if (!conns.empty()) { // connected
						// TODO: Consider multiple connections to the same output-input pair, maybe an average?
						//       might give some problems when dealing with different types...
						auto&& con = conns.front();
						auto&& nv = con.source->output(con.sourceOutput);
						gen.convertType(nv.type, paramOb.type, std::format("out_{}_{}", con.source->id(), con.sourceOutput));
						appendComma = true;
					}
					else {
						// iii
						appendComma = checkParams(gen, node, inputParamName, sType, paramOb.type);
					}
				}
				// iii
				else {
					appendComma = checkParams(gen, node, inputParamName, sType, paramOb.type);
				}

				if (appendComma) {
					gen.append(", ");
				}
			}

			// c
			for (size_t i = 0; i < node->outputCount(); i++) {
				auto& nv = node->output(i);
				auto varName = std::format("out_{}_{}", node->id(), i);
				gen.append(varName);
				if (i < node->outputCount() - 1) {
					gen.append(", ");
				}
			}

			gen.append(");\n");
		}

		// output the last node output by default
		if (lastNode) {
			auto varName = std::format("out_{}_{}", lastNode->id(), 0);
			gen.append("\timageStore(bOutput, cCoords, ");
			gen.convertType(lastNode->output(0).type, ValueType::vec4, varName);
			gen.append(");\n");
		}

		std::ofstream of("gen.glsl");
		of << gen.generate();
		of.close();

		if (generatedShader) {
			generatedShader.reset();
		}

		generatedShader = std::make_unique<Shader>();
		generatedShader->add(gen.generate(), GL_COMPUTE_SHADER);
		generatedShader->link();

		render();
	}

	bool checkParams(ShaderGen& gen, GraphicsNode* node, const std::string& inputParamName, SpecialType specialType, ValueType paramType) {
		auto nodeParams = node->parameters();

		if (node->hasParam(inputParamName)) {
			auto&& nv = node->param(inputParamName);

			if (nv.type == ValueType::image) {
				// find a texCoord input
				std::string uvsName = "";
				SpecialType uvsSpecialType = SpecialType::none;
				ValueType uvsType = ValueType::none;

				for (auto [ fnParam, ndParam ] : nodeParams) {
					if (ndParam.second == SpecialType::textureCoords) {
						uvsName = ndParam.first;
						uvsSpecialType = ndParam.second;
						break;
					}
				}

				std::string varName = "cUV";
				if (uvsSpecialType != SpecialType::none) {
					auto conns = getConnectionsToInput(node, node->inputIndex(uvsName));
					if (!conns.empty()) { // connected
						auto&& con = conns.front();
						auto&& nv = con.source->output(con.sourceOutput);
						varName = std::format("out_{}_{}", con.source->id(), con.sourceOutput);
						uvsType = nv.type;
					}
				}

				gen.append(std::format("Tex(param_{}_{}, ", node->id(), toCamelCase(inputParamName)));
				if (uvsType != ValueType::none) {
					gen.convertType(uvsType, ValueType::vec2, varName);
					gen.append(")");
				}
				else {
					gen.append("cUV)");
				}
			}
			else {
				std::string varName = std::format("param_{}_{}", node->id(), toCamelCase(inputParamName));
				gen.convertType(nv.type, paramType, varName);
			}

			return true;
		}
		// iv
		else {
			// TODO: Implement a proper built-in system. We only have the UVs for now
			if (inputParamName == "cUV") {
				gen.convertType(ValueType::vec2, paramType, "cUV");
			}
			else {
				// find a texCoord input
				std::string uvsName = "";
				SpecialType uvsSpecialType = SpecialType::none;
				ValueType uvsType = ValueType::none;

				for (auto [fnParam, ndParam] : nodeParams) {
					if (ndParam.second == SpecialType::textureCoords) {
						uvsName = ndParam.first;
						uvsSpecialType = ndParam.second;
						break;
					}
				}

				if (uvsSpecialType != SpecialType::none) {
					auto conns = getConnectionsToInput(node, node->inputIndex(uvsName));
					if (!conns.empty()) { // connected
						auto&& con = conns.front();
						auto&& nv = con.source->output(con.sourceOutput);
						gen.convertType(nv.type, ValueType::vec2, std::format("out_{}_{}", con.source->id(), con.sourceOutput));
					}
					else {
						gen.append("cUV");
					}
				}
				else {
					gen.append(std::format("{}(0.0)", typeStr[size_t(paramType)]));
				}
			}
			return true;
		}
	}

	std::unique_ptr<Shader> generatedShader;
	std::unique_ptr<Texture> output;

	void render(uint32_t width = 1024, uint32_t height = 1024) {
		if (!generatedShader) return;

		if (!output) {
			output = std::unique_ptr<Texture>(new Texture({ width, height, 0 }, GL_RGBA32F));
		}

		glBindImageTexture(0, output->id(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glUseProgram(generatedShader->id());

		setUniforms();

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glDispatchCompute(output->size()[0] / 16, output->size()[1] / 16, 1);

		glUseProgram(0);
	}

private:
	void setUniform(const std::string& name, const NodeValue& nv, size_t index) {
		auto shader = generatedShader.get();
		switch (nv.type) {
			case ValueType::scalar: shader->uniform<1>(name, { nv.value[0] }); break;
			case ValueType::vec2: shader->uniform<2>(name, { nv.value[0], nv.value[1] }); break;
			case ValueType::vec3: shader->uniform<3>(name, { nv.value[0], nv.value[1], nv.value[2] }); break;
			case ValueType::vec4: shader->uniform<4>(name, nv.value); break;
			case ValueType::image: {
				glBindImageTexture(index, GLuint(nv.value[0]), 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
			} break;
		}
	}

	void setNodeUniforms(GraphicsNode* node, size_t& binding) {
		for (auto& [paramName, nv] : node->params()) {
			auto uniName = std::format("param_{}_{}", node->id(), toCamelCase(paramName));
			
			// UNIFORM
			setUniform(uniName, nv, binding);

			// BODY
			if (nv.type == ValueType::image) {
				binding++;
			}
			
		}
	}

	void setUniforms() {
		size_t binding = 1;
		for (size_t i = 0; i < m_nodePath.size(); i++) {
			auto node = static_cast<GraphicsNode*>(get(m_nodePath[i]));
			setNodeUniforms(node, binding);
		}
	}

};
