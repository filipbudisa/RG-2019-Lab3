#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 pos;
layout(location = 2) in vec3 normal;

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
    color = vec4(fragColor, 1.0f);
}
