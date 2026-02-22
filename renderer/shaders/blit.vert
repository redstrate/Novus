// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: CC0-1.0

#version 460 core

layout (location = 0) out vec2 outUV;

void main() {
    vec2 pos = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    outUV = pos;
    // NOTE: This weird X flip is to make zones not appear flipped. I don't know why they are like this.
    outUV.x *= -1.0f;
    gl_Position = vec4(pos * 2.0f + -1.0f, 0.0f, 1.0f);
}
