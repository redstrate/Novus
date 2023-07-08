#include "gearlistwidget.h"

#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>
#include <magic_enum.hpp>

#include "gearlistmodel.h"

GearListWidget::GearListWidget(GameData* data, QWidget* parent) : data(data) {
    auto layout = new QVBoxLayout();
    setLayout(layout);

    auto searchModel = new QSortFilterProxyModel();
    searchModel->setRecursiveFilteringEnabled(true);
    searchModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

    auto searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Search...");
    searchEdit->setClearButtonEnabled(true);
    connect(searchEdit, &QLineEdit::textChanged, this, [=](const QString& text) {
        searchModel->setFilterRegularExpression(text);
    });
    layout->addWidget(searchEdit);

    auto originalModel = new GearListModel(data);
    searchModel->setSourceModel(originalModel);

    listWidget = new QTreeView();
    listWidget->setModel(searchModel);

    connect(listWidget, &QTreeView::clicked, [this, searchModel, originalModel](const QModelIndex& item) {
        if (auto gear = originalModel->getGearFromIndex(searchModel->mapToSource(item))) {
            Q_EMIT gearSelected(*gear);
        }
    });

    layout->addWidget(listWidget);
}

#include "moc_gearlistwidget.cpp"