// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: CC0-1.0

#version 460 core

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D boundSampler;

void main() {
    outColor = inColor * texture(boundSampler, inUV);
}
