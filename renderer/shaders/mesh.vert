#version 450

layout(location = 0) in vec3 inPosition;

layout( push_constant ) uniform PushConstant {
	mat4 mvp;
};

void main() {
    gl_Position = mvp * vec4(inPosition, 1.0);
}
