// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: CC0-1.0

#version 450

layout (location = 0) out vec2 out_uv;

layout(push_constant) uniform PushConstant{
    mat4 invProjectionMatrix, invViewMatrix;
    vec4 sun_position_fov;
};

void main() {
    out_uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(out_uv * 2.0f + -1.0f, 1.0f, 1.0f);
}
