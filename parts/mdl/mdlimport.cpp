// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mdlimport.h"

#include <QDebug>
#include <glm/glm.hpp>

#include "tiny_gltf.h"

void importModel(physis_MDL &existingModel, const QString &filename)
{
    tinygltf::Model model;

    std::string error, warning;

    tinygltf::TinyGLTF loader;
    if (!loader.LoadBinaryFromFile(&model, &error, &warning, filename.toStdString())) {
        qInfo() << "Error when loading glTF model:" << error;
        return;
    }

    if (!warning.empty()) {
        qInfo() << "Warnings when loading glTF model:" << warning;
    }

    struct ProcessedSubMesh {
        uint32_t subMeshIndex = 0;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
    };

    bool duplicateBuffers = false; // I hate this.

    // We may be reading the parts of order (0.1, then 1.0, maybe 0.2 and so on) so we have to keep track of our buffers
    struct ProcessedPart {
        uint32_t partIndex = 0;
        int lastPositionViewUsed = -1; // detect duplicate accessor and check their offsets
        std::vector<ProcessedSubMesh> subMeshes;
    };
    std::vector<ProcessedPart> processingParts;

    for (const auto &node : model.nodes) {
        // Detect if it's a mesh node
        if (node.mesh >= 0) {
            qInfo() << "Importing" << node.name;

            const QStringList parts = QString::fromStdString(node.name).split(QLatin1Char(' '));
            const QStringList lodPartNumber = parts[2].split(QLatin1Char('.'));

            const int lodNumber = 0;
            const int partNumber = lodPartNumber[0].toInt();
            const int submeshNumber = lodPartNumber[1].toInt();

            qInfo() << "- Part:" << partNumber;
            qInfo() << "- Submesh:" << submeshNumber;

            if (partNumber >= existingModel.lods[lodNumber].num_parts) {
                qInfo() << "- Skipping because of missing part...";
                continue;
            }

            if (submeshNumber >= existingModel.lods[lodNumber].parts[partNumber].num_submeshes) {
                qInfo() << "- Skipping because of missing submesh...";
                continue;
            }

            ProcessedPart *processedPart = nullptr;
            for (auto &part : processingParts) {
                if (part.partIndex == partNumber) {
                    processedPart = &part;
                    break;
                }
            }

            if (processedPart == nullptr) {
                processedPart = &processingParts.emplace_back();
                processedPart->partIndex = partNumber;
            }

            ProcessedSubMesh &processedSubMesh = processedPart->subMeshes.emplace_back();
            processedSubMesh.subMeshIndex = submeshNumber;

            auto &mesh = model.meshes[node.mesh];
            auto &primitive = mesh.primitives[0];

            const auto getAccessor = [&model, &primitive](const std::string &name, const int index) -> unsigned char const * {
                const auto &positionAccessor = model.accessors[primitive.attributes[name]];
                const auto &positionView = model.bufferViews[positionAccessor.bufferView];
                const auto &positionBuffer = model.buffers[positionView.buffer];

                int elementCount = tinygltf::GetNumComponentsInType(positionAccessor.type);
                int elementSize = tinygltf::GetComponentSizeInBytes(positionAccessor.componentType);

                return (positionBuffer.data.data() + (std::max(positionView.byteStride, (size_t)elementCount * elementSize) * index) + positionView.byteOffset
                        + positionAccessor.byteOffset);
            };

            // All of the accessors are mapped to the same buffer vertex view
            const auto &positionAccessor = model.accessors[primitive.attributes["POSITION"]];

            const auto &indexAccessor = model.accessors[primitive.indices];
            const auto &indexView = model.bufferViews[indexAccessor.bufferView];
            const auto &indexBuffer = model.buffers[indexView.buffer];

            qInfo() << "- Importing mesh of" << positionAccessor.count << "vertices and" << indexAccessor.count << "indices.";

            auto indexData = reinterpret_cast<const uint16_t *>(indexBuffer.data.data() + indexView.byteOffset + indexAccessor.byteOffset);
            for (size_t k = 0; k < indexAccessor.count; k++) {
                processedSubMesh.indices.push_back(indexData[k]);
            }

            std::vector<Vertex> newVertices;
            for (size_t i = 0; i < positionAccessor.count; i++) {
                // vertex data
                glm::vec3 const *positionData = reinterpret_cast<glm::vec3 const *>(getAccessor("POSITION", i));
                glm::vec3 const *normalData = reinterpret_cast<glm::vec3 const *>(getAccessor("NORMAL", i));
                glm::vec2 const *uv0Data = reinterpret_cast<glm::vec2 const *>(getAccessor("TEXCOORD_0", i));
                glm::vec2 const *uv1Data = reinterpret_cast<glm::vec2 const *>(getAccessor("TEXCOORD_1", i));
                glm::vec4 const *weightsData = reinterpret_cast<glm::vec4 const *>(getAccessor("WEIGHTS_0", i));
                uint8_t const *jointsData = reinterpret_cast<uint8_t const *>(getAccessor("JOINTS_0", i));

                // Replace position data
                Vertex vertex{};
                if (i < existingModel.lods[lodNumber].parts[partNumber].num_vertices) {
                    vertex = existingModel.lods[lodNumber].parts[partNumber].vertices[i];
                }

                vertex.position[0] = positionData->x;
                vertex.position[1] = positionData->y;
                vertex.position[2] = positionData->z;

                vertex.normal[0] = normalData->x;
                vertex.normal[1] = normalData->y;
                vertex.normal[2] = normalData->z;

                vertex.uv0[0] = uv0Data->x;
                vertex.uv0[1] = uv0Data->y;

                vertex.uv1[0] = uv1Data->x;
                vertex.uv1[1] = uv1Data->y;

                vertex.bone_weight[0] = weightsData->x;
                vertex.bone_weight[1] = weightsData->y;
                vertex.bone_weight[2] = weightsData->z;
                vertex.bone_weight[3] = weightsData->w;

                // We need to ensure the bones are mapped correctly
                // When exporting from modeling software, it's possible it sorted the nodes (Blender does this)
                for (int i = 0; i < 4; i++) {
                    int originalBoneId = *(jointsData + i);

                    auto joints = model.skins[0].joints;

                    int realBoneId = 0;
                    for (int j = 0; j < existingModel.num_affected_bones; j++) {
                        if (strcmp(existingModel.affected_bone_names[j], model.nodes[joints[originalBoneId]].name.c_str()) == 0) {
                            realBoneId = j;
                            break;
                        }
                    }

                    vertex.bone_id[i] = realBoneId;
                }

                newVertices.push_back(vertex);
            }

            // don't add duplicate vertex data!!
            if (processedPart->lastPositionViewUsed != positionAccessor.bufferView) {
                processedPart->lastPositionViewUsed = positionAccessor.bufferView;
                processedSubMesh.vertices = newVertices;
            } else {
                duplicateBuffers = true;
            }
        }
    }

    size_t index_offset = 0;

    for (auto &part : processingParts) {
        std::vector<Vertex> combinedVertices;
        std::vector<uint16_t> combinedIndices;
        std::vector<SubMesh> newSubmeshes;

        size_t vertex_offset = 0;

        // Turn 0.3, 0.2, 0.1 into 0.1, 0.2, 0.3 so they're all in the combined vertex list correctly
        std::sort(part.subMeshes.begin(), part.subMeshes.end(), [](const ProcessedSubMesh &a, const ProcessedSubMesh &b) {
            return a.subMeshIndex < b.subMeshIndex;
        });

        for (auto &submesh : part.subMeshes) {
            std::copy(submesh.vertices.cbegin(), submesh.vertices.cend(), std::back_inserter(combinedVertices));

            for (unsigned int indice : submesh.indices) {
                // if the buffers are duplicate and shared (like when exporting from Novus)
                // then we don't need to add vertex offset, they are already done
                if (duplicateBuffers) {
                    combinedIndices.push_back(indice);
                } else {
                    combinedIndices.push_back(indice + vertex_offset);
                }
            }

            newSubmeshes.push_back({.index_count = static_cast<uint32_t>(submesh.indices.size()), .index_offset = static_cast<uint32_t>(index_offset)});

            index_offset += submesh.indices.size();
            vertex_offset += submesh.vertices.size();
        }

        physis_mdl_replace_vertices(&existingModel,
                                    0,
                                    part.partIndex,
                                    combinedVertices.size(),
                                    combinedVertices.data(),
                                    combinedIndices.size(),
                                    combinedIndices.data(),
                                    newSubmeshes.size(),
                                    newSubmeshes.data());
    }

    qInfo() << "Successfully imported model!";
}