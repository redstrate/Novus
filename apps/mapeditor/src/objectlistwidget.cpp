// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "objectlistwidget.h"

#include <KLocalizedString>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QVBoxLayout>

#include "appstate.h"

ObjectListWidget::ObjectListWidget(AppState *appState, QWidget *parent)
    : QWidget(parent)
    , m_appState(appState)
{
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    auto searchModel = new QSortFilterProxyModel();
    searchModel->setRecursiveFilteringEnabled(true);
    searchModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

    auto searchEdit = new QLineEdit();
    searchEdit->setWhatsThis(i18nc("@info:whatsthis", "Search box for Excel sheet names."));
    searchEdit->setPlaceholderText(i18nc("@info:placeholder", "Search…"));
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::BottomEdge}));
    connect(searchEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        searchModel->setFilterRegularExpression(text);
    });
    layout->addWidget(searchEdit);

    m_originalModel = new QStringListModel();
    searchModel->setSourceModel(m_originalModel);

    listWidget = new QListView();
    listWidget->setWhatsThis(i18nc("@info:whatsthis", "A list of Excel sheet names. Select one to view it's contents."));
    listWidget->setModel(searchModel);

    layout->addWidget(listWidget);

    connect(m_appState, &AppState::mapLoaded, this, &ObjectListWidget::refresh);
}

void ObjectListWidget::refresh()
{
    QStringList list;

    for (int i = 0; i < m_appState->bgGroup.num_chunks; i++) {
        const auto chunk = m_appState->bgGroup.chunks[i];
        for (int j = 0; j < chunk.num_layers; j++) {
            const auto layer = chunk.layers[j];
            for (int z = 0; z < layer.num_objects; z++) {
                const auto object = layer.objects[z];
                const QString name = QString::fromLatin1(object.name);
                if (true) { // TODO: do display names if we have them
                    list << i18n("Unknown (%1)", object.instance_id);
                } else {
                    list << name;
                }
            }
        }
    }

    m_originalModel->setStringList(list);
}

#include "moc_objectlistwidget.cpp"
