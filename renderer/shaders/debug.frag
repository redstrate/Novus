// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: CC0-1.0

#version 450

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec4 outColor;

layout(std430, push_constant) uniform PushConstant {
	mat4 vp, model;
	vec4 color;
	vec4 discardCubeLines;
};

void main() {
    if (discardCubeLines.x == 1.0 && length(vec3(notEqual(abs(inPosition), vec3(1.0)))) > 1.0) {
        discard;
    } else {
        outColor = color;
    }
}
