// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "enemyinfowindow.h"

#include "pathedit.h"

#include <KLocalizedString>
#include <QFormLayout>
#include <QLineEdit>

EnemyInfoWindow::EnemyInfoWindow(uint32_t id, const QString &mdlPath, const QString &mtrlPath, QWidget *parent)
{
    auto layout = new QFormLayout();
    setLayout(layout);

    auto idEdit = new QLineEdit();
    idEdit->setText(QString::number(id));
    idEdit->setReadOnly(true);

    layout->addRow(i18n("ID"), idEdit);

    auto mdlPathEdit = new PathEdit();
    mdlPathEdit->setPath(mdlPath);
    mdlPathEdit->setReadOnly(true);

    layout->addRow(i18n("MDL"), mdlPathEdit);

    auto mtrlPathEdit = new PathEdit();
    mtrlPathEdit->setPath(mtrlPath);
    mtrlPathEdit->setReadOnly(true);

    layout->addRow(i18n("MTRL"), mtrlPathEdit);
}

#include "moc_enemyinfowindow.cpp"
