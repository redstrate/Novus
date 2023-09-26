// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: CC0-1.0

#version 460 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;

layout(push_constant) uniform PushConstants {
    vec2 scale, translate;
} pushConstants;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 outColor;

void main() {
    gl_Position = vec4(inPos * pushConstants.scale + pushConstants.translate, 0.0, 1.0);
    outUV = inUV;
    outColor = inColor;
}
