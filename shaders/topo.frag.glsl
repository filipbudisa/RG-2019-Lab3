#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 pos;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texCoord;

layout(location = 0) out vec4 color;

struct Light {
	vec3 pos;
	vec3 color;
};

layout(set = 1, binding = 0) uniform UniformBufferObject {
	Light[20] lights;
    vec3 eyePos;
} lights;

void main() {
	const vec3 sun = lights.lights[0].pos;
	const vec3 sunColor = lights.lights[0].color;

	vec3 global = 0.1 * fragColor;

	vec3 sunPoint = normalize(sun - pos);
	float angle = max(0, dot(sunPoint, normal) / (length(sunPoint) * length(normal)));
	vec3 dif = fragColor * 0.8 * angle;

	//vec3 spec = vec3(1, 0.9, 0.7) * 0.2 * pow(cos(), 2);

    //outColor = vec4(fragColor, 1.0);
    color = vec4(global + dif, 1.0f);
}
