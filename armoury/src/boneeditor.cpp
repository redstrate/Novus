// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "boneeditor.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "gearview.h"
#include "quaternionedit.h"
#include "vec3edit.h"

void addItem(
    physis_Skeleton& skeleton,
    physis_Bone& bone,
    QTreeWidget* widget,
    QTreeWidgetItem* parent_item = nullptr) {
    auto item = new QTreeWidgetItem();
    item->setText(0, QLatin1String(bone.name));

    if (parent_item == nullptr) {
        widget->addTopLevelItem(item);
    } else {
        parent_item->addChild(item);
    }

    for (int i = 0; i < skeleton.num_bones; i++) {
        if (skeleton.bones[i].parent_bone != nullptr && strcmp(skeleton.bones[i].parent_bone->name, bone.name) == 0)
            addItem(skeleton, skeleton.bones[i], widget, item);
    }
}

BoneEditor::BoneEditor(GearView* gearView, QWidget* parent) : gearView(gearView) {
    auto layout = new QHBoxLayout();
    setLayout(layout);

    auto boneListWidget = new QTreeWidget();
    boneListWidget->setHeaderLabel(QStringLiteral("Name"));

    connect(gearView, &GearView::modelReloaded, this, [this, boneListWidget, gearView] {
        boneListWidget->clear();
        addItem(*gearView->part().skeleton, *gearView->part().skeleton->root_bone, boneListWidget);
    });

    boneListWidget->setMaximumWidth(200);

    layout->addWidget(boneListWidget);

    auto transformLayout = new QVBoxLayout();
    layout->addLayout(transformLayout);

    auto transformGroup = new QGroupBox(QStringLiteral("Bone Transform"));
    transformLayout->addWidget(transformGroup);
    auto transformGroupLayout = new QFormLayout();
    transformGroup->setLayout(transformGroupLayout);

    posEdit = new Vector3Edit(currentPosition);
    connect(posEdit, &Vector3Edit::onValueChanged, [this, gearView] {
        memcpy(currentEditedBone->position, glm::value_ptr(currentPosition), sizeof(float) * 3);
        gearView->part().reloadRenderer();
    });
    transformGroupLayout->addRow(QStringLiteral("Position"), posEdit);

    rotationEdit = new QuaternionEdit(currentRotation);
    connect(rotationEdit, &QuaternionEdit::onValueChanged, [this, gearView] {
        memcpy(currentEditedBone->rotation, glm::value_ptr(currentRotation), sizeof(float) * 4);
        gearView->part().reloadRenderer();
    });
    transformGroupLayout->addRow(QStringLiteral("Rotation"), rotationEdit);

    scaleEdit = new Vector3Edit(currentScale);
    connect(scaleEdit, &Vector3Edit::onValueChanged, [this, gearView] {
        memcpy(currentEditedBone->scale, glm::value_ptr(currentScale), sizeof(float) * 3);
        gearView->part().reloadRenderer();
    });
    transformGroupLayout->addRow(QStringLiteral("Scale"), scaleEdit);

    connect(boneListWidget, &QTreeWidget::itemClicked, this, &BoneEditor::treeItemClicked);

    auto raceDeformGroup = new QGroupBox(QStringLiteral("Race Deform"));
    transformLayout->addWidget(raceDeformGroup);
    auto raceDeformGroupLayout = new QFormLayout();
    raceDeformGroup->setLayout(raceDeformGroupLayout);

    raceDeformPosEdit = new Vector3Edit(currentRacePosition);
    raceDeformGroupLayout->addRow(QStringLiteral("Position"), raceDeformPosEdit);

    raceDeformRotationEdit = new QuaternionEdit(currentRaceRotation);
    raceDeformGroupLayout->addRow(QStringLiteral("Rotation"), raceDeformRotationEdit);

    raceDeformScaleEdit = new Vector3Edit(currentRaceScale);
    raceDeformGroupLayout->addRow(QStringLiteral("Scale"), raceDeformScaleEdit);
}

void BoneEditor::treeItemClicked(QTreeWidgetItem* item, int column) {
    auto& skeleton = *gearView->part().skeleton;
    for (int i = 0; i < skeleton.num_bones; i++) {
        if (strcmp(skeleton.bones[i].name, item->text(column).toStdString().c_str()) == 0) {
            currentPosition = glm::make_vec3(skeleton.bones[i].position);
            currentRotation = glm::make_quat(skeleton.bones[i].rotation);
            currentScale = glm::make_vec3(skeleton.bones[i].scale);
            currentEditedBone = &skeleton.bones[i];

            posEdit->setVector(currentPosition);
            rotationEdit->setQuat(currentRotation);
            scaleEdit->setVector(currentScale);

            glm::mat4 transformation; // your transformation matrix.
            glm::vec3 scale;
            glm::quat rotation;
            glm::vec3 translation;
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(
                gearView->part().boneData[i].deformRaceMatrix, scale, rotation, translation, skew, perspective);

            currentRacePosition = translation;
            currentRaceRotation = rotation;
            currentRaceScale = scale;

            raceDeformPosEdit->setVector(currentRacePosition);
            raceDeformRotationEdit->setQuat(currentRaceRotation);
            raceDeformScaleEdit->setVector(currentRaceScale);
        }
    }
}

#include "moc_boneeditor.cpp"