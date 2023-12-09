// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QSpinBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWidget>
#include <glm/detail/type_quat.hpp>
#include <glm/glm.hpp>
#include <physis.hpp>

#include "quaternionedit.h"
#include "vec3edit.h"

class SklbPart : public QWidget
{
    Q_OBJECT

public:
    explicit SklbPart(QWidget *parent = nullptr);

    void clear();
    void load(physis_Skeleton file);

Q_SIGNALS:
    void valueChanged();

private:
    void treeItemClicked(QTreeWidgetItem *item, int column);

    QTreeWidget *boneListWidget = nullptr;

    glm::vec3 currentPosition;
    glm::quat currentRotation;
    glm::vec3 currentScale;

    physis_Bone *currentEditedBone = nullptr;

    Vector3Edit *posEdit = nullptr;
    QuaternionEdit *rotationEdit = nullptr;
    Vector3Edit *scaleEdit = nullptr;
    physis_Skeleton skeleton;
};