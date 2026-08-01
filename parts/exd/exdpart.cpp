// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exdpart.h"

#include "excelmodel.h"

#include <KLocalizedString>
#include <QCheckBox>
#include <QDesktopServices>
#include <QGraphicsDropShadowEffect>
#include <QScrollBar>
#include <QTableWidget>
#include <QVBoxLayout>
#include <physis.hpp>

#include "magic_enum.hpp"
#include "schema.h"
#include "settings.h"

#include <QActionGroup>
#include <QFileDialog>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QStandardPaths>
#include <qevent.h>

class SearchSettingsPopup : public QDialog
{
    Q_OBJECT

public:
    SearchSettingsPopup(const EXDPart::SearchSettings settings, const QAbstractItemModel *columnModel, QWidget *parent)
        : QDialog(parent, Qt::Popup)
    {
        const auto layout = new QFormLayout();
        setLayout(layout);

        m_columnBox = new QComboBox();
        m_columnBox->addItem(i18n("All Columns"), -1);
        for (int i = 0; i < columnModel->columnCount(); i++) {
            auto name = columnModel->headerData(i, Qt::Orientation::Horizontal, Qt::DisplayRole).toString();
            auto decoration = columnModel->headerData(i, Qt::Orientation::Horizontal, Qt::DecorationRole).value<QIcon>();
            m_columnBox->addItem(decoration, name, i);
        }
        m_columnBox->setCurrentIndex(m_columnBox->findData(settings.column));
        layout->addRow(i18nc("@info:label", "Search In"), m_columnBox);

        m_caseSensitiveCheck = new QCheckBox();
        m_caseSensitiveCheck->setChecked(settings.caseSensitive);
        layout->addRow(i18nc("@info:label", "Case Sensitive"), m_caseSensitiveCheck);

        m_regexCheck = new QCheckBox();
        m_regexCheck->setChecked(settings.enableRegex);
        layout->addRow(i18nc("@info:label", "Enable Regex"), m_regexCheck);
    }

    EXDPart::SearchSettings settings() const
    {
        return EXDPart::SearchSettings{
            .column = m_columnBox->currentData().toInt(),
            .caseSensitive = m_caseSensitiveCheck->isChecked(),
            .enableRegex = m_regexCheck->isChecked(),
        };
    }

private:
    QComboBox *m_columnBox;
    QCheckBox *m_caseSensitiveCheck;
    QCheckBox *m_regexCheck;
};

class ExcelTableView : public QTableView
{
    Q_OBJECT

public:
    explicit ExcelTableView()
    {
        setMouseTracking(true);

        horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(horizontalHeader(), &QHeaderView::customContextMenuRequested, this, [this](const QPoint pos) {
            const auto menu = new QMenu(this);

            const int logicalIndex = horizontalHeader()->logicalIndexAt(pos);
            if (logicalIndex == m_pinnedColumn) {
                const auto pinAction = menu->addAction(QIcon::fromTheme(QStringLiteral("window-unpin-symbolic")), i18n("Unpin"));
                connect(pinAction, &QAction::triggered, this, [this] {
                    initializeFrozenTableView(-1);
                });
            } else {
                const auto pinAction = menu->addAction(QIcon::fromTheme(QStringLiteral("window-pin-symbolic")), i18n("Pin"));
                connect(pinAction, &QAction::triggered, this, [this, logicalIndex] {
                    initializeFrozenTableView(logicalIndex);
                });
            }

            menu->exec(mapToGlobal(pos));
        });
    }

    void initializeFrozenTableView(const int column)
    {
        if (m_frozenTableView) {
            delete m_frozenTableView;
            m_frozenTableView = nullptr;
        }

        m_pinnedColumn = column;

        if (column == -1) {
            return;
        }

        m_frozenTableView = new QTableView(this);
        m_frozenTableView->setModel(model());
        m_frozenTableView->setFocusPolicy(Qt::NoFocus);
        m_frozenTableView->verticalHeader()->hide();
        m_frozenTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        m_frozenTableView->horizontalHeader()->setSectionsClickable(false);
        m_frozenTableView->horizontalHeader()->setSectionsMovable(false);

        m_frozenTableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_frozenTableView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, [this](const QPoint pos) {
            const auto menu = new QMenu(this);

            const auto pinAction = menu->addAction(QIcon::fromTheme(QStringLiteral("window-unpin-symbolic")), i18n("Unpin"));
            connect(pinAction, &QAction::triggered, this, [this] {
                initializeFrozenTableView(-1);
            });

            if (m_pinnedOnRight) {
                const auto moveAction = menu->addAction(QIcon::fromTheme(QStringLiteral("arrow-left-symbolic")), i18n("Move Left"));
                connect(moveAction, &QAction::triggered, this, [this] {
                    m_pinnedOnRight = false;
                    updateFrozenTableGeometry();
                    updateFrozenTableShadow();
                });
            } else {
                const auto moveAction = menu->addAction(QIcon::fromTheme(QStringLiteral("arrow-right-symbolic")), i18n("Move Right"));
                connect(moveAction, &QAction::triggered, this, [this] {
                    m_pinnedOnRight = true;
                    updateFrozenTableGeometry();
                    updateFrozenTableShadow();
                });
            }

            menu->exec(m_frozenTableView->mapToGlobal(pos));
        });

        viewport()->stackUnder(m_frozenTableView);

        m_frozenTableView->setSelectionModel(selectionModel());
        for (int col = 0; col < model()->columnCount(); ++col) {
            m_frozenTableView->setColumnHidden(col, col != column);
        }

        m_frozenTableView->setColumnWidth(m_pinnedColumn, columnWidth(m_pinnedColumn));
        m_frozenTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_frozenTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_frozenTableView->setAlternatingRowColors(alternatingRowColors());

        // make it read-only for now
        m_frozenTableView->setEditTriggers({});
        m_frozenTableView->setSelectionMode(NoSelection);

        m_frozenTableView->show();

        // Add a drop shadow to make it even more obvious
        updateFrozenTableShadow();

        connect(horizontalHeader(), &QHeaderView::sectionResized, this, &ExcelTableView::updateSectionWidth);
        connect(verticalHeader(), &QHeaderView::sectionResized, this, &ExcelTableView::updateSectionHeight);

        connect(m_frozenTableView->verticalScrollBar(), &QAbstractSlider::valueChanged, verticalScrollBar(), &QAbstractSlider::setValue);
        connect(verticalScrollBar(), &QAbstractSlider::valueChanged, m_frozenTableView->verticalScrollBar(), &QAbstractSlider::setValue);

        // update initial geometry
        updateFrozenTableGeometry();
    }

protected:
    void mouseMoveEvent(QMouseEvent *event) override
    {
        const auto index = indexAt(event->pos());
        if (!index.data(ExcelModel::ResolvedSheetRole).isNull()) {
            setCursor(Qt::PointingHandCursor);
        } else {
            unsetCursor();
        }

        QTableView::mouseMoveEvent(event);
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        const auto index = indexAt(event->pos());
        if (event->button() == Qt::LeftButton && !index.data(ExcelModel::ResolvedSheetRole).isNull()) {
            Q_EMIT requestJump(index.data(ExcelModel::ResolvedSheetRole).toString(), QString::number(index.data(ExcelModel::ResolvedRowRole).toInt()));
            return;
        }

        QTableView::mousePressEvent(event);
    }

    void resizeEvent(QResizeEvent *event) override
    {
        updateFrozenTableGeometry();
        QTableView::resizeEvent(event);
    }

Q_SIGNALS:
    void requestJump(const QString &name, const QString &rowQuery);

private:
    void updateFrozenTableGeometry() const
    {
        if (!m_frozenTableView) {
            return;
        }

        int x = 0;
        if (m_pinnedOnRight) {
            x = width() - columnWidth(m_pinnedColumn) - verticalScrollBar()->width();
        } else {
            x = verticalHeader()->width() + frameWidth();
        }
        m_frozenTableView->setGeometry(x, frameWidth(), columnWidth(m_pinnedColumn), viewport()->height() + horizontalHeader()->height());
    }

    void updateFrozenTableShadow() const
    {
        const auto effect = new QGraphicsDropShadowEffect();
        effect->setBlurRadius(5);
        if (m_pinnedOnRight) {
            effect->setXOffset(-5);
        } else {
            effect->setXOffset(5);
        }
        effect->setYOffset(0);
        effect->setColor(palette().shadow().color());

        m_frozenTableView->setGraphicsEffect(effect);
    }

    void updateSectionWidth(const int logicalIndex, const int oldSize, const int newSize) const
    {
        Q_UNUSED(oldSize)

        if (!m_frozenTableView) {
            return;
        }

        if (logicalIndex == m_pinnedColumn) {
            m_frozenTableView->setColumnWidth(m_pinnedColumn, newSize);
            updateFrozenTableGeometry();
        }
    }

    void updateSectionHeight(const int logicalIndex, const int oldSize, const int newSize) const
    {
        Q_UNUSED(oldSize)

        if (!m_frozenTableView) {
            return;
        }

        m_frozenTableView->setRowHeight(logicalIndex, newSize);
    }

    QTableView *m_frozenTableView = nullptr;
    int m_pinnedColumn = -1;
    bool m_pinnedOnRight = true;
};

EXDPart::EXDPart(FileCache &cache, AbstractExcelResolver *resolver, QWidget *parent)
    : QWidget(parent)
    , m_cache(cache)
    , m_preferredLanguage(getLanguage())
    , m_resolver(resolver)
{
    const auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    m_filterEdit = new QLineEdit();
    m_filterEdit->setClearButtonEnabled(true);
    m_filterEdit->setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::BottomEdge}));
    connect(m_filterEdit, &QLineEdit::textEdited, this, &EXDPart::filterData);
    layout->addWidget(m_filterEdit);

    const auto searchSettingsAction = new QAction(QIcon::fromTheme(QStringLiteral("settings-configure-symbolic")), i18n("Search Settings"), this);
    connect(searchSettingsAction, &QAction::triggered, this, [this] {
        const auto tableWidget = qobject_cast<QTableView *>(m_pageTabWidget->widget(0));
        if (!tableWidget) {
            return;
        }

        auto popup = new SearchSettingsPopup(m_searchSettings, tableWidget->horizontalHeader()->model(), this);
        connect(popup, &QDialog::finished, this, [this, popup] {
            setSearchSettings(popup->settings());
            popup->deleteLater();
        });

        // Move to bottom-right of the filter bar
        auto position = m_filterEdit->mapToGlobal(m_filterEdit->pos());
        position.setY(position.y() + m_filterEdit->height());
        position.setX(position.x() + m_filterEdit->width() - popup->width());
        popup->move(position);

        popup->show();
    });
    m_filterEdit->addAction(searchSettingsAction, QLineEdit::TrailingPosition);

    m_pageTabWidget = new QTabWidget();
    m_pageTabWidget->setTabPosition(QTabWidget::TabPosition::South);
    m_pageTabWidget->setDocumentMode(true); // hide borders
    layout->addWidget(m_pageTabWidget);

    m_selectLanguage = new QAction(i18nc("@action:inmenu", "Language"), this);
    m_selectLanguage->setEnabled(false);
    m_selectLanguage->setIcon(QIcon::fromTheme(QStringLiteral("languages-symbolic")));

    m_languageMenu = new QMenu(this);
    m_selectLanguage->setMenu(m_languageMenu);

    m_languageGroup = new QActionGroup(this);
    m_languageGroup->setExclusive(true);

    m_saveCsvAction = new QAction(QIcon::fromTheme(QStringLiteral("text-csv")), i18n("Save CSV…"), this);
    connect(m_saveCsvAction, &QAction::triggered, this, [this] {
        const QString savePath = getSaveFileName(this, QStringLiteral("ExcelEditorCSVFile"), i18nc("@title:window", "Save CSV"), {}, QStringLiteral("*.csv"));
        if (!savePath.isEmpty()) {
            QString csvString;

            for (uint32_t i = 0; i < m_exh.page_count; i++) {
                const auto tableWidget = qobject_cast<QTableView *>(m_pageTabWidget->widget(i));
                Q_ASSERT(tableWidget);

                const auto model = tableWidget->model();
                const int rows = model->rowCount();
                const int columns = model->columnCount();

                // Header
                if (i == 0) {
                    for (int j = 0; j < columns; j++) {
                        csvString += model->headerData(j, Qt::Horizontal).toString();
                        csvString += QStringLiteral(", ");
                    }
                    csvString += QStringLiteral("\n");
                }

                // Data
                for (int k = 0; k < rows; k++) {
                    for (int j = 0; j < columns; j++) {
                        csvString += model->data(model->index(k, j)).toString();
                        csvString += QStringLiteral(", ");
                    }
                    csvString += QStringLiteral("\n");
                }
            }

            QFile csvFile(savePath);
            if (csvFile.open(QIODevice::WriteOnly)) {
                QTextStream out(&csvFile);
                out << csvString;
            }
        }
    });
}

EXDPart::~EXDPart()
{
    physis_sqpack_free_excel_sheet(&m_sheet);
    physis_exh_free(&m_exh);
}

void EXDPart::loadSheet(const QString &name, const physis_Buffer buffer)
{
    if (buffer.size == 0) {
        return;
    }

    m_name = name;

    physis_exh_free(&m_exh); // Free existing
    m_exh = physis_exh_parse(m_cache.platform(), buffer);

    loadTables();

    // update language selection
    const auto languages = availableLanguages();
    if (languages.isEmpty()) {
        m_selectLanguage->setEnabled(false);
    } else {
        m_selectLanguage->setEnabled(true);

        m_languageMenu->clear();
        for (const auto &[name, language] : languages) {
            auto languageAction = new QAction(name, this);
            languageAction->setActionGroup(m_languageGroup);
            languageAction->setData(static_cast<int>(language));
            languageAction->setCheckable(true);
            languageAction->setChecked(language == preferredLanguage());

            connect(languageAction, &QAction::triggered, this, [this, languageAction](const bool checked) {
                if (checked) {
                    setPreferredLanguage(static_cast<Language>(languageAction->data().toInt()));
                }
            });

            m_languageMenu->addAction(languageAction);
        }
    }
}

void EXDPart::goToRow(const QString &query) const
{
    for (uint32_t i = 0; i < m_exh.page_count; i++) {
        const auto tableWidget = qobject_cast<QTableView *>(m_pageTabWidget->widget(i));
        Q_ASSERT(tableWidget);
        if (!tableWidget)
            continue;

        for (int row = 0; row < tableWidget->model()->rowCount(); row++) {
            const auto headerItem = tableWidget->model()->headerData(row, Qt::Vertical).toString();
            if (headerItem == query) {
                m_pageTabWidget->setCurrentIndex(i);
                tableWidget->selectRow(row);
                return;
            }
        }
    }
}

void EXDPart::resetSorting() const
{
    const auto tableWidget = qobject_cast<QTableView *>(m_pageTabWidget->currentWidget());
    Q_ASSERT(tableWidget);

    tableWidget->sortByColumn(-1, Qt::AscendingOrder);
}

void EXDPart::clear() const
{
    m_pageTabWidget->clear();
}

void EXDPart::focusFilterField() const
{
    m_filterEdit->setFocus(Qt::FocusReason::ShortcutFocusReason);
}

void EXDPart::setReadOnly(const bool readOnly)
{
    m_readOnly = readOnly;
}

void EXDPart::save() const
{
    const auto mods = getGameMods();
    if (mods.isEmpty()) {
        qWarning() << "No mod to write a file to!";
        return;
    }

    const QDir targetDir = mods.constFirst().path;
    const QDir exdDir = targetDir.absoluteFilePath(QStringLiteral("exd"));

    for (uint32_t i = 0; i < m_exh.page_count; i++) {
        const QString filename =
            QString::fromUtf8(physis_exd_calculate_filename(m_name.toStdString().c_str(), &m_exh, getSuitableLanguage(m_exh), i)).toLower();
        const auto buffer = physis_sqpack_write_sheet_page_to_buffer(&m_sheet.pages[i], &m_exh);

        QFile file(exdDir.absoluteFilePath(filename));
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reinterpret_cast<const char *>(buffer.data), buffer.size);
        }
    }
}

void EXDPart::editSchema() const
{
    QDesktopServices::openUrl(QUrl(Schema::getPath(m_name)));
}

void EXDPart::setPreferredLanguage(const Language language)
{
    if (language != m_preferredLanguage) {
        m_preferredLanguage = language;
        loadTables();
    }
}

Language EXDPart::preferredLanguage() const
{
    return m_preferredLanguage;
}

QList<QPair<QString, Language>> EXDPart::availableLanguages() const
{
    QList<QPair<QString, Language>> languages;

    for (unsigned int i = 0; i < m_exh.language_count; i++) {
        // Don't add None to the combo box, the reason for this is because
        // many localized sheets *report* this language but it's usually empty and useless.
        const auto language = m_exh.languages[i];
        if (language == Language::None) {
            continue;
        }

        const auto itemText = QString::fromUtf8(magic_enum::enum_name(language));
        languages.push_back({itemText, language});
    }

    return languages;
}

QAction *EXDPart::selectLanguageAction() const
{
    return m_selectLanguage;
}

QAction *EXDPart::saveCsvAction() const
{
    return m_saveCsvAction;
}

QString EXDPart::name() const
{
    return m_name;
}

QString EXDPart::selectedRow() const
{
    for (uint32_t i = 0; i < m_exh.page_count; i++) {
        const auto tableWidget = qobject_cast<QTableView *>(m_pageTabWidget->widget(i));
        if (!tableWidget) {
            continue;
        }

        if (tableWidget->currentIndex().isValid()) {
            return tableWidget->model()->headerData(tableWidget->currentIndex().row(), Qt::Vertical).toString();
        }
    }

    return {};
}

bool EXDPart::isModified() const
{
    return m_modified;
}

void EXDPart::loadTables()
{
    const Schema schema(Schema::getPath(m_name));

    clear();

    physis_sqpack_free_excel_sheet(&m_sheet); // Free existing
    m_sheet = m_cache.readExcelSheet(m_name, &m_exh, getSuitableLanguage(m_exh));

    for (uint32_t i = 0; i < m_sheet.page_count; i++) {
        const auto tableWidget = new ExcelTableView();
        connect(tableWidget, &ExcelTableView::requestJump, this, &EXDPart::requestJump);

        const auto excelModel = new ExcelModel(m_exh, m_sheet.pages[i], schema, m_resolver, getSuitableLanguage(m_exh), this);
        connect(excelModel, &ExcelModel::modified, this, [this] {
            m_modified = true;
            Q_EMIT modified();
        });

        // Wrap it in a sortfilterproxy so we get column sorting for free
        const auto proxyModel = new QSortFilterProxyModel(this);
        proxyModel->setSourceModel(excelModel);

        tableWidget->setModel(proxyModel);
        if (m_readOnly) {
            tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        } else {
            tableWidget->setEditTriggers(QAbstractItemView::AllEditTriggers);
        }
        tableWidget->resizeColumnsToContents();
        tableWidget->setAlternatingRowColors(true);
        tableWidget->setSortingEnabled(true);
        tableWidget->horizontalHeader()->setSortIndicatorClearable(true);

        // We have to call sort(-1) here because the above call to enable sorting sorts by the first column
        tableWidget->sortByColumn(-1, Qt::SortOrder::AscendingOrder);

        m_pageTabWidget->addTab(tableWidget, i18nc("@title:tab", "Page %1", i));
    }

    // Reset search column to the display field, if applicable.
    // We do this as searching *all* columns is very slow, and that's a bad default experience.
    if (schema.displayFieldIndex().has_value()) {
        const auto tableWidget = qobject_cast<QTableView *>(m_pageTabWidget->widget(0));
        if (!tableWidget) {
            return;
        }

        const auto model = static_cast<QSortFilterProxyModel *>(tableWidget->model());
        const auto sourceModel = static_cast<ExcelModel *>(model->sourceModel());
        m_searchSettings.column = sourceModel->displayFieldColumn();
    } else {
        m_searchSettings.column = -1;
    }
    setSearchSettings(m_searchSettings); // Apply to new models

    // Expand the tabs and hide the tab bar if there's only one page
    // (it effectively makes the tab bar useless, so why show it?)
    m_pageTabWidget->tabBar()->setExpanding(true);
    m_pageTabWidget->tabBar()->setVisible(m_exh.page_count > 1);
}

void EXDPart::filterData(const QString &pattern) const
{
    for (uint32_t i = 0; i < m_exh.page_count; i++) {
        const auto tableWidget = qobject_cast<QTableView *>(m_pageTabWidget->widget(i));
        if (!tableWidget) {
            continue;
        }

        const auto model = qobject_cast<QSortFilterProxyModel *>(tableWidget->model());

        if (m_searchSettings.enableRegex) {
            model->setFilterRegularExpression(pattern);
        } else {
            model->setFilterFixedString(pattern);
        }
    }
}

void EXDPart::setSearchSettings(const SearchSettings newSettings)
{
    m_searchSettings = newSettings;

    // Start with generic text
    m_filterEdit->setPlaceholderText(i18nc("@info:placeholder", "Filter all data…"));

    for (uint32_t i = 0; i < m_exh.page_count; i++) {
        const auto tableWidget = qobject_cast<QTableView *>(m_pageTabWidget->widget(i));
        if (!tableWidget) {
            continue;
        }

        const auto model = qobject_cast<QSortFilterProxyModel *>(tableWidget->model());
        model->setFilterKeyColumn(m_searchSettings.column);
        model->setFilterCaseSensitivity(m_searchSettings.caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);

        if (newSettings.column != -1) {
            m_filterEdit->setPlaceholderText(
                i18nc("@info:placeholder", "Filter %1…").arg(model->headerData(newSettings.column, Qt::Horizontal, Qt::DisplayRole).toString()));
        }
    }
}

Language EXDPart::getSuitableLanguage(const physis_EXH &pExh) const
{
    // Find the preferred language first
    for (uint32_t i = 0; i < pExh.language_count; i++) {
        if (pExh.languages[i] == m_preferredLanguage) {
            return m_preferredLanguage;
        }
    }

    // Fallback to None
    for (uint32_t i = 0; i < pExh.language_count; i++) {
        if (pExh.languages[i] == Language::None) {
            return Language::None;
        }
    }

    // Then the default language
    return getLanguage();
}

#include "exdpart.moc"
#include "moc_exdpart.cpp"
