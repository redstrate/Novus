// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "quaternionedit.h"
#include "vec3edit.h"
#include <QSpinBox>
#include <QTreeWidgetItem>
#include <QWidget>
#include <glm/detail/type_quat.hpp>
#include <glm/glm.hpp>
#include <physis.hpp>

class GearView;

class BoneEditor : public QWidget {
    Q_OBJECT

public:
    explicit BoneEditor(GearView* gearView, QWidget* parent = nullptr);

private:
    void treeItemClicked(QTreeWidgetItem* item, int column);

    GearView* gearView;

    glm::vec3 currentPosition;
    glm::quat currentRotation;
    glm::vec3 currentScale;

    glm::vec3 currentRacePosition;
    glm::quat currentRaceRotation;
    glm::vec3 currentRaceScale;

    physis_Bone* currentEditedBone;

    Vector3Edit* posEdit;
    QuaternionEdit* rotationEdit;
    Vector3Edit* scaleEdit;

    Vector3Edit* raceDeformPosEdit;
    QuaternionEdit* raceDeformRotationEdit;
    Vector3Edit* raceDeformScaleEdit;
};
