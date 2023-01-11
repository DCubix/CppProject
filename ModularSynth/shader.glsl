#version 460
layout (local_size_x=1, local_size_y=1) in;
layout (rgba8, binding=0) uniform image2D bOutput;
	
layout (rgba8, binding=1) uniform image2D uInA;
layout (rgba8, binding=2) uniform image2D uInB;
layout (rgba8, binding=3) uniform image2D uInFactor;

uniform bool uInAConnected;
uniform bool uInBConnected;
uniform bool uInFactorConnected;

uniform float uParamFactor;


//vec4 Tex(image2D img, vec2 uv) {
//	vec2 sz = vec2(imageSize(img));
//	ivec2 pc = ivec2(uv * sz);
//	return imageLoad(img, pc);
//}

#define Tex(name, uv) imageLoad(name, ivec2(uv * vec2(imageSize(name).xy)))


vec4 mainFunc(vec2 cUV) {
#line 0
	
			vec4 va = Tex(uInA, cUV);
			vec4 vb = Tex(uInB, cUV);
			float vf = uParamFactor;
			if (uInFactorConnected) {
				vf *= dot(Tex(uInFactor, cUV).rgb, vec3(0.299, 0.587, 0.114));
			}
			return mix(va, vb, vf);
		
}
void main() {
	ivec2 c__Coords = ivec2(gl_GlobalInvocationID.xy);
	vec2 c__uv = vec2(c__Coords) / vec2(gl_NumWorkGroups.xy);
	vec4 pixel = mainFunc(c__uv);
	imageStore(bOutput, c__Coords, pixel);
}