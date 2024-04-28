// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// Structure definitions from https://github.com/Shaderlayan/Ouroboros
struct CameraParameter {
    glm::mat3x4 m_ViewMatrix; // TODO: does it need alignment?
    glm::mat3x4 m_InverseViewMatrix;
    glm::mat4 m_ViewProjectionMatrix;
    glm::mat4 m_InverseViewProjectionMatrix;
    glm::mat4 m_InverseProjectionMatrix; // used for view position recalc
    glm::mat4 m_ProjectionMatrix; // FIXME: ourburos is wrong, this is actually viewProjection
    glm::mat4 m_MainViewToProjectionMatrix;
    glm::vec4 m_EyePosition;
    glm::vec4 m_LookAtVector;
};

struct JointMatrixArray {
    glm::mat3x4 g_JointMatrixArray[64];
};

struct CameraLight {
    glm::vec4 m_DiffuseSpecular;
    glm::vec4 m_Rim;
};

struct InstanceParameterStruct {
    glm::vec4 m_MulColor;
    glm::vec4 m_EnvParameter;
    CameraLight m_CameraLight;
    glm::vec4 m_Wetness;
};

struct InstanceParameter {
    InstanceParameterStruct g_InstanceParameter;
};

struct ModelParameterStruct {
    glm::vec4 m_Params;
};

struct ModelParameter {
    ModelParameterStruct g_ModelParameter;
};

struct MaterialParameters {
    glm::vec4 parameters[6] = {}; // TODO: this is actually different depending on the shpk
};

struct CommonParameter {
    glm::vec4 m_RenderTarget;
    glm::vec4 m_Viewport;
    glm::vec4 m_Misc;
    glm::vec4 m_Misc2;
};

struct LightParam {
    glm::vec4 m_Position;
    glm::vec4 m_Direction;
    glm::vec4 m_DiffuseColor;
    glm::vec4 m_SpecularColor;
    glm::vec4 m_Attenuation;
    /*glm::vec4 m_ClipMin;
    glm::vec3 m_ClipMax;
    glm::vec3 m_FadeScale;
    glm::vec4 m_ShadowTexMask;
    glm::vec4 m_PlaneRayDirection;
    glm::mat3x4 m_PlaneInversMatrix;
    glm::mat3x4 m_WorldViewInversMatrix;
    glm::mat4 m_LightMapMatrix;
    glm::mat4 m_WorldViewProjectionMatrix;*/
};

struct SceneParameter {
    glm::vec4 m_OcclusionIntensity;
    glm::vec4 m_Wetness;
};

struct CustomizeParameter {
    glm::vec4 m_SkinColor;
    glm::vec4 m_SkinFresnelValue0;
    glm::vec4 m_LipColor;
    glm::vec4 m_MainColor;
    glm::vec4 m_HairFresnelValue0;
    glm::vec4 m_MeshColor;
    glm::vec4 m_LeftColor;
    glm::vec4 m_RightColor;
    glm::vec4 m_OptionColor;
};

struct MaterialParameterDynamic {
    glm::vec4 m_EmissiveColor;
};

struct AmbientParameters {
    glm::vec4 g_AmbientParam[6];
};