#pragma once

#include "NodeGraph.h"
#include "GraphicsNode.h"
#include "ShaderGen.h"

#include <format>
#include <fstream>
#include <stack>

class TextureNodeGraph : public NodeGraph {
public:
	void solve() override {
		NodeGraph::solve();

		// TODO: Generate the shader here
		ShaderGen gen{};
		std::string lib = "";

		size_t imgId = 1; // 0 is the final output
		for (size_t i = 0; i < m_nodePath.size(); i++) {
			auto node = dynamic_cast<GraphicsNode*>(get(m_nodePath[i]));

			// load libraries
			lib += node->library();
			lib += "\n";

			//for (size_t i = 0; i < node->inputCount(); i++) {
			//	auto& nv = node->input(i);
			//	if (nv.type == ValueType::image) continue;

			//	// UNIFORM
			//	auto uniName = std::format("in_{}_{}", node->id(), toCamelCase(node->inputName(i)));
			//	gen.appendUniform(nv.type, uniName);
			//}

			// do the same for params
			for (auto& [ paramName, nv ] : node->params()) {
				// UNIFORM
				auto uniName = std::format("param_{}_{}", node->id(), toCamelCase(paramName));
				gen.appendUniform(nv.type, uniName, nv.type == ValueType::image ? (imgId++) : 0);

				// BODY
				if (nv.type == ValueType::image) {
					gen.appendUniform(std::format("uniform bool {}_conn;\n", uniName));

					auto varName = std::format("pix_{}_{}", node->id(), toCamelCase(paramName));
					gen.append("\t");
					gen.appendVariable(ValueType::vec4, varName);
					gen.append(std::format(" = Tex({}, cUV);\n", uniName));
				}
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
		for (size_t i = 0; i < m_nodePath.size(); i++) {
			nodes.push(get(m_nodePath[i]));
		}

		// solve nodes
		while (!nodes.empty()) {
			auto node = static_cast<GraphicsNode*>(nodes.top()); nodes.pop();
			
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
				auto inputParamName = nodeParams[param];
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
						appendComma = checkParams(gen, node, inputParamName, paramOb.type);
					}
				}
				// iii
				else {
					appendComma = checkParams(gen, node, inputParamName, paramOb.type);
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

		std::ofstream of("gen.glsl");
		of << gen.generate();
		of.close();
	}

	bool checkParams(ShaderGen& gen, GraphicsNode* node, const std::string& inputParamName, ValueType paramType) {
		if (node->hasParam(inputParamName)) {
			auto&& nv = node->param(inputParamName);

			std::string varName = "";
			if (nv.type == ValueType::image) {
				varName = std::format("pix_{}_{}", node->id(), toCamelCase(inputParamName));
			}
			else {
				varName = std::format("param_{}_{}", node->id(), toCamelCase(inputParamName));
			}

			gen.convertType(nv.type, paramType, varName);
			return true;
		}
		// iv
		else {
			// TODO: Implement a proper built-in system. We only have the UVs for now
			if (inputParamName == "cUV") {
				gen.convertType(ValueType::vec2, paramType, "cUV");
			}
			else {
				gen.append(std::format("{}(0.0)", typeStr[size_t(paramType)]));
			}
			return true;
		}
	}

};
