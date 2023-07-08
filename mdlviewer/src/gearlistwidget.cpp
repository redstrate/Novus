#include "gearlistwidget.h"

#include <QVBoxLayout>
#include <magic_enum.hpp>

#include "gearlistmodel.h"

GearListWidget::GearListWidget(GameData* data, QWidget* parent) : data(data) {
    auto layout = new QVBoxLayout();
    setLayout(layout);

    auto model = new GearListModel(data);

    listWidget = new QTreeView();
    listWidget->setModel(model);

    connect(listWidget, &QTreeView::clicked, [this, model](const QModelIndex& item) {
        if (auto gear = model->getGearFromIndex(item)) {
            Q_EMIT gearSelected(*gear);
        }
    });

    layout->addWidget(listWidget);
}

#include "moc_gearlistwidget.cpp"