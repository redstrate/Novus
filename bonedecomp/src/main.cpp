// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <magic_enum.hpp>

#include "gamedata.h"
#include "havokxmlparser.h"
#include "glm/gtc/type_ptr.hpp"

int findIndexOf(Skeleton skeleton, Bone *pBone) {
    if(pBone == nullptr)
        return -1;

    for(int i = 0; i < skeleton.bones.size(); i++) {
        if(skeleton.bones[i].name == pBone->name)
            return i;
    }

    return -1;
}

QString matrixToString(glm::mat4 matrix) {
    QString str = "[";
    for(int i = 0; i < 16; i++) {
        float f = glm::value_ptr(matrix)[i];
        if(f > 0.999 && f < 1) {
            str += QString::number(1.0f);
        } else if(f > -0.001 && f < 0) {
            str += QString::number(0.0f);
        } else if(f < 0.001 && f > 0) {
            str += QString::number(0.0f);
        } else {
            str += QString::number(f);
        }

        if(i < 15)
            str += ",";
    }

    str += "]";

    return str;
}

void calculate_bone_inverse_pose(Skeleton& skeleton, Bone& bone, Bone* parent_bone) {
    const glm::mat4 parentMatrix = parent_bone == nullptr ? glm::mat4(1.0f) : parent_bone->inversePose;

    glm::mat4 local(1.0f);
    local = glm::translate(local, glm::vec3(bone.position[0], bone.position[1], bone.position[2]));
    local *= glm::mat4_cast(glm::quat(bone.rotation[3], bone.rotation[0], bone.rotation[1], bone.rotation[2]));
    local = glm::scale(local, glm::vec3(bone.scale[0], bone.scale[1], bone.scale[2]));

    bone.inversePose = parentMatrix * local;
    bone.finalTransform = local;

    for(auto& b : skeleton.bones) {
        if(b.parent != nullptr && b.parent->name == bone.name)
            calculate_bone_inverse_pose(skeleton, b, &bone);
    }
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    GameData data(argv[1]);

    fmt::print("Welcome to bonedecomp!\n");
    fmt::print("This tool produces JSON skel and deform files compatible with TexTools.");
    fmt::print("You will NEED an AssetCC.exe or compatible program to use this tool.");
    fmt::print("We will start by extracting the skeleton data for the known races.\n");

    for(auto [race, name] : magic_enum::enum_entries<Race>()) {
        fmt::print("Exporting {}...\n", name);
        data.extractSkeleton(race);

        fmt::print("Now converting havok data to JSON...\n");

        const std::string outputName = fmt::format("skl_c{race:04d}b0001.sklb",
                                                   fmt::arg("race", get_race_id(race)));

        const std::string xmlName = fmt::format("skl_c{race:04d}b0001.xml",
                                                   fmt::arg("race", get_race_id(race)));

        QProcess process;
        process.execute("wine", {"assetcc2.exe", outputName.c_str(), xmlName.c_str()});

        process.waitForFinished();

        Skeleton skeleton = parseHavokXML(xmlName);
        calculate_bone_inverse_pose(skeleton, *skeleton.root_bone, nullptr);
        for(auto& bone : skeleton.bones) {
            bone.inversePose = glm::inverse(bone.inversePose);
        }

        QJsonDocument document;

        QJsonArray array;

        for(int i = 0; i < skeleton.bones.size(); i++) {
            QJsonObject boneObject;
            boneObject["BoneName"] = skeleton.bones[i].name.c_str();
            boneObject["BoneNumber"] = i;
            boneObject["BoneParent"] = findIndexOf(skeleton, skeleton.bones[i].parent);
            boneObject["InversePoseMatrix"] = matrixToString(skeleton.bones[i].inversePose);
            boneObject["PoseMatrix"] = matrixToString(skeleton.bones[i].finalTransform);

            array.push_back(boneObject);
        }

        const std::string jsonName = fmt::format("c{race:04d}b0001.skel",
                                                fmt::arg("race", get_race_id(race)));

        document.setArray(array);

        QFile file(jsonName.c_str());
        file.open(QFile::ReadWrite);
        file.write(document.toJson());
        file.close();
    }

    fmt::print("Complete!");

    return 0;
}