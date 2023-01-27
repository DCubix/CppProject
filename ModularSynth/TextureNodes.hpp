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

	GraphicsNodeParams parameters() {
		return {
			{ "color", { "Color", SpecialType::none } }
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

	GraphicsNodeParams parameters() {
		return {
			{ "uv", { "UV", SpecialType::textureCoords } },
			{ "angle", { "Angle", SpecialType::none } }
		};
	}

	void onCreate() {
		addOutput("Output", ValueType::scalar);
		addInput("UV", ValueType::vec2);
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

	GraphicsNodeParams parameters() {
		return {
			{ "fac", { "Factor", SpecialType::none } },
			{ "op", { "Mode", SpecialType::none } },
			{ "ca", { "A", SpecialType::none } },
			{ "cb", { "B", SpecialType::none } }
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

	GraphicsNodeParams parameters() {
		return {
			{ "uv", { "UV", SpecialType::textureCoords } },
			{ "scale", { "Scale", SpecialType::none } },
			{ "patternX", { "Pattern X", SpecialType::none } },
			{ "patternY", { "Pattern Y", SpecialType::none } }
		};
	}

	void onCreate() {
		addInput("UV", ValueType::vec2);
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

	GraphicsNodeParams parameters() {
		return {
			{ "color", { "A", SpecialType::none } },
			{ "threshold", { "Threshold", SpecialType::none } },
			{ "feather", { "Feather", SpecialType::none } }
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

	GraphicsNodeParams parameters() {
		return {
			{ "uv", { "UV", SpecialType::textureCoords } },
			{ "img", { "Image", SpecialType::none } }
		};
	}

	void onCreate() {
		addInput("UV", ValueType::vec2);
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

vec2 op_rep(in vec2 p, float c, vec2 lmax) {
    return p - c * clamp(round(p / c), vec2(0.0), lmax);
}

void out_uv(
	vec2 uvIn, float clampMode, float deformAmt, vec2 deform,

	vec2 repeat, float spacing,

	vec2 pos, vec2 scale, float rot,

	out vec2 duv
) {
	vec2 sz = imageSize(bOutput);
	float s = sin(rot);
	float c = cos(rot);

	vec2 uv = uvIn;
	uv.x -= 0.5;
	uv.x *= sz.x / sz.y;

	mat2 xform =
		mat2(scale.x, 0.0, 0.0, scale.y) *
		mat2(c, -s, s, c);	
	uv *= xform;
	uv += pos;

	uv = op_rep(uv, spacing, repeat);

	uv += ((deform * 2.0 - 1.0) * deformAmt);

	duv = uv;
	if (clampMode == 0.0) { // clamp to edge
		duv = clamp(duv, 0.0, 1.0);
	} else if (clampMode == 1.0) { // repeat
		duv = mod(duv, 1.0);
	} else if (clampMode == 2.0) { // mirror
		mirrored(duv, duv);
	}
}
)";
	}

	std::string functionName() { return "out_uv"; }

	GraphicsNodeParams parameters() {
		return {
			{ "uvIn", { "cUV", SpecialType::none } },
			{ "clampMode", { "Clamp", SpecialType::none } },
			{ "deformAmt", { "Deform Amount", SpecialType::none } },
			{ "deform", { "Deform", SpecialType::none } },
			{ "repeat", { "Repeat", SpecialType::none } },
			{ "spacing", { "Spacing", SpecialType::none } },
			{ "pos", { "Position", SpecialType::none } },
			{ "scale", { "Scale", SpecialType::none } },
			{ "rot", { "Rotation", SpecialType::none } }
		};
	}

	void onCreate() {
		addInput("Deform", ValueType::vec2);

		addParam("Repeat", ValueType::vec2);
		addParam("Spacing", ValueType::scalar);

		addParam("Position", ValueType::vec2);
		addParam("Scale", ValueType::vec2);
		addParam("Rotation", ValueType::scalar);

		addParam("Clamp", ValueType::scalar);

		addParam("Deform Amount", ValueType::scalar);
		setParam("Deform Amount", 1.0f);

		addOutput("Output", ValueType::vec2);
	}

};

class RadialGradientNode : public GraphicsNode {
public:
	std::string library() {
		return R"(void gen_radial_gradient(in vec2 uv, out float res) {
	vec2 center = clamp(uv, 0.0, 1.0) * 2.0 - 1.0;
	res = 1.0 - length(center);
})";
	}

	std::string functionName() { return "gen_radial_gradient"; }

	GraphicsNodeParams parameters() {
		return {
			{ "uv", { "UV", SpecialType::textureCoords } }
		};
	}

	void onCreate() {
		addOutput("Output", ValueType::scalar);
		addInput("UV", ValueType::vec2);
	}
};

class NormalMapNode : public GraphicsNode {
public:
	bool multiPassNode() {
		return true;
	}

	std::string library() {
		return R"(void gen_normal_map(in vec2 uv, float scale, out vec3 res) {
	vec2 step = 1.0 / vec2(imageSize(bOutput));

	float height = rgb_to_float($TREE(uv).rgb);
	float s1 = rgb_to_float($TREE(uv + vec2(step.x, 0.0)).rgb);
	float s2 = rgb_to_float($TREE(uv + vec2(0.0, step.y)).rgb);

	vec2 dxy = height - vec2(s1, s2);

	res = normalize(vec3(dxy * scale / step, 1.0)) * 0.5 + 0.5;
})";
	}

	std::string functionName() { return "gen_normal_map"; }

	GraphicsNodeParams parameters() {
		return {
			{ "uv", { "cUV", SpecialType::none } },
			{ "scale", { "Scale", SpecialType::none } }
		};
	}

	void onCreate() {
		addOutput("Output", ValueType::vec3);
		addInput("Source", ValueType::vec4);
		addParam("Scale", ValueType::scalar);
		setParam("Scale", 0.1f);
	}
};

class OutputNode : public GraphicsNode {
public:
	std::string library() {
		return R"(
void emit_out_$NODE(in vec2 uv, in vec4 color) {
	imageStore(bOutput$NODE, ivec2(uv * vec2(imageSize(bOutput$NODE))), color);
})";
	}

	std::string functionName() { return "emit_out_$NODE"; }

	GraphicsNodeParams parameters() {
		return {
			{ "uv", { "cUV", SpecialType::none } },
			{ "color", { "Color", SpecialType::none } }
		};
	}

	void onCreate() {
		addInput("Color", ValueType::vec4);
		addParam("Size", ValueType::vec2);
		setParam("Size", 512.0f, 512.0f);
	}

	void beginRender(size_t binding = 0) {
		uint32_t width = uint32_t(paramValue("Size")[0]);
		uint32_t height = uint32_t(paramValue("Size")[1]);

		if (!texture) {
			texture = std::unique_ptr<Texture>(new Texture({ width, height }, GL_RGBA32F));
		}
		else {
			if (texture->size()[0] != width || texture->size()[1] != height) {
				texture.reset(nullptr);
				texture = std::unique_ptr<Texture>(new Texture({ width, height }, GL_RGBA32F));
			}
		}

		glBindImageTexture(binding, texture->id(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);	
	}

	void endRender() {
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glDispatchCompute(texture->size()[0] / 16, texture->size()[1] / 16, 1);
	}

	std::unique_ptr<Texture> texture;
};

class CircleShapeNode : public GraphicsNode {
public:
	std::string library() {
		return R"(
void gen_shape_circle(in vec2 uv, float r, out float res) {
	vec2 p = clamp(uv, 0.0, 1.0) * 2.0 - 1.0;
	res = 1.0 - (length(p) - r);
})";
	}

	std::string functionName() { return "gen_shape_circle"; }

	GraphicsNodeParams parameters() {
		return {
			{ "uv", { "UV", SpecialType::textureCoords } },
			{ "r", { "Radius", SpecialType::none } }
		};
	}

	void onCreate() {
		addOutput("Output", ValueType::scalar);
		addInput("UV", ValueType::vec2);
		addParam("Radius", ValueType::scalar);
		setParam("Radius", 0.5f);
	}
};

class BoxShapeNode : public GraphicsNode {
public:
	std::string library() {
		return R"(
void gen_shape_box(in vec2 uv, vec2 b, in vec4 r, out float res) {
	vec2 p = clamp(uv, 0.0, 1.0) * 2.0 - 1.0;

	r.xy = (p.x > 0.0) ? r.xy : r.zw;
    r.x  = (p.y > 0.0) ? r.x  : r.y;
    vec2 q = abs(p) - b + r.x;
    res = 1.0 - (min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r.x);
})";
	}

	std::string functionName() { return "gen_shape_box"; }

	GraphicsNodeParams parameters() {
		return {
			{ "uv", { "UV", SpecialType::textureCoords } },
			{ "b", { "Bounds", SpecialType::none } },
			{ "r", { "Border Radius", SpecialType::none } },
		};
	}

	void onCreate() {
		addOutput("Output", ValueType::scalar);
		addInput("UV", ValueType::vec2);

		addParam("Bounds", ValueType::vec2);
		addParam("Border Radius", ValueType::vec4);

		setParam("Bounds", { 0.5f, 0.5f });
		setParam("Border Radius", { 0.0f, 0.0f, 0.0f, 0.0f });
	}
};
