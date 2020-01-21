#version 450
#extension GL_ARB_separate_shader_objects : enable
#define time 0.001

struct Vertex {
    vec3 pos;
    vec3 color;
    vec3 normal;
    vec2 texCoord;
};

struct MassPoint {
    vec4 force;
    vec4 velocity;
    bool pinned;
};

struct Spring {
    uint first;
    uint second;
    float k;
    float len;
};

layout(set = 0, binding = 0) buffer MassPointData {
    MassPoint points[];
} mpData;

layout(set = 1, binding = 0) buffer VertexData {
    Vertex vertices[];
} vData;

layout(set = 2, binding = 0) buffer SpringData {
    Spring springs[];
} sData;

void main() {
    Spring spring = sData.springs[gl_GlobalInvocationID.x];
    Vertex vecA = vData.vertices[spring.first];
    Vertex vecB = vData.vertices[spring.second];

    vec3 direction = vec3(vecA.pos) - vec3(vecB.pos);

    float currentLength = length(direction);
    float force = spring.k * (currentLength - spring.len);

    direction = normalize(direction);

    mpData.points[spring.first].force += vec4(direction * (force / -2.0), 0);
    mpData.points[spring.second].force += vec4(direction * (force / 2.0), 0);
}