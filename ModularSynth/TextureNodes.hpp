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

			if (uParamMode == 1.0) { // Add
				return vec4(va.rgb + (vb.rgb * vf), 1.0);
			} else if (uParamMode == 2.0) { // Sub
				return clamp(vec4(va.rgb - (vb.rgb * vf), 1.0), vec4(0.0), vec4(1.0));
			} else if (uParamMode == 3.0) { // Multiply
				return vec4(va.rgb * (vb.rgb * vf), 1.0);
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
		addParam("Mode", NodeValueType::float1);
		setParam("Factor", 0.5f);
		setParam("Mode", 0.0f);
	}

};

class NoiseNode : public GraphicsNode {
public:
	std::string source() {
		return R"(
			return vec4(vec3(iqnoise(uParamScale * cUV, uParamPatternX, uParamPatternY)), 1.0);
		)";
	}

	std::string definitions() override {
		return R"(
		float noise(vec2 n) {
			const vec2 d = vec2(0.0, 1.0);
			vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
			return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
		}

		float noise(vec2 p, float freq) {
			float unit = 1.0 / freq;
			vec2 ij = floor(p / unit);
			vec2 xy = mod(p, unit) / unit;
			xy = 0.5*(1.-cos(PI*xy));
			float a = rand((ij+vec2(0.0,0.0)));
			float b = rand((ij+vec2(1.0,0.0)));
			float c = rand((ij+vec2(0.0,1.0)));
			float d = rand((ij+vec2(1.0,1.0)));
			float x1 = mix(a, b, xy.x);
			float x2 = mix(c, d, xy.x);
			return mix(x1, x2, xy.y);
		}

		float pNoise(vec2 p, int res){
			float persistance = 0.5;
			float n = 0.0;
			float normK = 0.0;
			float f = 4.0;
			float amp = 1.0;
			int iCount = 0;
			for (int i = 0; i < 50; i++){
				n+=amp*noise(p, f);
				f*=2.;
				normK+=amp;
				amp*=persistance;
				if (iCount == res) break;
				iCount++;
			}
			float nf = n/normK;
			return nf*nf*nf*nf;
		}

		vec3 hash3(vec2 p) {
			vec3 q = vec3(dot(p,vec2(127.1,311.7)), 
						  dot(p,vec2(269.5,183.3)), 
						  dot(p,vec2(419.2,371.9)));
			return fract(sin(q)*43758.5453);
		}

		float iqnoise(vec2 x, float u, float v) {
			vec2 p = floor(x);
			vec2 f = fract(x);
		
			float k = 1.0+63.0*pow(1.0-v,4.0);
	
			float va = 0.0;
			float wt = 0.0;
			for( int j=-2; j<=2; j++ )
			for( int i=-2; i<=2; i++ )
			{
				vec2 g = vec2( float(i),float(j) );
				vec3 o = hash3( p + g )*vec3(u,u,1.0);
				vec2 r = g - f + o.xy;
				float d = dot(r,r);
				float ww = pow( 1.0-smoothstep(0.0,1.414,sqrt(d)), k );
				va += o.z*ww;
				wt += ww;
			}
	
			return va/wt;
		}

		)";
	}

	void onCreate() {
		// TODO: How to maintain the same size accross nodes?
		outputWidth = 512;
		outputHeight = 512;

		addParam("Pattern X", NodeValueType::float1);
		addParam("Pattern Y", NodeValueType::float1);
		addParam("Scale", NodeValueType::float1);
		setParam("Scale", 1.0f);
	}

};

class ThresholdNode : public GraphicsNode {
public:
	std::string source() {
		return R"(
			float fac = uParamFeather / 2.0;
			float e0 = uParamThreshold - fac;
			float e1 = uParamThreshold + fac;

			vec4 col = Tex(uInA, cUV);
			float luma = dot(col.rgb, vec3(0.299, 0.587, 0.114));

			return vec4(vec3(smoothstep(e0, e1, luma)), col.a);
		)";
	}

	void onCreate() {
		// TODO: How to maintain the same size accross nodes?
		outputWidth = 512;
		outputHeight = 512;

		addInput("A", NodeValueType::image);
		addParam("Feather", NodeValueType::float1);
		addParam("Threshold", NodeValueType::float1);
		setParam("Threshold", 0.5f);
	}

};