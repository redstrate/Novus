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

const int MAX_LIGHTS = 1024; // NOTE: Keep in sync with SimpleRenderer's MAX_LIGHTS!

// NOTE: Keep in sync with Physis!
const int LIGHT_TYPE_DIRECTIONAL = 1;
const int LIGHT_TYPE_POINT = 2;
const int LIGHT_TYPE_SPOT = 3;

// To make lights less intense.
const float INTENSITY_FACTOR = 0.5;

struct Light {
    vec4 directionOrPos;
    vec4 colorIntensity;
};

layout(binding = 7) buffer readonly LightInformation {
    Light lights[MAX_LIGHTS];
};

layout(std430, push_constant) uniform PushConstant {
    mat4 vp, model;
    int boneOffset;
    int type;
    vec3 viewPos;
};

void main() {
    vec3 diffuse;
    if (textureSize(diffuseTexture, 0).x == 1) {
        diffuse = vec3(1);
    } else {
        vec4 tex = texture(diffuseTexture, inUV);
        diffuse = tex.rgb;
        // TODO: use alpha threshold from the material
        if (tex.a <= 0.1) {
            discard;
        }
    }
    if(type == 1) {
        const float skinInfluence = texture(specularTexture, inUV).r;
        vec3 skinColor = vec3(250 / 255.0, 199 / 255.0, 166 / 255.0);

        diffuse = mix(texture(diffuseTexture, inUV).rgb, skinColor, 1.0);
    }

    vec3 norm = normalize(inNormal);

    vec3 lightFactor = vec3(0.25);
    vec3 specular = vec3(0);
    for (int i = 0; i < MAX_LIGHTS; i++) {
        int lightType = int(lights[i].directionOrPos.w);
        if (lightType == LIGHT_TYPE_DIRECTIONAL) {
            vec3 lightDir = -lights[i].directionOrPos.xyz;

            float diff = max(dot(norm, lightDir), 0.0);
            lightFactor += lights[i].colorIntensity.rgb * diff * lights[i].colorIntensity.a * INTENSITY_FACTOR;
        } else if (lightType == LIGHT_TYPE_POINT) {
            vec3 lightDir = normalize(lights[i].directionOrPos.xyz - inFragPos);

            float diff = max(dot(norm, lightDir), 0.0);
            float constant = 1.0f;
            float linear = 0.09f;
            float quadratic = 0.032f;
            float distance = length(lights[i].directionOrPos.xyz - inFragPos);
            float attenuation = 1.0 / (constant + linear * distance +
                		    quadratic * (distance * distance));
            lightFactor += lights[i].colorIntensity.rgb * (attenuation * diff) * lights[i].colorIntensity.a * INTENSITY_FACTOR;

            vec3 viewDir = normalize(viewPos - inFragPos);
            vec3 reflectDir = reflect(-lightDir, norm);

            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            specular += 0.25 * (spec * lights[i].colorIntensity.rgb * attenuation * lights[i].colorIntensity.a * INTENSITY_FACTOR * texture(specularTexture, inUV).r);
        } else if (lightType == LIGHT_TYPE_SPOT) {
             // TODO: actually implement spot lights

             vec3 lightDir = normalize(lights[i].directionOrPos.xyz - inFragPos);

             float diff = max(dot(norm, lightDir), 0.0);
             float constant = 1.0f;
             float linear = 0.09f;
             float quadratic = 0.032f;
             float distance = length(lights[i].directionOrPos.xyz - inFragPos);
             float attenuation = 1.0 / (constant + linear * distance +
                            quadratic * (distance * distance));
             lightFactor += lights[i].colorIntensity.rgb * (attenuation * diff) * lights[i].colorIntensity.a * INTENSITY_FACTOR;
         }
    }

    outColor = vec4((lightFactor + specular) * diffuse, 1.0);
}
