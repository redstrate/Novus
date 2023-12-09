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
            const QString &name = parts[0];
            const QStringList lodPartNumber = parts[2].split(QLatin1Char('.'));

            const int lodNumber = lodPartNumber[0].toInt();
            const int partNumber = lodPartNumber[1].toInt();

            qInfo() << "- LOD:" << lodNumber;
            qInfo() << "- Part:" << partNumber;

            auto &mesh = model.meshes[node.mesh];
            auto &primitive = mesh.primitives[0];

            // All of the accessors are mapped to the same buffer vertex view
            const auto &vertexAccessor = model.accessors[primitive.attributes["POSITION"]];
            const auto &vertexView = model.bufferViews[vertexAccessor.bufferView];
            const auto &vertexBuffer = model.buffers[vertexView.buffer];

            const auto &indexAccessor = model.accessors[primitive.indices];
            const auto &indexView = model.bufferViews[indexAccessor.bufferView];
            const auto &indexBuffer = model.buffers[indexView.buffer];

            qInfo() << "- Importing mesh of" << vertexAccessor.count << "vertices and" << indexAccessor.count << "indices.";

            auto vertexData = (glm::vec3 *)(&vertexBuffer.data.at(0) + vertexView.byteOffset);

            std::vector<Vertex> newVertices;
            for (int i = 0; i < vertexAccessor.count; i++) {
                // Replace position data
                auto vertex = existingModel.lods[lodNumber].parts[partNumber].vertices[i];
                vertex.position[0] = vertexData[i].x;
                vertex.position[1] = vertexData[i].y;
                vertex.position[2] = vertexData[i].z;

                newVertices.push_back(vertex);
            }

            auto indexData = (const uint16_t *)(&indexBuffer.data.at(0) + indexView.byteOffset);

            physis_mdl_replace_vertices(&existingModel, lodNumber, partNumber, vertexAccessor.count, newVertices.data(), indexAccessor.count, indexData);
        }
    }

    qInfo() << "Successfully imported model!";
}