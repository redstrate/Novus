#include "mainwindow.h"

#include <QHBoxLayout>
#include <QTableWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QTimer>

#include <QPushButton>
#include <QFileDialog>
#include <magic_enum.hpp>
#include <QMenuBar>
#include <QAction>
#include <glm/gtc/type_ptr.hpp>
#include <QTreeWidget>
#include <physis.hpp>

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

MainWindow::MainWindow(GameData* in_data) : data(*in_data) {
    setWindowTitle("mdlviewer");
    setMinimumSize(QSize(800, 600));

    auto fileMenu = menuBar()->addMenu("File");

    // TODO: move to a dedicated mdlview?
    /*auto openMDLFile = fileMenu->addAction("Open MDL...");
    connect(openMDLFile, &QAction::triggered, [=] {
        auto fileName = QFileDialog::getOpenFileName(nullptr,
                                                     "Open MDL File",
                                                     "~",
                                                     "FFXIV Model File (*.mdl)");

        auto buffer = physis_read_file(fileName.toStdString().c_str());

        loadedGear.model = physis_mdl_parse(buffer.size, buffer.data);

        reloadGearAppearance();
    });*/

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QHBoxLayout();
    dummyWidget->setLayout(layout);

    // smallclothes body
    {
        GearInfo info = {};
        info.name = "Smallclothes Body";
        info.slot = Slot::Body;

        gears.push_back(info);
    }

    // smallclothes legs
    {
        GearInfo info = {};
        info.name = "Smallclothes Legs";
        info.slot = Slot::Legs;

        gears.push_back(info);
    }

    auto exh = physis_gamedata_read_excel_sheet_header(&data, "Item");
    auto exd = physis_gamedata_read_excel_sheet(&data, "Item", &exh, Language::English, 1);

    for(int i = 0; i < exd.row_count; i++) {
        const auto row = exd.row_data[i];
        auto primaryModel = row.column_data[47].u_int64._0;
        auto secondaryModel = row.column_data[48].u_int64._0;

        int16_t parts[4];
        memcpy(parts, &primaryModel, sizeof(int16_t) * 4);

        GearInfo info = {};
        info.name = row.column_data[9].string._0;
        info.slot = physis_slot_from_id(row.column_data[17].u_int8._0);
        info.modelInfo.primaryID = parts[0];

        gears.push_back(info);
    }

    auto listWidget = new QListWidget();
    for(auto gear : gears)
        listWidget->addItem(gear.name.c_str());

    listWidget->setMaximumWidth(200);

    layout->addWidget(listWidget);

    gearView = new SingleGearView(&data);
    connect(gearView, &SingleGearView::addToFullModelViewer, this, [=](GearInfo& info) {
        fullModelViewer->addGear(info);
    });
    layout->addWidget(gearView);

    connect(listWidget, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
        for(auto& gear : gears) {
            if(gear.name == item->text().toStdString()) {
                gearView->setGear(gear);
                return;
            }
        }
    });

    // TODO: reintroduce into SingleGearView
    /*auto boneListWidget = new QTreeWidget();
    for(auto& bone : extraBone) {
        bone.inversePose = glm::inverse(bone.inversePose);
    }

    addItem(skeleton, *skeleton.root_bone, boneListWidget);

    boneListWidget->setMaximumWidth(200);

    connect(boneListWidget, &QTreeWidget::itemClicked, [this](QTreeWidgetItem* item, int column) {
        for(int i = 0; i < skeleton.num_bones; i++) {
            if(strcmp(skeleton.bones[i].name, item->text(column).toStdString().c_str()) == 0) {
                currentScale = glm::make_vec3(skeleton.bones[i].scale);
                currentEditedBone = &skeleton.bones[i];
            }
        }
    });

    layout->addWidget(boneListWidget);

    Vector3Edit* scaleEdit = new Vector3Edit(currentScale);
    connect(scaleEdit, &Vector3Edit::onValueChanged, [this] {
        memcpy(currentEditedBone->scale, glm::value_ptr(currentScale), sizeof(float) * 3);
        reloadGearAppearance();
    });
    layout->addWidget(scaleEdit);*/

    fullModelViewer = new FullModelViewer(&data);
    fullModelViewer->show();
}