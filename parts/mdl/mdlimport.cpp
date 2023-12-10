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

    for (const auto &node : model.nodes) {
        // Detect if it's a mesh node
        if (node.mesh >= 0) {
            qInfo() << "Importing" << node.name;

            const QStringList parts = QString::fromStdString(node.name).split(QLatin1Char(' '));
            const QStringList lodPartNumber = parts[2].split(QLatin1Char('.'));

            // const int lodNumber = lodPartNumber[0].toInt();
            const int lodNumber = 0;
            const int partNumber = lodPartNumber[0].toInt();

            qInfo() << "- LOD:" << lodNumber;
            qInfo() << "- Part:" << partNumber;

            if (partNumber >= existingModel.lods[lodNumber].num_parts) {
                qInfo() << "- Skipping because of missing part...";
                continue;
            }

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

            const int newVertexCount = positionAccessor.count;
            const int oldVertexCount = existingModel.lods[lodNumber].parts[partNumber].num_vertices;

            if (newVertexCount != oldVertexCount) {
                qInfo() << "- Difference in vertex count!" << newVertexCount << "old:" << oldVertexCount;
            }

            const int newIndexCount = indexAccessor.count;
            const int oldIndexCount = existingModel.lods[lodNumber].parts[partNumber].num_indices;

            if (newIndexCount != oldIndexCount) {
                qInfo() << "- Difference in index count!" << newIndexCount << "old:" << oldIndexCount;
            }

            qInfo() << "- Importing mesh of" << positionAccessor.count << "vertices and" << indexAccessor.count << "indices.";

            std::vector<Vertex> newVertices;
            for (uint32_t i = 0; i < positionAccessor.count; i++) {
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

            auto indexData = (const uint16_t *)(indexBuffer.data.data() + indexView.byteOffset + indexAccessor.byteOffset);

            physis_mdl_replace_vertices(&existingModel, lodNumber, partNumber, newVertices.size(), newVertices.data(), indexAccessor.count, indexData);
        }
    }

    qInfo() << "Successfully imported model!";
}