// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: CC0-1.0

#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 outUV;

layout(std430, push_constant) uniform PushConstant {
    mat4 mvp;
};

void main() {
    gl_Position = mvp * vec4(inPosition, 1.0);
    outUV = inUV;
}
