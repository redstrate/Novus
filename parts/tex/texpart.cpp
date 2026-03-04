// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texpart.h"

#include <KLocalizedString>
#include <QFileDialog>
#include <QVBoxLayout>
#include <physis.hpp>

TexPart::TexPart(physis_SqPackResource *data, QWidget *parent)
    : QWidget(parent)
    , data(data)
{
    auto layout = new QVBoxLayout();
    setLayout(layout);

    m_label = new ImageLabel();
    layout->addWidget(m_label);

    m_saveImage = new QAction(i18n("Save PNG…"));
    m_saveImage->setIcon(QIcon::fromTheme(QStringLiteral("document-save")));
    connect(m_saveImage, &QAction::triggered, this, [this] {
        const QString savePath = QFileDialog::getSaveFileName(this, i18nc("@title:window", "Save Texture"), QDir::homePath(), QStringLiteral("*.png"));
        if (!savePath.isEmpty()) {
            m_label->pixmap().save(savePath, "PNG");
        }
    });
}

void TexPart::loadTex(physis_Buffer file)
{
    auto tex = physis_texture_parse(data->platform, file);

    QImage image(tex.rgba, tex.width, tex.height, QImage::Format_RGBA8888);
    m_label->setQPixmap(QPixmap::fromImage(image));
}

void TexPart::loadHwc(physis_Buffer file)
{
    auto tex = physis_hwc_parse(data->platform, file);

    QImage image(tex.rgba, Hwc_WIDTH, Hwc_HEIGHT, QImage::Format_RGBA8888);
    m_label->setQPixmap(QPixmap::fromImage(image));
}

QAction *TexPart::saveImageAction()
{
    return m_saveImage;
}

#include "moc_texpart.cpp"
