// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: CC0-1.0

#version 450

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inFragPos;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec4 outColor;

layout(binding = 3) uniform sampler2D diffuseTexture;
layout(binding = 4) uniform sampler2D normalTexture;
layout(binding = 5) uniform sampler2D specularTexture;
layout(binding = 6) uniform sampler2D multiTexture;

layout(std430, push_constant) uniform PushConstant {
    mat4 vp, model;
    int boneOffset;
    int type;
};

void main() {
    const vec3 lightPos = vec3(5);

    vec3 diffuse;
    if (textureSize(diffuseTexture, 0).x == 1) {
        diffuse = vec3(1);
    } else {
        diffuse = texture(diffuseTexture, inUV).rgb;
    }
    if(type == 1) {
        const float skinInfluence = texture(specularTexture, inUV).r;
        vec3 skinColor = vec3(250 / 255.0, 199 / 255.0, 166 / 255.0);

        diffuse = mix(texture(diffuseTexture, inUV).rgb, skinColor, 1.0);
    }

    vec3 norm = normalize(inNormal);
    vec3 lightDir = normalize(lightPos - inFragPos);

    float diff = max(dot(norm, lightDir), 0.0);

    outColor = vec4(diffuse * (diff + 0.5), 1.0);
}
