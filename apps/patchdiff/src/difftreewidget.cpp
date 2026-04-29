// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "difftreewidget.h"

#include <KLocalizedString>
#include <QClipboard>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QMenu>
#include <QTimer>

DiffTreeWidget::DiffTreeWidget(HashDatabase &database, physis_SqPackResource *data, QWidget *parent)
    : QWidget(parent)
    , data(data)
    , m_database(database)
{
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    m_searchModel = new QSortFilterProxyModel();
    m_searchModel->setRecursiveFilteringEnabled(true);
    m_searchModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

    m_searchEdit = new QLineEdit();

    auto searchTimer = new QTimer();
    searchTimer->setSingleShot(true);
    searchTimer->setInterval(1500);
    connect(searchTimer, &QTimer::timeout, m_searchModel, [this] {
        m_searchModel->setFilterFixedString(m_searchEdit->text());
    });

    m_searchEdit->setPlaceholderText(i18nc("@info:placeholder", "Search…"));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::BottomEdge}));
    connect(m_searchEdit, &QLineEdit::textChanged, searchTimer, [searchTimer] {
        searchTimer->start();
    });
    layout->addWidget(m_searchEdit);

    m_treeWidget = new QTreeView();
    m_treeWidget->setModel(m_searchModel);
    layout->addWidget(m_treeWidget);

    connect(m_treeWidget, &QTreeView::activated, [this](const QModelIndex &item) {
        if (item.isValid()) {
            const auto buffer = m_searchModel->data(item, DiffTreeModel::CustomRoles::BufferRole).value<physis_Buffer>();
            Q_EMIT bufferSelected(buffer);
        }
    });

    refreshModel();
}

void DiffTreeWidget::refreshModel()
{
    // TODO: this should really be handled by the proxy
    m_fileModel = new DiffTreeModel(m_database, data);
    m_searchModel->setSourceModel(m_fileModel);
}

void DiffTreeWidget::openPatch(const QString &path)
{
    m_fileModel->openPatch(path);
}

#include "moc_difftreewidget.cpp"
