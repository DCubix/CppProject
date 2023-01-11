#version 460
layout (local_size_x=1, local_size_y=1) in;
layout (rgba8, binding=0) uniform image2D bOutput;
	


uniform float uParamAngle;


//vec4 Tex(image2D img, vec2 uv) {
//	vec2 sz = vec2(imageSize(img));
//	ivec2 pc = ivec2(uv * sz);
//	return imageLoad(img, pc);
//}

#define Tex(name, uv) imageLoad(name, ivec2(uv * vec2(imageSize(name).xy)))


vec4 mainFunc(vec2 cUV) {
#line 0
	
			float c = cos(uParamAngle);
			float s = sin(uParamAngle);
			mat2 mat = mat2(c, -s, s, c);
			vec4 ca = vec4(0.0, 0.0, 0.0, 1.0);
			vec4 cb = vec4(1.0, 1.0, 1.0, 1.0);
			vec2 rotatedUV = (mat * (cUV * 2.0 - 1.0)) * 0.5 + 0.5;
			return mix(ca, cb, clamp(rotatedUV.x, 0.0, 1.0));
		
}
void main() {
	ivec2 c__Coords = ivec2(gl_GlobalInvocationID.xy);
	vec2 c__uv = vec2(c__Coords) / vec2(gl_NumWorkGroups.xy);
	vec4 pixel = mainFunc(c__uv);
	imageStore(bOutput, c__Coords, pixel);
}