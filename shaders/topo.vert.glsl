#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 position;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec2 texCoord;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout( push_constant ) uniform UniformTransformObject {
	mat4 transform;
	mat4 nTransform;
} uniformTransform;

void main() {
	vec4 pos = uniformTransform.transform * vec4(inPosition, 1);

    gl_Position = ubo.proj * ubo.view  * pos;
    gl_Position[2] /= 5.0;

    fragColor = vec3(inColor);
    position = vec3(pos) / pos[3];

    vec4 fragNormalH = uniformTransform.nTransform * vec4(normal, 1);
    fragNormal = normalize(vec3(fragNormalH) / fragNormalH[3]);

    texCoord = inTexCoord;
}
