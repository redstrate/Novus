// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: CC0-1.0

#version 450

#include "atmosphere.glsl"

layout (location = 0) in vec2 in_uv;

layout (location = 0) out vec4 out_color;

layout(push_constant) uniform PushConstant{
    mat4 invProjectionMatrix, invViewMatrix;
    vec4 sun_position_fov;
};

vec3 sky_ray() {
    vec2 px_nds = in_uv * 2. - 1.;
    vec3 point_nds = vec3(px_nds, -1.);
    vec4 point_ndsh = vec4(point_nds, 1.0);
    vec4 eye = invProjectionMatrix * point_ndsh;
    eye.w = 0.;
    vec3 dirWorld = (invViewMatrix * eye).xyz;
    return normalize(dirWorld);
}

void main() {
    const vec3 color = atmosphere(
        // ray direction
        sky_ray(),
        // ray origin
        vec3(0, 6372e3, 0),
        // position of the sun in world space (this will be normalized)
        sun_position_fov.xyz,
        // intensity of the sun
        22.0,
        // radius of the plant in meters
        6371e3,
        // radius of the atmosphere in meters
        6471e3,
        // Rayleigh scattering coefficient
        vec3(5.5e-6, 13.0e-6, 22.4e-6),
        // Mie scattering coefficient
        21e-6,
        // Rayleigh scale height
        8e3,
        // Mie scale height
        1.2e3,
        // Mie preferred scattering direction
        0.758
    );

    out_color = vec4(color, 1.0);
}
