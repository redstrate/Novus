// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: CC0-1.0

#version 450

layout (location = 0) out vec2 outUV;

layout(std430, push_constant) uniform PushConstant{
    mat4 mvp;
    mat4 view;
    vec4 color;
    vec4 scale;
};

void main() {
    vec2 p = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    outUV = vec2(1.0 - p.x, p.y);

    p = p * 2.0f + -1.0f;
    p *= scale.xy;

    const vec3 right = {view[0][0], view[1][0], view[2][0]};
    const vec3 up = {view[0][1], view[1][1], view[2][1]};

    vec3 position = right * p.x * 0.25 + up * p.y * 0.25;

    gl_Position = mvp * vec4(position, 1.0);
}

