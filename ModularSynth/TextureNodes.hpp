#pragma once

#include "GraphicsNode.h"
#include "Texture.h"

class ColorNode : public GraphicsNode {
public:
	std::string library() {
		return R"(void gen_color(in vec4 color, out vec4 outColor) {
	outColor = color;
})";
	}

	std::string functionName() { return "gen_color"; }

	std::map<std::string, std::string> parameters() {
		return {
			{ "color", "Color" }
		};
	}

	void onCreate() {
		addOutput("Output", ValueType::vec4);
		addParam("Color", ValueType::vec4);
	}

};

class SimpleGradientNode : public GraphicsNode {
public:
	std::string library() {
		return R"(void gen_simple_gradient(in vec2 uv, float angle, out float res) {
	float c = cos(angle);
	float s = sin(angle);
	mat2 mat = mat2(c, -s, s, c);
	vec2 rotatedUV = (mat * (uv * 2.0 - 1.0)) * 0.5 + 0.5;
	res = clamp(rotatedUV.x, 0.0, 1.0);
})";
	}

	std::string functionName() { return "gen_simple_gradient"; }

	std::map<std::string, std::string> parameters() {
		return {
			{ "uv", "cUV" },
			{ "angle", "Angle" }
		};
	}

	void onCreate() {
		addOutput("Output", ValueType::scalar);
		addParam("Angle", ValueType::scalar);
	}
};

class MixNode : public GraphicsNode {
public:
	std::string library() {
		return R"(void opr_mix_blend(float fac, vec4 ca, vec4 cb, out vec4 outColor) {
	outColor = mix(ca, cb, clamp(fac, 0.0, 1.0));
	outColor.a = ca.a;
}

void opr_mix_add(float fac, vec4 ca, vec4 cb, out vec4 outColor) {
	outColor = mix(ca, ca + cb, clamp(fac, 0.0, 1.0));
	outColor.a = ca.a;
})

void opr_mix_sub(float fac, vec4 ca, vec4 cb, out vec4 outColor) {
	outColor = mix(ca, ca - cb, clamp(fac, 0.0, 1.0));
	outColor.a = ca.a;
})

void opr_mix_mul(float fac, vec4 ca, vec4 cb, out vec4 outColor) {
	outColor = mix(ca, ca * cb, clamp(fac, 0.0, 1.0));
	outColor.a = ca.a;
}

void opr_mix(float fac, float op, vec4 ca, vec4 cb, out vec4 outColor) {
	if (op == 1.0) { // Add
		opr_mix_add(fac, ca, cb, outColor);
	} else if (op == 2.0) { // Sub
		opr_mix_sub(fac, ca, cb, outColor);
	} else if (op == 3.0) { // Multiply
		opr_mix_mul(fac, ca, cb, outColor);
	} else {
		opr_mix_blend(fac, ca, cb, outColor);
	}
}
)";
	}

	std::string functionName() { return "opr_mix"; }

	std::map<std::string, std::string> parameters() {
		return {
			{ "fac", "Factor" },
			{ "op", "Mode" },
			{ "ca", "A" },
			{ "cb", "B" }
		};
	}

	void onCreate() {
		addInput("A", ValueType::vec4);
		addInput("B", ValueType::vec4);
		addInput("Factor", ValueType::scalar);
		addParam("Factor", ValueType::scalar);
		addParam("Mode", ValueType::scalar);
		setParam("Factor", 0.5f);
		setParam("Mode", 0.0f);
		addOutput("Output", ValueType::vec4);
	}

};

class NoiseNode : public GraphicsNode {
public:
	std::string functionName() { return "gen_noise"; }

	std::string library() {
		return R"(void noise(vec2 n, out float res) {
	const vec2 d = vec2(0.0, 1.0);
	vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
	res = mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

void noise(vec2 p, float freq, out float res) {
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
	res = mix(x1, x2, xy.y);
}

void pNoise(vec2 p, int res, out float outValue){
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
	outValue = nf*nf*nf*nf;
}

void iqnoise(vec2 x, float u, float v, out float res) {
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
	
	res = va/wt;
}

void gen_noise(in vec2 uv, float scale, float patternX, float patternY, out float res) {
	iqnoise(uv * scale, patternX, patternY, res);
})";
	}

	std::map<std::string, std::string> parameters() {
		return {
			{ "uv", "cUV" },
			{ "scale", "Scale" },
			{ "patternX", "Pattern X" },
			{ "patternY", "Pattern Y" }
		};
	}

	void onCreate() {
		addParam("Pattern X", ValueType::scalar);
		addParam("Pattern Y", ValueType::scalar);
		addParam("Scale", ValueType::scalar);
		setParam("Scale", 1.0f);
		addOutput("Output", ValueType::scalar);
	}

};

class ThresholdNode : public GraphicsNode {
public:
	std::string library() {
		return R"(void opr_threshold(in vec4 color, float threshold, float feather, out float outValue) {
	float fac = feather / 2.0;
	float e0 = threshold - fac;
	float e1 = threshold + fac;

	float luma = dot(color.rgb, vec3(0.299, 0.587, 0.114));

	outValue = smoothstep(e0, e1, luma) * color.a;
})";
	}

	std::string functionName() { return "opr_threshold"; }

	std::map<std::string, std::string> parameters() {
		return {
			{ "color", "A" },
			{ "threshold", "Threshold" },
			{ "feather", "Feather" }
		};
	}

	void onCreate() {
		addInput("A", ValueType::vec4);
		addParam("Feather", ValueType::scalar);
		addParam("Threshold", ValueType::scalar);
		setParam("Threshold", 0.5f);
		addOutput("Output", ValueType::scalar);
	}

};

#include <fstream>

class ImageNode : public GraphicsNode {
public:
	std::string library() {
		return R"(void gen_image(in vec4 img, out vec4 outColor) {
	outColor = img;
})";
	}

	std::string functionName() { return "gen_image"; }

	std::map<std::string, std::string> parameters() {
		return {
			//{ "uv", "UV" },
			{ "img", "Image" }
		};
	}

	void onCreate() {
		//addInput("UV", ValueType::vec2);
		addParam("Image", ValueType::image);
		addOutput("Output", ValueType::vec4);
	}

	Texture* handle;

};

class UVNode : public GraphicsNode {
public:
	/** TODO: Need a node for this part:
		vec2 s = 1.0 / vec2(imageSize(uInDeform));
        
		float p  = Tex(uInDeform, cUV).x;
		float h1 = Tex(uInDeform, cUV + s * vec2(1.0, 0.0)).x;
		float v1 = Tex(uInDeform, cUV + s * vec2(0.0, 1.0)).x;
      
   		vec2 n = (p - vec2(h1, v1));
	*/

	std::string library() {
		return R"(void mirrored(vec2 v, out vec2 outV) {
	vec2 m = mod(v, 2.0);
	outV = mix(m, 2.0 - m, step(1.0, m));
}

void out_uv(vec2 uv, float repeat, float strength, in vec2 deform, out vec2 duv) {
	duv = uv + ((deform * 2.0 - 1.0) * strength);
	if (repeat == 0.0) { // clamp to edge
		duv = clamp(duv, 0.0, 1.0);
	} else if (repeat == 1.0) { // repeat
		duv = mod(duv, 1.0);
	} else if (repeat == 2.0) { // mirror
		mirrored(duv, duv);
	}
}
)";
	}

	std::string functionName() { return "out_uv"; }

	std::map<std::string, std::string> parameters() {
		return {
			{ "uv", "cUV" },
			{ "repeat", "Repeat" },
			{ "strength", "Strength" },
			{ "deform", "Deform" }
		};
	}

	void onCreate() {
		addInput("Deform", ValueType::vec2);
		addParam("Repeat", ValueType::scalar);
		addParam("Strength", ValueType::scalar);
		setParam("Strength", 1.0f);
		addOutput("Output", ValueType::vec2);
	}

};
