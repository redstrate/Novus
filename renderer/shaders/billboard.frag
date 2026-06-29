// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: CC0-1.0

#version 450

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

layout(std430, push_constant) uniform PushConstant{
    mat4 mvp;
    mat4 view;
    vec4 color;
    vec4 scale;
};

layout (binding = 1) uniform sampler2D colorSampler;

void main() {
    if(inUV.y < 0.0 || inUV.x > 1.0)
        discard;

    outColor = texture(colorSampler, inUV) * color;
}
