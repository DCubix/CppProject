#pragma once

#include "GraphicsNode.h"

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

class SimpleGradientNode : public GraphicsNode {
public:
	std::string source() {
		return R"(
			float c = cos(uParamAngle);
			float s = sin(uParamAngle);
			mat2 mat = mat2(c, -s, s, c);
			vec4 ca = vec4(0.0, 0.0, 0.0, 1.0);
			vec4 cb = vec4(1.0, 1.0, 1.0, 1.0);
			vec2 rotatedUV = (mat * (cUV * 2.0 - 1.0)) * 0.5 + 0.5;
			return mix(ca, cb, clamp(rotatedUV.x, 0.0, 1.0));
		)";
	}

	void onCreate() {
		// TODO: How to maintain the same size accross nodes?
		outputWidth = 512;
		outputHeight = 512;
		addParam("Angle", NodeValueType::float1);
	}
};

class MixNode : public GraphicsNode {
public:
	std::string source() {
		return R"(
			vec4 va = Tex(uInA, cUV);
			vec4 vb = Tex(uInB, cUV);
			float vf = uParamFactor;
			if (uInFactorConnected) {
				vf *= dot(Tex(uInFactor, cUV).rgb, vec3(0.299, 0.587, 0.114));
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
		setParam("Factor", 0.5f);
	}

};
