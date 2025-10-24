// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: CC0-1.0

#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV0;
layout(location = 2) in vec2 inUV1;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec4 inBiTangent;
layout(location = 5) in vec4 inColor;
layout(location = 6) in vec4 inBoneWeights;
layout(location = 7) in uvec4 inBoneIds;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outFragPos;
layout(location = 2) out vec2 outUV;

layout(binding = 3) uniform sampler2D diffuseTexture;
layout(binding = 4) uniform sampler2D normalTexture;
layout(binding = 5) uniform sampler2D specularTexture;
layout(binding = 6) uniform sampler2D multiTexture;

layout(std430, push_constant) uniform PushConstant {
	mat4 vp, model;
	int boneOffset;
    int type;
};

layout(std430, binding = 2) buffer readonly BoneInformation {
    mat3x4 bones[256];
};

void main() {
    vec4 bPos = model * vec4(inPosition, 1.0);
    vec4 bNor = vec4(inNormal, 0.0);

    gl_Position = vp * bPos;
    outNormal = bNor.xyz;
    outFragPos = vec3(model * vec4(inPosition, 1.0));
    outUV = inUV0;
}
