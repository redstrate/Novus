#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inBoneWeights;
layout(location = 4) in uvec4 inBoneIds;

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
    mat4 bones[128];
};

void main() {
    mat4 BoneTransform = bones[boneOffset + inBoneIds[0]] * inBoneWeights[0];
    BoneTransform += bones[boneOffset + inBoneIds[1]] * inBoneWeights[1];
    BoneTransform += bones[boneOffset + inBoneIds[2]] * inBoneWeights[2];
    BoneTransform += bones[boneOffset + inBoneIds[3]] * inBoneWeights[3];

    BoneTransform = model * BoneTransform;

    vec4 bPos = BoneTransform * vec4(inPosition, 1.0);
    vec4 bNor = BoneTransform * vec4(inNormal, 0.0);

    gl_Position = vp * bPos;
    outNormal = bNor.xyz;
    outFragPos = vec3(model * vec4(inPosition, 1.0));
    outUV = inUV;
}
