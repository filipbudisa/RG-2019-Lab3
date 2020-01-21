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
    vec3 force;
    vec3 velocity;
    bool pinned;
};

layout(set = 0, binding = 0) buffer MassPointData {
    MassPoint points[];
} mpData;

layout(set = 1, binding = 0) buffer VertexData {
    Vertex vertices[];
} vData;

void main() {
    if(mpData.points[gl_GlobalInvocationID.x].pinned) return;

    // damping
    //mpData.points[gl_GlobalInvocationID.x].force += mpData.points[gl_GlobalInvocationID.x].velocity * -3.0f;

    // gravity
    mpData.points[gl_GlobalInvocationID.x].force = vec3(0, 0, -9.81);

    // velocity
    mpData.points[gl_GlobalInvocationID.x].velocity += mpData.points[gl_GlobalInvocationID.x].force * time;

    // position
    vData.vertices[gl_GlobalInvocationID.x].pos += mpData.points[gl_GlobalInvocationID.x].velocity * time;

    // force reset
    mpData.points[gl_GlobalInvocationID.x].force = vec3(0, 0, 0);
}