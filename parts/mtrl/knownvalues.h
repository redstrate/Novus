// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

const QHash<uint, const char *> keys = {
    // Taken from https://github.com/0ceal0t/Dalamud-VFXEditor
    {0x2C6C023C, "DecodeDepthBuffer"},
    {0x627FCF5B, "DecodeDepthBuffer_RAWZ"},
    {0xDDB840BF, "DecodeDepthBuffer_INTZ_FETCH4"},
    {0x52DCF96D, "ComputeSoftParticleAlpha"},
    {0xE6A45E9B, "ComputeSoftParticleAlphaOff"},
    {0xC9D9121B, "ComputeSoftParticleAlphaOn"},
    {0x9A80A23B, "UvPrecisionType_Table"},
    {0x51D70427, "UvPrecisionType_High"},
    {0x3AA90E96, "UvPrecisionType_Medium"},
    {0x45E00267, "UvPrecisionType_Low"},
    {0x2BD1F674, "ForceFarZ_Table"},
    {0xCA351FBF, "ForceFarZ_Disable"},
    {0xE9A88E46, "ForceFarZ_Enable"},
    {0xF4DA16D6, "ApplyFog_Table"},
    {0xEEF0EA2A, "ApplyFog_None"},
    {0x84329E3A, "ApplyFog_RGB"},
    {0xD213A0FA, "ApplyFog_Alpha"},
    {0xA19D807E, "ComputeFinalColorType_Table"},
    {0xDA313F25, "ComputeFinalColorType_NoneControl"},
    {0x9B8DF63B, "ComputeFinalColorType_LerpWhite"},
    {0x1321587A, "ComputeFinalColorType_ModulateAlpha"},
    {0x152531DE, "DepthOffsetType_Table"},
    {0x3BFEF1AB, "DepthOffsetType_Legacy"},
    {0x051B89C4, "DepthOffsetType_FixedIntervalNDC"},
    {0x1F6F0483, "ComputeSoftParticleType_Table"},
    {0xC27BCF4B, "ComputeSoftParticleType_Disable"},
    {0xCA45570B, "ComputeSoftParticleType_Enable"},
    {0x4B77C38D, "UvCompute0_Table"},
    {0x692CA241, "UvCompute_ByParameter"},
    {0x8F47B02C, "UvCompute_ByPixelPosition"},
    {0xED00C839, "UvCompute1_Table"},
    {0xDCE8D2A4, "UvCompute2_Table"},
    {0x7A9FD910, "UvCompute3_Table"},
    {0xFD7A2BE9, "UvSetCount_Table"},
    {0x870D6FAF, "UvSetCount_1"},
    {0xF00A5F39, "UvSetCount_0"},
    {0x1E043E15, "UvSetCount_2"},
    {0x69030E83, "UvSetCount_3"},
    {0xF7679B20, "UvSetCount_4"},
    {0x2182C4CC, "TextureColor1_ColorToAlpha_Table"},
    {0x6B6CB989, "TextureColor1_ColorToAlpha_Off"},
    {0x0933F504, "TextureColor1_ColorToAlpha_On"},
    {0x38F81F3C, "TextureColor1_Decode_Table"},
    {0xEA226A56, "TextureColor1_Decode_Off"},
    {0xF9F6E7D0, "TextureColor1_Decode_On"},
    {0x640B0CFA, "TextureColor1_UvNo_Table"},
    {0x5910EB7B, "TextureColor1_Uv_0"},
    {0x2E17DBED, "TextureColor1_Uv_1"},
    {0xB71E8A57, "TextureColor1_Uv_2"},
    {0xC019BAC1, "TextureColor1_Uv_3"},
    {0x30450F85, "TextureColor1_Table"},
    {0x03F39E6F, "TextureColor1_Disable"},
    {0x2309D835, "TextureColor1_Enable"},
    {0x8E2B8906, "TextureColor2_ColorToAlpha_Table"},
    {0x81EA64EB, "TextureColor2_ColorToAlpha_Off"},
    {0x5AA9AE80, "TextureColor2_ColorToAlpha_On"},
    {0x44993AE7, "TextureColor2_Decode_Table"},
    {0x9DBCB8A6, "TextureColor2_Decode_Off"},
    {0x1395DE0A, "TextureColor2_UvNo_Table"},
    {0xDF8499D5, "TextureColor2_Uv_0"},
    {0xA883A943, "TextureColor2_Uv_1"},
    {0x318AF8F9, "TextureColor2_Uv_2"},
    {0x468DC86F, "TextureColor2_Uv_3"},
    {0x4CAB2E3A, "TextureColor2_CalculateAlpha_Table"},
    {0x337A817D, "TextureColor2_CalculateAlpha_Mul"},
    {0x226D091D, "TextureColor2_CalculateAlpha_None"},
    {0x070E8255, "TextureColor2_CalculateAlpha_Max"},
    {0x3B03BD0C, "TextureColor2_CalculateAlpha_Min"},
    {0xE9F712D7, "TextureColor2_CalculateColor_Table"},
    {0x09388548, "TextureColor2_CalculateColor_Mul"},
    {0x5D20D70E, "TextureColor2_CalculateColor_Add"},
    {0x74E48F3A, "TextureColor2_CalculateColor_None"},
    {0xF8382635, "TextureColor2_CalculateColor_Sub"},
    {0x3D4C8660, "TextureColor2_CalculateColor_Max"},
    {0x0141B939, "TextureColor2_CalculateColor_Min"},
    {0x01AD1518, "TextureColor2_Table"},
    {0x3A7EA2AA, "TextureColor2_Disable"},
    {0xAD86DFD6, "TextureColor2_Enable"},
    {0xEB4CB240, "TextureColor3_ColorToAlpha_Table"},
    {0x6EB8D20A, "TextureColor3_ColorToAlpha_Off"},
    {0xDD0F65C3, "TextureColor3_ColorToAlpha_On"},
    {0xD996DB91, "TextureColor3_Decode_Table"},
    {0x0619F4C9, "TextureColor3_Decode_Off"},
    {0x88309265, "TextureColor3_UvNo_Table"},
    {0x14D84A70, "TextureColor3_Uv_0"},
    {0x63DF7AE6, "TextureColor3_Uv_1"},
    {0xFAD62B5C, "TextureColor3_Uv_2"},
    {0x8DD11BCA, "TextureColor3_Uv_3"},
    {0x5B834AFA, "TextureColor3_CalculateAlpha_Table"},
    {0x561DBA3B, "TextureColor3_CalculateAlpha_Mul"},
    {0x6269B913, "TextureColor3_CalculateAlpha_Max"},
    {0xBDB78A83, "TextureColor3_CalculateAlpha_None"},
    {0x5E64864A, "TextureColor3_CalculateAlpha_Min"},
    {0xFEDF7617, "TextureColor3_CalculateColor_Table"},
    {0x6C5FBE0E, "TextureColor3_CalculateColor_Mul"},
    {0x3847EC48, "TextureColor3_CalculateColor_Add"},
    {0xEB3E0CA4, "TextureColor3_CalculateColor_None"},
    {0x582BBD26, "TextureColor3_CalculateColor_Max"},
    {0x9D5F1D73, "TextureColor3_CalculateColor_Sub"},
    {0x6426827F, "TextureColor3_CalculateColor_Min"},
    {0xA7DA1EAC, "TextureColor3_Table"},
    {0x2D05B6E9, "TextureColor3_Disable"},
    {0x612CDF48, "TextureColor3_Enable"},
    {0x0A0814D3, "TextureColor4_ColorToAlpha_Table"},
    {0x8F96D86E, "TextureColor4_ColorToAlpha_Off"},
    {0xFD9D1988, "TextureColor4_ColorToAlpha_On"},
    {0xBC5B7151, "TextureColor4_Decode_Table"},
    {0x72811D46, "TextureColor4_Decode_Off"},
    {0xFCA87BEA, "TextureColor4_UvNo_Table"},
    {0x09DD7AC8, "TextureColor4_Uv_0"},
    {0xE7D31BE4, "TextureColor4_Uv_2"},
    {0x90D42B72, "TextureColor4_Uv_3"},
    {0x7EDA4A5E, "TextureColor4_Uv_1"},
    {0x3E5A74BA, "TextureColor4_CalculateAlpha_Table"},
    {0xB7591CA8, "TextureColor4_CalculateAlpha_Mul"},
    {0xD4500CDB, "TextureColor4_CalculateAlpha_None"},
    {0xBF2020D9, "TextureColor4_CalculateAlpha_Min"},
    {0x832D1F80, "TextureColor4_CalculateAlpha_Max"},
    {0x9B064857, "TextureColor4_CalculateColor_Table"},
    {0x8D1B189D, "TextureColor4_CalculateColor_Mul"},
    {0xD9034ADB, "TextureColor4_CalculateColor_Add"},
    {0x82D98AFC, "TextureColor4_CalculateColor_None"},
    {0x856224EC, "TextureColor4_CalculateColor_Min"},
    {0x7C1BBBE0, "TextureColor4_CalculateColor_Sub"},
    {0xB96F1BB5, "TextureColor4_CalculateColor_Max"},
    {0x627D2022, "TextureColor4_Table"},
    {0x4964DB20, "TextureColor4_Disable"},
    {0x6BE9D651, "TextureColor4_Enable"},
    {0x95D37D89, "TextureNormal_UvNo_Table"},
    {0x5285A598, "TextureNormal_Uv_0"},
    {0x2582950E, "TextureNormal_Uv_1"},
    {0xBC8BC4B4, "TextureNormal_Uv_2"},
    {0xCB8CF422, "TextureNormal_Uv_3"},
    {0x094D2909, "TextureNormal_Table"},
    {0x46C8F5DD, "TextureNormal_Disable"},
    {0xC73E1F18, "TextureNormal_Enable"},
    {0xA1F5312D, "TextureReflection_CalculateColor_Table"},
    {0xCC110032, "TextureReflection_CalculateColor_Mul"},
    {0x98095274, "TextureReflection_CalculateColor_Add"},
    {0xF865031A, "TextureReflection_CalculateColor_Max"},
    {0x3D11A34F, "TextureReflection_CalculateColor_Sub"},
    {0xFAFCF387, "TextureReflection_Table"},
    {0x85CE8863, "TextureReflection_Disable"},
    {0xCDCD00E5, "TextureReflection_Enable"},
    {0x295712ED, "TextureDistortion_UvNo_Table"},
    {0x9FBC8583, "TextureDistortion_Uv_0"},
    {0xE8BBB515, "TextureDistortion_Uv_1"},
    {0x06B5D439, "TextureDistortion_Uv_3"},
    {0x71B2E4AF, "TextureDistortion_Uv_2"},
    {0x97561751, "TextureDistortion_UvSet0_Table"},
    {0x261C31BD, "TextureDistortion_UvSet_Disable"},
    {0xA374AA87, "TextureDistortion_UvSet_Enable"},
    {0x31211CE5, "TextureDistortion_UvSet1_Table"},
    {0x00C90678, "TextureDistortion_UvSet2_Table"},
    {0xA6BE0DCC, "TextureDistortion_UvSet3_Table"},
    {0x83E5D9C5, "TextureDistortion_Table"},
    {0x9C3A152B, "TextureDistortion_Disable"},
    {0x55663973, "TextureDistortion_Enable"},
    {0x837F9F33, "TexturePalette_Table"},
    {0x4163B72F, "TexturePalette_Disable"},
    {0x01B8F41C, "TexturePalette_Enable"},
    {0x97C9F730, "PointLightPositionType_Table"},
    {0xC696A15D, "PointLightPositionType_PerPixel"},
    {0x6EB1F7A6, "PointLightPositionType_PerModel"},
    {0xF0E08E18, "PointLightType_Table"},
    {0xF4C45A84, "PointLightType_Lambert"},
    {0x127BA6E5, "PointLightType_HalfLambert"},
    {0xAC3C2480, "PointLightType_Ex"},
    {0x9391070F, "PointLightCount_Table"},
    {0xA1CC1C77, "PointLightCount_0_0"},
    {0xA00E7640, "PointLightCount_1_0"},
    {0xD70946D6, "PointLightCount_1_1"},
    {0x8FF4ACEB, "DirectionalLightType_Table"},
    {0x1E35D60A, "DirectionalLightType_Lambert"},
    {0xBAAA5E93, "DirectionalLightType_HalfLambert"},
    {0xFC588A87, "DirectionalLightType_Ex"},
    {0x907B83D2, "DirectionalLight_Table"},
    {0x12073AC9, "DirectionalLight_Disable"},
    {0xD6A622EE, "DirectionalLight_Enable"},
    {0x82730F4C, "ApplyLightBufferType_Table"},
    {0x19CB405E, "ApplyLightBufferType_Disable"},
    {0xBB592EBB, "OutputType_Table"},
    {0x21D5E65A, "OutputType_Debug"},
    {0xDF5D501C, "OutputType_Release"},
    {0x086F8E39, "GetInstanceData"},
    {0x815446B5, "GeometryInstancingOn"},
    {0xD7825D20, "GeometryInstancingOff"},
    {0x4855866D, "GetNoInstancingData_Bush"},
    {0xC011FE92, "GetInstancingData_Bush"},
    {0xBB30A69D, "GetLocalPosition"},
    {0xEFCC34B1, "GetLocalPositionNone"},
    {0xCF415365, "GetLocalPositionTerrainEadg"},
    {0x8B036665, "ApplyDitherClip"},
    {0x0802566A, "ApplyDitherClipOff"},
    {0x61B0CF19, "ApplyDitherClipOn"},
    {0xCBDFD5EC, "GetNormalMap"},
    {0xA66B15A1, "GetNormalMapOff"},
    {0xD9994EF1, "GetNormalMapOn"},
    {0xD9FD8A1C, "GetNormalMapParallaxOcclusion"},
    {0x105C6A52, "ApplyWavingAnim"},
    {0x7E47A68D, "ApplyWavingAnimOff"},
    {0xF801B859, "ApplyWavingAnimOn"},
    {0xA5A1910D, "TransformView"},
    {0x4123B1A3, "TransformViewRigid"},
    {0x9C14C8E9, "TransformViewSkin"},
    {0x8955127D, "GetAmbientLight"},
    {0x3FC15CC4, "GetAmbientLight_None"},
    {0xB1AD809A, "GetAmbientLight_SH"},
    {0x67F75CDF, "GetReflectColor"},
    {0x54ECE850, "GetReflectColor_None"},
    {0x447C6F75, "GetReflectColor_Texture"},
    {0x594F3698, "GetAmbientOcclusion"},
    {0x602CCFBB, "GetAmbientOcclusion_None"},
    {0x385A6A78, "GetAmbientOcclusion_Apply"},
    {0x8BBA71F8, "SelectOutput"},
    {0xC8498E95, "SelectOutputMul"},
    {0x9C51DCD3, "SelectOutputAdd"},
    {0x0BD07791, "GetColor"},
    {0xA1DA4309, "GetColor0"},
    {0xD6DD739F, "GetColor1"},
    {0xF3BA7D0E, "GetColor_Texture"},
    {0x8115916D, "GetDirectionalLight"},
    {0x4C5E8831, "GetDirectionalLight_Disable"},
    {0x51EDD496, "GetDirectionalLight_Enable"},
    {0xD73B9E89, "GetDirectionalLight_Shadow"},
    {0x3C957CD3, "GetFakeSpecular"},
    {0x1BB2530E, "GetFakeSpecular_Disable"},
    {0xD583CEE2, "GetFakeSpecular_Enable"},
    {0xEAC154EC, "GetUnderWaterLighting"},
    {0x2AA7A53C, "GetUnderWaterLighting_Disable"},
    {0xAEFB134C, "GetUnderWaterLighting_Enable"},
    {0xA89D89F0, "ShadowSoftShadowType"},
    {0x749F8AB2, "ShadowSoftShadowType_1x1"},
    {0x99153FF0, "ShadowSoftShadowType_3x3"},
    {0x3312B7E1, "ShadowDistanceFadeType"},
    {0x93A57FCE, "ShadowDistanceFadeType_Disable"},
    {0xCAF5E83A, "ShadowDistanceFadeType_Enable"},
    {0x09500613, "TransformProj"},
    {0x1B3986BB, "TransformProjPlaneNear"},
    {0xD6E21545, "TransformProjPlaneFar"},
    {0xECF43C9A, "TransformProjBox"},
    {0xF9C71291, "GetShadow"},
    {0xF30C5874, "GetShadowCascade"},
    {0x17153645, "GetShadowCascadeWith"},
    {0x93003148, "GetShadowCloudOnly"},
    {0x37A94607, "GetShadow_Enable"},
    {0x764AECE7, "ApplyWavingAnimation"},
    {0x69BA5521, "ApplyWavingAnimation_Nothing"},
    {0x7DDA17B6, "ApplyWavingAnimation_AutoPlacement"},
    {0x772A5C52, "ApplyWavingAnimation_Shigemi"},
    {0x53AF00ED, "ApplyAttenuation"},
    {0x2795EAA4, "ApplyAttenuation_Linear"},
    {0xE79A9E9B, "ApplyAttenuation_Quadratic"},
    {0x4495A6B1, "ApplyAttenuation_Cubic"},
    {0x51668572, "ApplyOmniShadow"},
    {0x7E17135D, "ApplyOmniShadow_Disable"},
    {0xCECC3682, "ApplyOmniShadow_Map"},
    {0x3E6FD38A, "ApplyOmniShadow_Chara"},
    {0x5CC2C7B3, "ApplyOmniShadow_MapChara"},
    {0x7DB09695, "LightClip"},
    {0x61628441, "LightClip_Disable"},
    {0x6F0E2969, "LightClip_Enable"},
    {0x0D812FA4, "SpecularLighting"},
    {0xAB1CE916, "SpecularLighting_Disable"},
    {0xABA1F498, "SpecularLighting_Enable"},
    {0xF66E4589, "ApplyMaskTexture"},
    {0x3E5F4521, "ApplyMaskTexture_Disable"},
    {0x57474150, "ApplyMaskTexture_Enable"},
    {0x11433F2D, "GetRLR"},
    {0x6B2E2D05, "GetRLROff"},
    {0x4BA77904, "GetRLROn"},
    {0x7725989B, "ApplyUnderWater"},
    {0xEF6A4182, "ApplyUnderWaterOff"},
    {0xB18A2017, "ApplyUnderWaterOn"},
    {0xD7826DAA, "TransformType"},
    {0x1943C146, "TransformTypePlane"},
    {0xC8AFF84F, "TransformTypeBox"},
    {0x52D21D34, "ApplyConeAttenuation"},
    {0x8B39CBDF, "ApplyConeAttenuation_Disable"},
    {0xE1068EED, "ApplyConeAttenuation_Enable"},
    {0xEA931ECA, "AddLayer"},
    {0x5D82881C, "AddLayer0"},
    {0x2A85B88A, "AddLayer1"},
    {0xB38CE930, "AddLayer2"},
    {0x575CA84C, "Lighting"},
    {0x470E5A1E, "LightingNormal"},
    {0x2807B89E, "LightingLow"},
    {0xB18FE63D, "Default"},
    {0x86CA5FE4, "DefaultTechnique"},
    {0x61B590F0, "Color"},
    {0xFD40C470, "Depth"},
    {0xF43B2F35, "SUB_VIEW_MAIN"},
    {0x99B22D1C, "SUB_VIEW_SHADOW_0"},
    {0x66244231, "SUB_VIEW_CUBE_0"},
    {0xAE5E6A42, "SUB_VIEW_ROOF"},
    {0xEEB51D8A, "SUB_VIEW_SHADOW_1"},
    {0x344CE408, "SHADOW"},

    // Taken from https://github.com/Shaderlayan/Ouroboros
    {0xF52CCF05, "Vertex Color Mode"},
    {0xDFE74BAC, "Color"},
    {0xA7D2FF60, "Mask"},

    {0xB616DC5A, "Texture Mode"},
    {0x5CC605B5, "Multi"},
    {0x22A4AABF, "Simple"},
    {0x600EF9DF, "Compatibility"},

    {0xD2777173, "Decal Mode"},
    {0x4242B842, "None"},
    {0x584265DD, "Alpha"},
    {0xF35F5131, "Color"},

    {0xC8BD1DEF, "Specular Map Mode"},
    {0x198D11CD, "Color"},
    {0xA02F4828, "Multi"},

    // Parameters
    {physis_shpk_crc("g_DiffuseColor"), "g_DiffuseColor"},
    {physis_shpk_crc("g_AlphaThreshold"), "g_AlphaThreshold"},
    {physis_shpk_crc("g_FresnelValue0"), "g_FresnelValue0"},
    {physis_shpk_crc("g_SpecularMask"), "g_SpecularMask"},
    {physis_shpk_crc("g_LipFresnelValue0"), "g_LipFresnelValue0"},
    {physis_shpk_crc("g_Shininess"), "g_Shininess"},
    {physis_shpk_crc("g_EmissiveColor"), "g_EmissiveColor"},
    {physis_shpk_crc("g_LipShininess"), "g_LipShininess"},
    {physis_shpk_crc("g_TileScale"), "g_TileScale"},
    {physis_shpk_crc("g_AmbientOcclusionMask"), "g_AmbientOcclusionMask"},
    {physis_shpk_crc("g_TileIndex"), "g_TileIndex"},
    {physis_shpk_crc("g_ScatteringLevel"), "g_ScatteringLevel"},
    {physis_shpk_crc("g_NormalScale"), "g_NormalScale"},
    {physis_shpk_crc("g_ShaderID"), "g_ShaderID"},
    {physis_shpk_crc("g_SpecularColor"), "g_SpecularColor"},
    {physis_shpk_crc("g_SpecularColorMask"), "g_SpecularColorMask"},
    {physis_shpk_crc("g_LipRoughnessScale"), "g_LipRoughnessScale"},
    {physis_shpk_crc("g_WhiteEyeColor"), "g_WhiteEyeColor"},
    {physis_shpk_crc("g_SphereMapIndex"), "g_SphereMapIndex"},
    {physis_shpk_crc("g_SSAOMask"), "g_SSAOMask"},
    {physis_shpk_crc("g_TileAlpha"), "g_TileAlpha"},
    {physis_shpk_crc("g_SheenRate"), "g_SheenRate"},
    {physis_shpk_crc("g_SheenTintRate"), "g_SheenTintRate"},
    {physis_shpk_crc("g_SheenAperture"), "g_SheenAperture"},
    {physis_shpk_crc("g_IrisRingColor"), "g_IrisRingColor"},
    {physis_shpk_crc("g_IrisRingEmissiveIntensity"), "g_IrisRingEmissiveIntensity"},
    {physis_shpk_crc("g_IrisThickness"), "g_IrisThickness"},
    {physis_shpk_crc("g_IrisOptionColorRate"), "g_IrisOptionColorRate"},
    {physis_shpk_crc("g_AlphaAperture"), "g_AlphaAperture"},
    {physis_shpk_crc("g_AlphaOffset"), "g_AlphaOffset"},
    {physis_shpk_crc("g_GlassIOR"), "g_GlassIOR"},
    {physis_shpk_crc("g_GlassThicknessMax"), "g_GlassThicknessMax"},
    {physis_shpk_crc("g_TextureMipBias"), "g_TextureMipBias"},
    {physis_shpk_crc("g_OutlineColor"), "g_OutlineColor"},
    {physis_shpk_crc("g_OutlineWidth"), "g_OutlineWidth"},
    {physis_shpk_crc("g_Ray"), "g_Ray"},
    {physis_shpk_crc("g_TexU"), "g_TexU"},
    {physis_shpk_crc("g_TexV"), "g_TexV"},
    {physis_shpk_crc("g_TexAnim"), "g_TexAnim"},
    {physis_shpk_crc("g_Color"), "g_Color"},
    {physis_shpk_crc("g_ShadowAlphaThreshold"), "g_ShadowAlphaThreshold"},
    {physis_shpk_crc("g_NearClip"), "g_NearClip"},
    {physis_shpk_crc("g_AngleClip"), "g_AngleClip"},
    {physis_shpk_crc("g_CausticsReflectionPowerBright"), "g_CausticsReflectionPowerBright"},
    {physis_shpk_crc("g_CausticsReflectionPowerDark"), "g_CausticsReflectionPowerDark"},
    {physis_shpk_crc("g_HeightScale"), "g_HeightScale"},
    {physis_shpk_crc("g_HeightMapScale"), "g_HeightMapScale"},
    {physis_shpk_crc("g_HeightMapUVScale"), "g_HeightMapUVScale"},
    {physis_shpk_crc("g_MultiWaveScale"), "g_MultiWaveScale"},
    {physis_shpk_crc("g_WaveSpeed"), "g_WaveSpeed"},
    {physis_shpk_crc("g_WaveTime"), "g_WaveTime"},
    {physis_shpk_crc("g_AlphaMultiParam"), "g_AlphaMultiParam"},
    {physis_shpk_crc("g_ColorUVScale"), "g_ColorUVScale"},
    {physis_shpk_crc("g_DetailID"), "g_DetailID"},
    {physis_shpk_crc("g_DetailNormalScale"), "g_DetailNormalScale"},
    {physis_shpk_crc("g_DetailColorUvScale"), "g_DetailColorUvScale"},
    {physis_shpk_crc("g_DetailColor"), "g_DetailColor"},
    {physis_shpk_crc("g_DetailNormalUvScale"), "g_DetailNormalUvScale"},
    {physis_shpk_crc("g_EnvMapPower"), "g_EnvMapPower"},
    {physis_shpk_crc("g_InclusionAperture"), "g_InclusionAperture"},
    {physis_shpk_crc("g_IrisRingForceColor"), "g_IrisRingForceColor"},
    {physis_shpk_crc("g_LayerDepth"), "g_LayerDepth"},
    {physis_shpk_crc("g_LayerIrregularity"), "g_LayerIrregularity"},
    {physis_shpk_crc("g_LayerScale"), "g_LayerScale"},
    {physis_shpk_crc("g_LayerVelocity"), "g_LayerVelocity"},
    {physis_shpk_crc("g_MultiDetailColor"), "g_MultiDetailColor"},
    {physis_shpk_crc("g_MultiDiffuseColor"), "g_MultiDiffuseColor"},
    {physis_shpk_crc("g_MultiEmissiveColor"), "g_MultiEmissiveColor"},
    {physis_shpk_crc("g_MultiHeightScale"), "g_MultiHeightScale"},
    {physis_shpk_crc("g_MultiNormalScale"), "g_MultiNormalScale"},
    {physis_shpk_crc("g_MultiSpecularColor "), "g_MultiSpecularColor "},
    {physis_shpk_crc("g_MultiSSAOMask"), "g_MultiSSAOMask"},
    {physis_shpk_crc("g_MultiWhitecapDistortion"), "g_MultiWhitecapDistortion"},
    {physis_shpk_crc("g_MultiWhitecapScale"), "g_MultiWhitecapScale"},
    {physis_shpk_crc("g_NormalScale1"), "g_NormalScale1"},
    {physis_shpk_crc("g_NormalUVScale"), "g_NormalUVScale"},
    {physis_shpk_crc("g_PrefersFailure"), "g_PrefersFailure"},
    {physis_shpk_crc("g_ReflectionPower"), "g_ReflectionPower"},
    {physis_shpk_crc("g_ShadowOffset"), "g_ShadowOffset"},
    {physis_shpk_crc("g_ShadowPosOffset"), "g_ShadowPosOffset"},
    {physis_shpk_crc("g_SpecularPower"), "g_SpecularPower"},
    {physis_shpk_crc("g_SpecularUVScale"), "g_SpecularUVScale"},
    {physis_shpk_crc("g_ToonIndex"), "g_ToonIndex"},
    {physis_shpk_crc("g_ToonLightScale"), "g_ToonLightScale"},
    {physis_shpk_crc("g_ToonReflectionScale"), "g_ToonReflectionScale"},
    {physis_shpk_crc("g_ToonSpecIndex"), "g_ToonSpecIndex"},
    {physis_shpk_crc("g_TransparencyDistance"), "g_TransparencyDistance"},
    {physis_shpk_crc("g_WaveletDistortion"), "g_WaveletDistortion"},
    {physis_shpk_crc("g_WaveletNoiseParam"), "g_WaveletNoiseParam"},
    {physis_shpk_crc("g_WaveletOffset"), "g_WaveletOffset"},
    {physis_shpk_crc("g_WaveletScale"), "g_WaveletScale"},
    {physis_shpk_crc("g_WaveTime1"), "g_WaveTime1"},
    {physis_shpk_crc("g_WhitecapDistance"), "g_WhitecapDistance"},
    {physis_shpk_crc("g_WhitecapDistortion"), "g_WhitecapDistortion"},
    {physis_shpk_crc("g_WhitecapNoiseScale"), "g_WhitecapNoiseScale"},
    {physis_shpk_crc("g_WhitecapScale"), "g_WhitecapScale"},
    {physis_shpk_crc("g_WhitecapSpeed"), "g_WhitecapSpeed"},
    {physis_shpk_crc("g_Fresnel"), "g_Fresnel"},
    {physis_shpk_crc("g_Gradation"), "g_Gradation"},
    {physis_shpk_crc("g_Intensity"), "g_Intensity"},
    {physis_shpk_crc("g_LayerColor"), "g_LayerColor"},
    {physis_shpk_crc("g_RefractionColor"), "g_RefractionColor"},
    {physis_shpk_crc("g_WhitecapColor"), "g_WhitecapColor"},
    {physis_shpk_crc("g_BackScatterPower"), "g_BackScatterPower"},
    {physis_shpk_crc("g_FarClip"), "g_FarClip"},
    {physis_shpk_crc("g_FurLength"), "g_FurLength"},
    {physis_shpk_crc("g_HairBackScatterRoughnessOffsetRate"), "g_HairBackScatterRoughnessOffsetRate"},
    {physis_shpk_crc("g_HairScatterColorShift"), "g_HairScatterColorShift"},
    {physis_shpk_crc("g_HairSecondaryRoughnessOffsetRate"), "g_HairSecondaryRoughnessOffsetRate"},
    {physis_shpk_crc("g_HairSpecularBackScatterShift"), "g_HairSpecularBackScatterShift"},
    {physis_shpk_crc("g_HairSpecularPrimaryShift"), "g_HairSpecularPrimaryShift"},
    {physis_shpk_crc("g_HairSpecularSecondaryShift"), "g_HairSpecularSecondaryShift"},
    {physis_shpk_crc("g_LightingType"), "g_LightingType"},
    {physis_shpk_crc("g_MultiSpecularColor"), "g_MultiSpecularColor"},
    {physis_shpk_crc("g_SubSurfacePower"), "g_SubSurfacePower"},
    {physis_shpk_crc("g_SubSurfaceProfileID"), "g_SubSurfaceProfileID"},
    {physis_shpk_crc("g_SubSurfaceWidth"), "g_SubSurfaceWidth"},
    {physis_shpk_crc("g_Transparency"), "g_Transparency"},
    {physis_shpk_crc("g_UseSubSurfaceRate"), "g_UseSubSurfaceRate"},
    {physis_shpk_crc("g_VertexMovementScale"), "g_VertexMovementScale"},

    // Samplers
    {physis_shpk_crc("g_SamplerViewPosition"), "g_SamplerViewPosition"},
    {physis_shpk_crc("g_SamplerVPosition"), "g_SamplerVPosition"},
    {physis_shpk_crc("g_SamplerDither"), "g_SamplerDither"},
    {physis_shpk_crc("g_SamplerColorMap0"), "g_SamplerColorMap0"},
    {physis_shpk_crc("g_SamplerColorMap1"), "g_SamplerColorMap1"},
    {physis_shpk_crc("g_SamplerNormalMap0"), "g_SamplerNormalMap0"},
    {physis_shpk_crc("g_SamplerNormalMap1"), "g_SamplerNormalMap1"},
    {physis_shpk_crc("g_SamplerSpecularMap0"), "g_SamplerSpecularMap0"},
    {physis_shpk_crc("g_SamplerSpecularMap1"), "g_SamplerSpecularMap1"},
    {physis_shpk_crc("g_SamplerEnvMap"), "g_SamplerEnvMap"},
    {physis_shpk_crc("g_SamplerLightDiffuse"), "g_SamplerLightDiffuse"},
    {physis_shpk_crc("g_SamplerLightSpecular"), "g_SamplerLightSpecular"},
    {physis_shpk_crc("g_SamplerAttenuation"), "g_SamplerAttenuation"},
    {physis_shpk_crc("g_SamplerOmniShadowStatic"), "g_SamplerOmniShadowStatic"},
    {physis_shpk_crc("g_SamplerOmniShadowDynamic"), "g_SamplerOmniShadowDynamic"},
    {physis_shpk_crc("g_SamplerOmniShadowIndexTable"), "g_SamplerOmniShadowIndexTable"},
    {physis_shpk_crc("g_SamplerGBuffer"), "g_SamplerGBuffer"},
    {physis_shpk_crc("g_SamplerGBuffer1"), "g_SamplerGBuffer1"},
    {physis_shpk_crc("g_SamplerGBuffer2"), "g_SamplerGBuffer2"},
    {physis_shpk_crc("g_SamplerGBuffer3"), "g_SamplerGBuffer3"},
    {physis_shpk_crc("g_SamplerLight"), "g_SamplerLight"},
    {physis_shpk_crc("g_SamplerShadow"), "g_SamplerShadow"},
    {physis_shpk_crc("g_CloudShadowSampler"), "g_CloudShadowSampler"},
    {physis_shpk_crc("g_RoofSampler"), "g_RoofSampler"},
    {physis_shpk_crc("g_AnimSampler"), "g_AnimSampler"},
    {physis_shpk_crc("g_SamplerCaustics"), "g_SamplerCaustics"},
    {physis_shpk_crc("g_SamplerCaustics"), "g_SamplerCaustics"},
    {physis_shpk_crc("g_SamplerColor1"), "g_SamplerColor1"},
    {physis_shpk_crc("g_SamplerColor2"), "g_SamplerColor2"},
    {physis_shpk_crc("g_SamplerColor3"), "g_SamplerColor3"},
    {physis_shpk_crc("g_SamplerColor4"), "g_SamplerColor4"},
    {physis_shpk_crc("g_SamplerDistortion"), "g_SamplerDistortion"},
    {physis_shpk_crc("g_SamplerPalette"), "g_SamplerPalette"},
    {physis_shpk_crc("g_SamplerDiffuse"), "g_SamplerDiffuse"},
    {physis_shpk_crc("g_SamplerNormal"), "g_SamplerNormal"},
    {physis_shpk_crc("g_SamplerMask"), "g_SamplerMask"},
    {physis_shpk_crc("g_SamplerSpecular"), "g_SamplerSpecular"},
    {physis_shpk_crc("g_SamplerDecal"), "g_SamplerDecal"},
    {physis_shpk_crc("g_SamplerTable"), "g_SamplerTable"},
    {physis_shpk_crc("g_SamplerTileDiffuse"), "g_SamplerTileDiffuse"},
    {physis_shpk_crc("g_SamplerTileNormal"), "g_SamplerTileNormal"},
    {physis_shpk_crc("g_SamplerIndex"), "g_SamplerIndex"},
    {physis_shpk_crc("g_SamplerCatchlight"), "g_SamplerCatchlight"},
    {physis_shpk_crc("g_SamplerReflection"), "g_SamplerReflection"},
    {physis_shpk_crc("g_SamplerOcclusion"), "g_SamplerOcclusion"},
    {physis_shpk_crc("g_SamplerDepth"), "g_SamplerDepth"},
    {physis_shpk_crc("g_ToneMapSampler"), "g_ToneMapSampler"},
    {physis_shpk_crc("g_SkySampler"), "g_SkySampler"},
    {physis_shpk_crc("g_Sampler"), "g_Sampler"},
    {physis_shpk_crc("g_SamplerFresnel"), "g_SamplerFresnel"},
    {physis_shpk_crc("g_SamplerColorMap"), "g_SamplerColorMap"},
    {physis_shpk_crc("g_SamplerNormalMap"), "g_SamplerNormalMap"},
    {physis_shpk_crc("g_SamplerMaskMap"), "g_SamplerMaskMap"},
    {physis_shpk_crc("g_SamplerSpecularMap"), "g_SamplerSpecularMap"},
    {physis_shpk_crc("g_SamplerDistortionMap"), "g_SamplerDistortionMap"},
    {physis_shpk_crc("g_SamplerWaveMap"), "g_SamplerWaveMap"},
    {physis_shpk_crc("g_SamplerWhitecapMap"), "g_SamplerWhitecapMap"},
    {physis_shpk_crc("g_SamplerWaveletMap0"), "g_SamplerWaveletMap0"},
    {physis_shpk_crc("g_SamplerWaveletMap1"), "g_SamplerWaveletMap1"},
    {physis_shpk_crc("g_SamplerReflectionMap"), "g_SamplerReflectionMap"},
    {physis_shpk_crc("g_SamplerNoise"), "g_SamplerNoise"},
    {physis_shpk_crc("g_SamplerWaveletNoise"), "g_SamplerWaveletNoise"},
    {physis_shpk_crc("g_SamplerRefractionMap"), "g_SamplerRefractionMap"},
};
