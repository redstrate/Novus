// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hashdatabase.h"
#include "settings.h"

const std::array known_folders{"common",
                               "common/font",
                               "common/graphics",
                               "common/graphics/texture",
                               "common/softwarecursor",
                               "exd",
                               "chara/equipment/e0000/material/v0001",
                               "chara/equipment/e0000/texture",
                               "chara/human/c0101/obj/face/f0001/model",
                               "shader/sm5/shpk",
                               "chara/xls/bonedeformer",
                               "chara/xls/charamake",
                               "chara/human/c0101/skeleton/base/b0001"};

const std::array common_font{"common/VulgarWordsFilter.dic",
                             "common/VulgarWordsFilter_party.dic",
                             "common/font/fontIcon_Xinput.tex",
                             "common/font/gfdata.gfd",
                             "common/font/fontIcon_Ps3.tex",
                             "common/font/gfdata.gfd",
                             "common/font/fontIcon_Ps4.tex",
                             "common/font/gfdata.gfd",
                             "common/font/fontIcon_Ps5.tex",
                             "common/font/gfdata.gfd",
                             "common/font/MiedingerMid_36.fdt",
                             "common/font/MiedingerMid_18.fdt",
                             "common/font/MiedingerMid_14.fdt",
                             "common/font/MiedingerMid_12.fdt",
                             "common/font/MiedingerMid_10.fdt",
                             "common/font/Meidinger_40.fdt",
                             "common/font/Meidinger_20.fdt",
                             "common/font/Meidinger_16.fdt",
                             "common/font/=TrumpGothic_68.fdt",
                             "common/font/TrumpGothic_34.fdt",
                             "common/font/TrumpGothic_23.fdt",
                             "common/font/TrumpGothic_184.fdt",
                             "common/font/Jupiter_46.fdt",
                             "common/font/Jupiter_23.fdt",
                             "common/font/Jupiter_20.fdt",
                             "common/font/Jupiter_16.fdt",
                             "common/font/Jupiter_90.fdt",
                             "common/font/Jupiter_45.fdt",
                             "common/font/MiedingerMid_36.fdt",
                             "common/font/MiedingerMid_18.fdt",
                             "common/font/MiedingerMid_14.fdt",
                             "common/font/MiedingerMid_12.fdt",
                             "common/font/MiedingerMid_10.fdt",
                             "common/font/Meidinger_40.fdt",
                             "common/font/Meidinger_20.fdt",
                             "common/font/Meidinger_16.fdt",
                             "common/font/TrumpGothic_68.fdt",
                             "common/font/TrumpGothic_34.fdt",
                             "common/font/TrumpGothic_23.fdt",
                             "common/font/TrumpGothic_184.fdt",
                             "common/font/Jupiter_46.fdt",
                             "common/font/Jupiter_23.fdt",
                             "common/font/Jupiter_20.fdt",
                             "common/font/Jupiter_16.fdt",
                             "common/font/Jupiter_90.fdt",
                             "common/font/Jupiter_45.fdt",
                             "common/font/AXIS_18_lobby.fdt",
                             "common/font/AXIS_14_lobby.fdt",
                             "common/font/AXIS_12_lobby.fdt",
                             "common/font/AXIS_12_lobby.fdt",
                             "common/font/>MiedingerMid_18_lobby.fdt",
                             "common/font/L=MiedingerMid_18_lobby.fdt",
                             "common/font/>MiedingerMid_14_lobby.fdt",
                             "common/font/MiedingerMid_12_lobby.fdt",
                             "common/font/MiedingerMid_10_lobby.fdt",
                             "common/font/Meidinger_20_lobby.fdt",
                             "common/font/Meidinger_20_lobby.fdt",
                             "common/font/Meidinger_16_lobby.fdt",
                             "common/font/TrumpGothic_34_lobby.fdt",
                             "common/font/TrumpGothic_34_lobby.fdt",
                             "common/font/TrumpGothic_23_lobby.fdt",
                             "common/font/TrumpGothic_184_lobby.fdt",
                             "common/font/Jupiter_23_lobby.fdt",
                             "common/font/Jupiter_23_lobby.fdt",
                             "common/font/Jupiter_20_lobby.fdt",
                             "common/font/Jupiter_16_lobby.fdt",
                             "common/font/Jupiter_45_lobby.fdt",
                             "common/font/Jupiter_45_lobby.fdt",
                             "common/font/AXIS_18_lobby.fdt",
                             "common/font/MiedingerMid_18_lobby.fdt",
                             "common/font/MiedingerMid_18_lobby.fdt",
                             "common/font/MiedingerMid_14_lobby.fdt",
                             "common/font/MiedingerMid_12_lobby.fdt",
                             "common/font/MiedingerMid_10_lobby.fdt",
                             "common/font/Meidinger_20_lobby.fdt",
                             "common/font/Meidinger_20_lobby.fdt",
                             "common/font/Meidinger_16_lobby.fdt",
                             "common/font/TrumpGothic_34_lobby.fdt",
                             "common/font/TrumpGothic_34_lobby.fdt",
                             "common/font/TrumpGothic_23_lobby.fdt",
                             "common/font/TrumpGothic_184_lobby.fdt",
                             "common/font/Jupiter_23_lobby.fdt",
                             "common/font/Jupiter_23_lobby.fdt",
                             "common/font/Jupiter_20_lobby.fdt",
                             "common/font/Jupiter_16_lobby.fdt",
                             "common/font/Jupiter_45_lobby.fdt",
                             "common/font/Jupiter_45_lobby.fdt",
                             "common/font/AXIS_18_lobby.fdt",
                             "common/font/AXIS_14_lobby.fdt",
                             "common/font/AXIS_12_lobby.fdt",
                             "common/font/AXIS_12_lobby.fdt",
                             "common/font/MiedingerMid_36_lobby.fdt",
                             "common/font/MiedingerMid_18_lobby.fdt",
                             "common/font/MiedingerMid_14_lobby.fdt",
                             "common/font/MiedingerMid_12_lobby.fdt",
                             "common/font/MiedingerMid_10_lobby.fdt",
                             "common/font/Meidinger_40_lobby.fdt",
                             "common/font/Meidinger_20_lobby.fdt",
                             "common/font/Meidinger_16_lobby.fdt",
                             "common/font/TrumpGothic_68_lobby.fdt",
                             "common/font/TrumpGothic_34_lobby.fdt",
                             "common/font/TrumpGothic_23_lobby.fdt",
                             "common/font/TrumpGothic_184_lobby.fdt",
                             "common/font/Jupiter_46_lobby.fdt",
                             "common/font/Jupiter_23_lobby.fdt",
                             "common/font/Jupiter_20_lobby.fdt",
                             "common/font/Jupiter_16_lobby.fdt",
                             "common/font/Jupiter_90_lobby.fdt",
                             "common/font/Jupiter_45_lobby.fdt",
                             "common/font/AXIS_36_lobby.fdt",
                             "common/font/MiedingerMid_36_lobby.fdt",
                             "common/font/MiedingerMid_18_lobby.fdt",
                             "common/font/MiedingerMid_14_lobby.fdt",
                             "common/font/MiedingerMid_12_lobby.fdt",
                             "common/font/MiedingerMid_10_lobby.fdt",
                             "common/font/Meidinger_40_lobby.fdt",
                             "common/font/Meidinger_20_lobby.fdt",
                             "common/font/Meidinger_16_lobby.fdt",
                             "common/font/TrumpGothic_68_lobby.fdt",
                             "common/font/TrumpGothic_34_lobby.fdt",
                             "common/font/TrumpGothic_23_lobby.fdt",
                             "common/font/TrumpGothic_184_lobby.fdt",
                             "common/font/Jupiter_46_lobby.fdt",
                             "common/font/Jupiter_23_lobby.fdt",
                             "common/font/Jupiter_20_lobby.fdt",
                             "common/font/Jupiter_16_lobby.fdt",
                             "common/font/Jupiter_90_lobby.fdt",
                             "common/font/Jupiter_45_lobby.fdt"};

int main(int argc, char *argv[])
{
    HashDatabase database;

    for (auto &folder : known_folders) {
        database.addFolder(QLatin1String(folder));
    }

    for (auto &file : common_font) {
        database.addFile(QLatin1String(file));
    }

    database.addFile(QStringLiteral("exd/root.exl"));
    database.addFile(QStringLiteral("exd/achievement.exh"));
    database.addFile(QStringLiteral("exd/Achievement_0_en.exd"));
    database.addFile(QStringLiteral("chara/equipment/e0000/material/v0001/mt_c0101e0000_sho_a.mtrl"));
    database.addFile(QStringLiteral("chara/equipment/e0000/texture/v01_c0101e0000_sho_d.tex"));
    database.addFile(QStringLiteral("chara/human/c0101/obj/face/f0001/model/c0101f0001_fac.mdl"));
    database.addFile(QStringLiteral("shader/sm5/shpk/character.shpk"));
    database.addFile(QStringLiteral("chara/xls/bonedeformer/human.pbd"));
    database.addFile(QStringLiteral("chara/xls/charamake/human.cmp"));
    database.addFile(QStringLiteral("chara/human/c0101/skeleton/base/b0001/skl_c0101b0001.sklb"));

    /*const QString gameDir{getGameDirectory()};
    const std::string gameDirStd{gameDir.toStdString()};
    auto data = physis_gamedata_initialize(gameDirStd.c_str());

    addPath(QStringLiteral("common/font/AXIS_12.fdt"));

    addPath(QStringLiteral("exd/root.exl"));

    auto sheetNames = physis_gamedata_get_all_sheet_names(data);

    for(int i = 0; i < sheetNames.name_count; i++) {
        auto sheetName = sheetNames.names[i];
        auto nameLowercase = QString::fromStdString(sheetName).toLower().toStdString();

        addPath(QStringLiteral("exd/") + QLatin1String(nameLowercase.c_str()) + QStringLiteral(".exh"));

        auto exh = physis_gamedata_read_excel_sheet_header(data, sheetName);
        for (int j = 0; j < exh->page_count; j++) {
            for (int z = 0; z < exh->language_count; z++) {
                std::string path = physis_gamedata_get_exd_filename(nameLowercase.c_str(), exh, exh->languages[z], j);

                addPath(QStringLiteral("exd/") + QString::fromStdString(path));
            }
        }
    }*/
}