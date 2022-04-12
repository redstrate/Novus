#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outFragPos;

layout(push_constant) uniform PushConstant {
	mat4 mvp;
};

void main() {
    gl_Position = mvp * vec4(inPosition, 1.0);
    outNormal = inNormal;
    outFragPos = inNormal;
}
