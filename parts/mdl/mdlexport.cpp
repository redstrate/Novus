// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mdlexport.h"
#include <glm/gtc/type_ptr.hpp>

#include "tiny_gltf.h"

void exportModel(const QString &name, const physis_MDL &model, const physis_Skeleton &skeleton, const std::vector<BoneData> &boneData, const QString &fileName)
{
    const int selectedLod = 0;

    const physis_LOD &lod = model.lods[selectedLod];

    tinygltf::Model gltfModel;
    gltfModel.asset.generator = "Novus";

    // TODO: just write the code better! dummy!!
    gltfModel.nodes.reserve(1 + model.num_affected_bones + lod.num_parts);

    auto &gltfSkeletonNode = gltfModel.nodes.emplace_back();
    gltfSkeletonNode.name = skeleton.root_bone->name;

    // find needed root bones
    std::vector<physis_Bone> root_bones;
    // hardcode to n_hara for now
    root_bones.push_back(skeleton.bones[1]);

    for (uint32_t i = 0; i < root_bones.size(); i++) {
        auto &node = gltfModel.nodes.emplace_back();
        node.name = root_bones[i].name;

        auto &real_bone = root_bones[i];
        node.translation = {real_bone.position[0], real_bone.position[1], real_bone.position[2]};
        node.rotation = {real_bone.rotation[0], real_bone.rotation[1], real_bone.rotation[2], real_bone.rotation[3]};
        node.scale = {real_bone.scale[0], real_bone.scale[1], real_bone.scale[2]};
    }

    gltfSkeletonNode.children.push_back(1);

    for (uint32_t i = 0; i < model.num_affected_bones; i++) {
        auto &node = gltfModel.nodes.emplace_back();
        node.name = model.affected_bone_names[i];

        int real_bone_id = 0;
        for (uint32_t k = 0; k < skeleton.num_bones; k++) {
            if (strcmp(skeleton.bones[k].name, model.affected_bone_names[i]) == 0) {
                real_bone_id = k;
            }
        }

        auto &real_bone = skeleton.bones[real_bone_id];
        node.translation = {real_bone.position[0], real_bone.position[1], real_bone.position[2]};
        node.rotation = {real_bone.rotation[0], real_bone.rotation[1], real_bone.rotation[2], real_bone.rotation[3]};
        node.scale = {real_bone.scale[0], real_bone.scale[1], real_bone.scale[2]};
    }

    // setup parenting
    for (uint32_t i = 0; i < model.num_affected_bones; i++) {
        int real_bone_id = 0;
        for (uint32_t k = 0; k < skeleton.num_bones; k++) {
            if (strcmp(skeleton.bones[k].name, model.affected_bone_names[i]) == 0) {
                real_bone_id = k;
            }
        }

        auto &real_bone = skeleton.bones[real_bone_id];
        if (real_bone.parent_bone != nullptr) {
            bool found = false;
            for (uint32_t k = 0; k < model.num_affected_bones; k++) {
                if (strcmp(model.affected_bone_names[k], real_bone.parent_bone->name) == 0) {
                    gltfModel.nodes[k + 2].children.push_back(i + 2); // +1 for the skeleton node taking up the first index
                    found = true;
                }
            }

            // Find the next closest bone that isn't a direct descendant
            // of n_root, but won't have a parent anyway
            if (!found) {
                gltfModel.nodes[1].children.push_back(i + 2);
            }
        } else {
            gltfModel.nodes[1].children.push_back(i + 2);
        }
    }

    auto &gltfSkin = gltfModel.skins.emplace_back();
    gltfSkin.name = gltfSkeletonNode.name;
    gltfSkin.skeleton = 0;
    for (size_t i = 1; i < gltfModel.nodes.size(); i++) {
        gltfSkin.joints.push_back(i);
    }

    // Inverse bind matrices
    {
        gltfSkin.inverseBindMatrices = gltfModel.accessors.size();

        auto &inverseAccessor = gltfModel.accessors.emplace_back();
        inverseAccessor.bufferView = gltfModel.bufferViews.size();
        inverseAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        inverseAccessor.count = gltfModel.nodes.size() - 1;
        inverseAccessor.type = TINYGLTF_TYPE_MAT4;

        auto &inverseBufferView = gltfModel.bufferViews.emplace_back();
        inverseBufferView.buffer = gltfModel.buffers.size();

        auto &inverseBuffer = gltfModel.buffers.emplace_back();
        for (uint32_t i = 1; i < gltfModel.nodes.size(); i++) {
            int real_bone_id = 0;
            for (uint32_t k = 0; k < skeleton.num_bones; k++) {
                if (strcmp(skeleton.bones[k].name, gltfModel.nodes[i].name.c_str()) == 0) {
                    real_bone_id = k;
                }
            }

            auto &real_bone = skeleton.bones[real_bone_id];
            auto inverseMatrix = boneData[real_bone.index].inversePose;
            auto inverseMatrixCPtr = reinterpret_cast<uint8_t *>(glm::value_ptr(inverseMatrix));

            inverseBuffer.data.insert(inverseBuffer.data.end(), inverseMatrixCPtr, inverseMatrixCPtr + sizeof(float) * 16);
        }

        inverseBufferView.byteLength = inverseBuffer.data.size();
    }

    for (uint32_t i = 0; i < lod.num_parts; i++) {
        gltfSkeletonNode.children.push_back(gltfModel.nodes.size());

        auto &gltfNode = gltfModel.nodes.emplace_back();

        gltfNode.name = name.toStdString() + " Part " + std::to_string(i) + ".0";
        gltfNode.skin = 0;

        gltfNode.mesh = gltfModel.meshes.size();
        auto &gltfMesh = gltfModel.meshes.emplace_back();

        gltfMesh.name = gltfNode.name + " Mesh Attribute";

        auto &gltfPrimitive = gltfMesh.primitives.emplace_back();
        gltfPrimitive.attributes["POSITION"] = gltfModel.accessors.size();
        gltfPrimitive.attributes["TEXCOORD_0"] = gltfModel.accessors.size() + 1;
        gltfPrimitive.attributes["TEXCOORD_1"] = gltfModel.accessors.size() + 2;
        gltfPrimitive.attributes["NORMAL"] = gltfModel.accessors.size() + 3;
        gltfPrimitive.attributes["COLOR_0"] = gltfModel.accessors.size() + 6;
        gltfPrimitive.attributes["WEIGHTS_0"] = gltfModel.accessors.size() + 7;
        gltfPrimitive.attributes["JOINTS_0"] = gltfModel.accessors.size() + 8;
        gltfPrimitive.mode = TINYGLTF_MODE_TRIANGLES;

        // Vertices
        {
            auto &positionAccessor = gltfModel.accessors.emplace_back();
            positionAccessor.bufferView = gltfModel.bufferViews.size();
            positionAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            positionAccessor.count = lod.parts[i].num_vertices;
            positionAccessor.type = TINYGLTF_TYPE_VEC3;

            auto &uv0Accessor = gltfModel.accessors.emplace_back();
            uv0Accessor.bufferView = gltfModel.bufferViews.size();
            uv0Accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            uv0Accessor.count = lod.parts[i].num_vertices;
            uv0Accessor.type = TINYGLTF_TYPE_VEC2;
            uv0Accessor.byteOffset = offsetof(Vertex, uv0);

            auto &uv1Accessor = gltfModel.accessors.emplace_back();
            uv1Accessor.bufferView = gltfModel.bufferViews.size();
            uv1Accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            uv1Accessor.count = lod.parts[i].num_vertices;
            uv1Accessor.type = TINYGLTF_TYPE_VEC2;
            uv1Accessor.byteOffset = offsetof(Vertex, uv1);

            auto &normalAccessor = gltfModel.accessors.emplace_back();
            normalAccessor.bufferView = gltfModel.bufferViews.size();
            normalAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            normalAccessor.count = lod.parts[i].num_vertices;
            normalAccessor.type = TINYGLTF_TYPE_VEC3;
            normalAccessor.byteOffset = offsetof(Vertex, normal);

            auto &tangent1Accessor = gltfModel.accessors.emplace_back();
            tangent1Accessor.bufferView = gltfModel.bufferViews.size();
            tangent1Accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
            tangent1Accessor.count = lod.parts[i].num_vertices;
            tangent1Accessor.type = TINYGLTF_TYPE_VEC4;
            tangent1Accessor.byteOffset = offsetof(Vertex, tangent1);

            auto &tangent2Accessor = gltfModel.accessors.emplace_back();
            tangent2Accessor.bufferView = gltfModel.bufferViews.size();
            tangent2Accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
            tangent2Accessor.count = lod.parts[i].num_vertices;
            tangent2Accessor.type = TINYGLTF_TYPE_VEC4;
            tangent2Accessor.byteOffset = offsetof(Vertex, tangent2);

            auto &colorAccessor = gltfModel.accessors.emplace_back();
            colorAccessor.bufferView = gltfModel.bufferViews.size();
            colorAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            colorAccessor.count = lod.parts[i].num_vertices;
            colorAccessor.type = TINYGLTF_TYPE_VEC4;
            colorAccessor.byteOffset = offsetof(Vertex, color);

            auto &boneWeightAccessor = gltfModel.accessors.emplace_back();
            boneWeightAccessor.bufferView = gltfModel.bufferViews.size();
            boneWeightAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            boneWeightAccessor.count = lod.parts[i].num_vertices;
            boneWeightAccessor.type = TINYGLTF_TYPE_VEC4;
            boneWeightAccessor.byteOffset = offsetof(Vertex, bone_weight);

            auto &boneIdAccessor = gltfModel.accessors.emplace_back();
            boneIdAccessor.bufferView = gltfModel.bufferViews.size();
            boneIdAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
            boneIdAccessor.count = lod.parts[i].num_vertices;
            boneIdAccessor.type = TINYGLTF_TYPE_VEC4;
            boneIdAccessor.byteOffset = offsetof(Vertex, bone_id);

            auto &vertexBufferView = gltfModel.bufferViews.emplace_back();
            vertexBufferView.buffer = gltfModel.buffers.size();
            vertexBufferView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

            std::vector<Vertex> newVertices;
            for (int a = 0; a < lod.parts[i].num_vertices; a++) {
                Vertex vertex = lod.parts[i].vertices[a];

                // Account for additional root bone
                vertex.bone_id[0]++;
                vertex.bone_id[1]++;
                vertex.bone_id[2]++;
                vertex.bone_id[3]++;

                newVertices.push_back(vertex);
            }

            auto &vertexBuffer = gltfModel.buffers.emplace_back();
            vertexBuffer.data.resize(lod.parts[i].num_vertices * sizeof(Vertex));
            memcpy(vertexBuffer.data.data(), newVertices.data(), vertexBuffer.data.size());

            vertexBufferView.byteLength = vertexBuffer.data.size();
            vertexBufferView.byteStride = sizeof(Vertex);
        }

        // Indices
        {
            gltfPrimitive.indices = gltfModel.accessors.size();
            auto &indexAccessor = gltfModel.accessors.emplace_back();
            indexAccessor.bufferView = gltfModel.bufferViews.size();
            indexAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
            indexAccessor.count = lod.parts[i].num_indices;
            indexAccessor.type = TINYGLTF_TYPE_SCALAR;

            auto &indexBufferView = gltfModel.bufferViews.emplace_back();
            indexBufferView.buffer = gltfModel.buffers.size();
            indexBufferView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

            auto &indexBuffer = gltfModel.buffers.emplace_back();
            indexBuffer.data.resize(lod.parts[i].num_indices * sizeof(uint16_t));
            memcpy(indexBuffer.data.data(), lod.parts[i].indices, indexBuffer.data.size());

            indexBufferView.byteLength = indexBuffer.data.size();
            indexBufferView.byteStride = sizeof(uint16_t);
        }
    }

    auto &scene = gltfModel.scenes.emplace_back();
    scene.name = name.toStdString();
    scene.nodes = {0};

    tinygltf::TinyGLTF loader;
    loader.WriteGltfSceneToFile(&gltfModel, fileName.toStdString(), true, true, false, true);
}