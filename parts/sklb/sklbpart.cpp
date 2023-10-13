// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sklbpart.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "quaternionedit.h"
#include "vec3edit.h"

void addItem(physis_Skeleton &skeleton, physis_Bone &bone, QTreeWidget *widget, QTreeWidgetItem *parent_item = nullptr)
{
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

SklbPart::SklbPart()
{
    auto layout = new QHBoxLayout();
    setLayout(layout);

    boneListWidget = new QTreeWidget();
    boneListWidget->setHeaderLabel(QStringLiteral("Name"));
    boneListWidget->setMaximumWidth(200);

    layout->addWidget(boneListWidget);

    auto transformLayout = new QVBoxLayout();
    layout->addLayout(transformLayout);

    auto transformGroup = new QGroupBox(QStringLiteral("Bone Transform"));
    transformLayout->addWidget(transformGroup);
    auto transformGroupLayout = new QFormLayout();
    transformGroup->setLayout(transformGroupLayout);

    posEdit = new Vector3Edit(currentPosition);
    connect(posEdit, &Vector3Edit::onValueChanged, [this] {
        memcpy(currentEditedBone->position, glm::value_ptr(currentPosition), sizeof(float) * 3);
        Q_EMIT valueChanged();
    });
    transformGroupLayout->addRow(QStringLiteral("Position"), posEdit);

    rotationEdit = new QuaternionEdit(currentRotation);
    connect(rotationEdit, &QuaternionEdit::onValueChanged, [this] {
        memcpy(currentEditedBone->rotation, glm::value_ptr(currentRotation), sizeof(float) * 4);
        Q_EMIT valueChanged();
    });
    transformGroupLayout->addRow(QStringLiteral("Rotation"), rotationEdit);

    scaleEdit = new Vector3Edit(currentScale);
    connect(scaleEdit, &Vector3Edit::onValueChanged, [this] {
        memcpy(currentEditedBone->scale, glm::value_ptr(currentScale), sizeof(float) * 3);
        Q_EMIT valueChanged();
    });
    transformGroupLayout->addRow(QStringLiteral("Scale"), scaleEdit);

    connect(boneListWidget, &QTreeWidget::itemClicked, this, &SklbPart::treeItemClicked);
}

void SklbPart::treeItemClicked(QTreeWidgetItem *item, int column)
{
    for (int i = 0; i < skeleton.num_bones; i++) {
        if (strcmp(skeleton.bones[i].name, item->text(column).toStdString().c_str()) == 0) {
            currentPosition = glm::make_vec3(skeleton.bones[i].position);
            currentRotation = glm::make_quat(skeleton.bones[i].rotation);
            currentScale = glm::make_vec3(skeleton.bones[i].scale);
            currentEditedBone = &skeleton.bones[i];

            posEdit->setVector(currentPosition);
            rotationEdit->setQuat(currentRotation);
            scaleEdit->setVector(currentScale);
        }
    }
}

void SklbPart::clear()
{
    boneListWidget->clear();
}

void SklbPart::load(physis_Skeleton file)
{
    clear();
    addItem(file, *file.root_bone, boneListWidget);
    skeleton = file;
}

#include "moc_sklbpart.cpp"