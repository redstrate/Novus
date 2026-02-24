// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "maplistwidget.h"

#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QVBoxLayout>

MapListWidget::MapListWidget(physis_SqPackResource *data, QWidget *parent)
    : QDialog(parent)
    , data(data)
{
    setModal(true);
    setMinimumSize(QSize(640, 480));

    auto layout = new QVBoxLayout();
    setLayout(layout);

    m_searchModel = new QSortFilterProxyModel();
    m_searchModel->setRecursiveFilteringEnabled(true);
    m_searchModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

    auto searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText(i18nc("@info:placeholder", "Search…"));
    searchEdit->setClearButtonEnabled(true);
    connect(searchEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        m_searchModel->setFilterRegularExpression(text);
    });
    layout->addWidget(searchEdit);

    auto originalModel = new QStandardItemModel();
    m_searchModel->setSourceModel(originalModel);

    auto nameExh = physis_exh_parse(data->platform, physis_sqpack_read(data, "exd/PlaceName.exh"));
    auto territoryExh = physis_exh_parse(data->platform, physis_sqpack_read(data, "exd/TerritoryType.exh"));

    auto nameSheet = physis_sqpack_read_excel_sheet(data, "PlaceName", &nameExh, Language::English);
    auto territorySheet = physis_sqpack_read_excel_sheet(data, "TerritoryType", &territoryExh, Language::None);

    // TODO: figure out why row_count in EXH is wrong?!
    for (uint32_t i = 0; i < territoryExh.pages[0].row_count; i++) {
        auto territoryExdRow = physis_excel_get_row(&territorySheet, i); // TODO: free, use all rows
        if (territoryExdRow.columns) {
            const char *bg = territoryExdRow.columns[1].string._0;
            if (strlen(bg) == 0) {
                continue;
            }

            int placeRegionKey = territoryExdRow.columns[3].u_int16._0;
            auto regionExdRow = physis_excel_get_row(&nameSheet, placeRegionKey); // TODO: free, use all rows
            const char *placeRegion = regionExdRow.columns[0].string._0;

            int placeZoneKey = territoryExdRow.columns[4].u_int16._0;
            auto zoneExdRow = physis_excel_get_row(&nameSheet, placeZoneKey); // TODO: free, use all rows
            const char *placeZone = zoneExdRow.columns[0].string._0;

            int placeNameKey = territoryExdRow.columns[5].u_int16._0;
            auto nameExdRow = physis_excel_get_row(&nameSheet, placeNameKey); // TODO: free, use all rows
            const char *placeName = nameExdRow.columns[0].string._0;

            int contentFinderCondition = territoryExdRow.columns[10].u_int16._0;
            int territoryIntendedUse = territoryExdRow.columns[9].u_int8._0;
            QString territoryIntendedUseString = i18n("Unknown TIU (%1)").arg(territoryIntendedUse);
            // NOTE: Please keep in sync with Kawari!
            switch (territoryIntendedUse) {
            case 0:
                territoryIntendedUseString = i18n("Town");
                break;
            case 1:
                territoryIntendedUseString = i18n("OpenWorld");
                break;
            case 2:
                territoryIntendedUseString = i18n("Inn");
                break;
            case 3:
                territoryIntendedUseString = i18n("Dungeon");
                break;
            case 4:
                territoryIntendedUseString = i18n("VariantDungeon");
                break;
            case 5:
                territoryIntendedUseString = i18n("Jail");
                break;
            case 6:
                territoryIntendedUseString = i18n("OpeningArea");
                break;
            case 7:
                territoryIntendedUseString = i18n("LobbyArea");
                break;
            case 8:
                territoryIntendedUseString = i18n("AllianceRaid");
                break;
            case 9:
                territoryIntendedUseString = i18n("OpenWorldInstanceBattle");
                break;
            case 10:
                territoryIntendedUseString = i18n("Trial");
                break;
            case 11:
                territoryIntendedUseString = i18n("Unk100");
                break;
            case 12:
                territoryIntendedUseString = i18n("Unk110");
                break;
            case 13:
                territoryIntendedUseString = i18n("HousingOutdoor");
                break;
            case 14:
                territoryIntendedUseString = i18n("HousingIndoor");
                break;
            case 15:
                territoryIntendedUseString = i18n("SoloOverworldInstance");
                break;
            case 16:
                territoryIntendedUseString = i18n("Raid1");
                break;
            case 17:
                territoryIntendedUseString = i18n("Raid2");
                break;
            case 18:
                territoryIntendedUseString = i18n("Frontline");
                break;
            case 19:
                territoryIntendedUseString = i18n("Unk120");
                break;
            case 20:
                territoryIntendedUseString = i18n("ChocoboRacing");
                break;
            case 21:
                territoryIntendedUseString = i18n("IshgardRestoration");
                break;
            case 22:
                territoryIntendedUseString = i18n("Wedding");
                break;
            case 23:
                territoryIntendedUseString = i18n("GoldSaucer");
                break;
            case 26:
                territoryIntendedUseString = i18n("ExploratoryMissions");
                break;
            case 27:
                territoryIntendedUseString = i18n("HallOfTheNovice");
                break;
            case 28:
                territoryIntendedUseString = i18n("CrystallineConflict");
                break;
            case 29:
                territoryIntendedUseString = i18n("SoloDuty");
                break;
            case 30:
                territoryIntendedUseString = i18n("FreeCompanyGarrison");
                break;
            case 31:
                territoryIntendedUseString = i18n("DeepDungeon");
                break;
            case 32:
                territoryIntendedUseString = i18n("Seasonal");
                break;
            case 33:
                territoryIntendedUseString = i18n("TreasureDungeon");
                break;
            case 34:
                territoryIntendedUseString = i18n("SeasonalInstancedArea");
                break;
            case 35:
                territoryIntendedUseString = i18n("TripleTriadBattleHall");
                break;
            case 36:
                territoryIntendedUseString = i18n("ChaoticRaid");
                break;
            case 37:
                territoryIntendedUseString = i18n("CrystallineConflictCustomMatch");
                break;
            case 39:
                territoryIntendedUseString = i18n("RivalWings");
                break;
            case 40:
                territoryIntendedUseString = i18n("PrivateEventArea");
                break;
            case 41:
                territoryIntendedUseString = i18n("Eureka");
                break;
            case 42:
                territoryIntendedUseString = i18n("Unk2");
                break;
            case 43:
                territoryIntendedUseString = i18n("Unk3");
                break;
            case 44:
                territoryIntendedUseString = i18n("Leap of Faith");
                break;
            case 45:
                territoryIntendedUseString = i18n("MaskedCarnival");
                break;
            case 46:
                territoryIntendedUseString = i18n("OceanFishing");
                break;
            case 47:
                territoryIntendedUseString = i18n("Unk7");
                break;
            case 48:
                territoryIntendedUseString = i18n("Unk8");
                break;
            case 49:
                territoryIntendedUseString = i18n("IslandSanctuary");
                break;
            case 50:
                territoryIntendedUseString = i18n("Unk10");
                break;
            case 51:
                territoryIntendedUseString = i18n("TripleTriadInvitationalParlor");
                break;
            case 52:
                territoryIntendedUseString = i18n("Unk12");
                break;
            case 53:
                territoryIntendedUseString = i18n("Unk13");
                break;
            case 54:
                territoryIntendedUseString = i18n("Unk14");
                break;
            case 55:
                territoryIntendedUseString = i18n("Unk15");
                break;
            case 56:
                territoryIntendedUseString = i18n("Elysion");
                break;
            case 57:
                territoryIntendedUseString = i18n("CriterionDungeon");
                break;
            case 58:
                territoryIntendedUseString = i18n("SavageCriterionDungeon");
                break;
            case 59:
                territoryIntendedUseString = i18n("Blunderville");
                break;
            case 60:
                territoryIntendedUseString = i18n("CosmicExploration");
                break;
            case 61:
                territoryIntendedUseString = i18n("OccultCrescent");
                break;
            case 62:
                territoryIntendedUseString = i18n("Unk22");
                break;
            default:
                break;
            }

            QStandardItem *item = new QStandardItem();
            item->setData(QString::fromStdString(bg));
            item->setData(contentFinderCondition, Qt::UserRole + 2);
            item->setText(QStringLiteral("%1 %2 %3 (%4, %5, %6)")
                              .arg(QString::number(i))
                              .arg(territoryIntendedUseString,
                                   QString::fromStdString(bg),
                                   QString::fromStdString(placeRegion),
                                   QString::fromStdString(placeZone),
                                   QString::fromStdString(placeName)));

            originalModel->insertRow(originalModel->rowCount(), item);
        }
    }

    listWidget = new QListView();
    listWidget->setEditTriggers(QListView::EditTrigger::NoEditTriggers);
    listWidget->setModel(m_searchModel);

    connect(listWidget, &QListView::activated, this, &MapListWidget::accept);

    layout->addWidget(listWidget);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &MapListWidget::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &MapListWidget::reject);
    layout->addWidget(buttonBox);

    // Disable when there's no selection
    connect(listWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this, buttonBox] {
        buttonBox->button(QDialogButtonBox::Open)->setEnabled(listWidget->selectionModel()->hasSelection());
    });

    // And it should be disabled by default
    buttonBox->button(QDialogButtonBox::Open)->setEnabled(false);
}

QString MapListWidget::acceptedMap() const
{
    return m_acceptedMap;
}

int MapListWidget::acceptedContentFinderCondition() const
{
    return m_acceptedContentFinderCondition;
}

void MapListWidget::accept()
{
    // Figure out the selection first
    const auto index = listWidget->selectionModel()->selectedIndexes().constFirst();
    if (index.isValid()) {
        m_acceptedMap = m_searchModel->mapToSource(index).data(Qt::UserRole + 1).toString();
        m_acceptedContentFinderCondition = m_searchModel->mapToSource(index).data(Qt::UserRole + 2).toInt();
    }

    QDialog::accept();
}

#include "moc_maplistwidget.cpp"
