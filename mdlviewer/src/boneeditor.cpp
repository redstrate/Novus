#include "boneeditor.h"

#include <QTreeWidget>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <glm/gtc/type_ptr.hpp>

#include "vec3edit.h"
#include "quaternionedit.h"
#include "gearview.h"

void addItem(physis_Skeleton& skeleton, physis_Bone& bone, QTreeWidget* widget, QTreeWidgetItem* parent_item = nullptr) {
    auto item = new QTreeWidgetItem();
    item->setText(0, bone.name);

    if(parent_item == nullptr) {
        widget->addTopLevelItem(item);
    } else {
        parent_item->addChild(item);
    }

    for(int i = 0; i < skeleton.num_bones; i++) {
        if(skeleton.bones[i].parent_bone != nullptr && strcmp(skeleton.bones[i].parent_bone->name, bone.name) == 0)
            addItem(skeleton, skeleton.bones[i], widget, item);
    }
}

BoneEditor::BoneEditor(GearView* gearView, QWidget *parent) {
    auto layout = new QHBoxLayout();
    setLayout(layout);

    auto boneListWidget = new QTreeWidget();

    connect(gearView, &GearView::modelReloaded, this, [this, boneListWidget, gearView] {
        boneListWidget->clear();
        addItem(*gearView->part().skeleton, *gearView->part().skeleton->root_bone, boneListWidget);
    });

    boneListWidget->setMaximumWidth(200);

    layout->addWidget(boneListWidget);

    auto transformGroup = new QGroupBox("Bone Transform");
    layout->addWidget(transformGroup);
    auto transformGroupLayout = new QFormLayout();
    transformGroup->setLayout(transformGroupLayout);

    auto posEdit = new Vector3Edit(currentPosition);
    connect(posEdit, &Vector3Edit::onValueChanged, [this, gearView] {
        memcpy(currentEditedBone->position, glm::value_ptr(currentPosition), sizeof(float) * 3);
        gearView->part().reloadRenderer();
    });
    transformGroupLayout->addRow("Position", posEdit);

    auto rotationEdit = new QuaternionEdit(currentRotation);
    connect(rotationEdit, &QuaternionEdit::onValueChanged, [this, gearView] {
        memcpy(currentEditedBone->rotation, glm::value_ptr(currentRotation), sizeof(float) * 4);
        gearView->part().reloadRenderer();
    });
    transformGroupLayout->addRow("Rotation", rotationEdit);

    auto scaleEdit = new Vector3Edit(currentScale);
    connect(scaleEdit, &Vector3Edit::onValueChanged, [this, gearView] {
        memcpy(currentEditedBone->scale, glm::value_ptr(currentScale), sizeof(float) * 3);
        gearView->part().reloadRenderer();
    });
    transformGroupLayout->addRow("Scale", scaleEdit);

    connect(boneListWidget, &QTreeWidget::itemClicked, [this, posEdit, rotationEdit, scaleEdit, gearView](QTreeWidgetItem* item, int column) {
        auto& skeleton = *gearView->part().skeleton;
        for(int i = 0; i < skeleton.num_bones; i++) {
            if(strcmp(skeleton.bones[i].name, item->text(column).toStdString().c_str()) == 0) {
                currentPosition = glm::make_vec3(skeleton.bones[i].position);
                currentRotation = glm::make_quat(skeleton.bones[i].rotation);
                currentScale = glm::make_vec3(skeleton.bones[i].scale);
                currentEditedBone = &skeleton.bones[i];

                posEdit->setVector(currentPosition);
                rotationEdit->setQuat(currentRotation);
                scaleEdit->setVector(currentScale);
            }
        }
    });
}

#include "moc_boneeditor.cpp"