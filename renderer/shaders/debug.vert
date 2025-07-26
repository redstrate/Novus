// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: CC0-1.0

#version 450

layout(location = 0) in vec3 inPosition;

layout(std430, push_constant) uniform PushConstant {
	mat4 vp, model;
	vec4 color;
};

void main() {
    vec4 bPos = model * vec4(inPosition, 1.0);

    gl_Position = vp * bPos;
}
