#version 450

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inFragPos;

layout(location = 0) out vec4 outColor;

void main() {
    const vec3 lightPos = vec3(3);

    vec3 norm = normalize(inNormal);
    vec3 lightDir = normalize(lightPos - inFragPos);

    float diff = max(dot(norm, lightDir), 0.0);

    outColor = vec4(1.0) * diff;
}
