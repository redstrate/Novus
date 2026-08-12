// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scdpart.h"

#include <KLocalizedString>
#include <QAudioOutput>
#include <QBuffer>
#include <QMediaPlayer>
#include <QPushButton>
#include <QVBoxLayout>
#include <physis.hpp>

ScdPart::ScdPart(QWidget *parent)
    : QWidget(parent)
{
    const auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    m_playButton = new QPushButton(i18n("Play"));
    m_playButton->setEnabled(false);
    layout->addWidget(m_playButton);

    m_mediaPlayer = new QMediaPlayer(this);
    connect(m_playButton, &QPushButton::pressed, m_mediaPlayer, &QMediaPlayer::play);

    m_audioOutput = new QAudioOutput(this);
    m_mediaPlayer->setAudioOutput(m_audioOutput);
}

ScdPart::~ScdPart()
{
    m_mediaPlayer->stop();
}

void ScdPart::load(const Platform platform, const physis_Buffer file)
{
    const auto scd = physis_scd_parse(platform, file);
    if (scd.audio_count > 0) {
        m_audioData = QByteArray::fromRawData(reinterpret_cast<const char *>(scd.audios[0].data), scd.audios[0].data_size);
        m_mediaPlayer->setSourceDevice(new QBuffer(&m_audioData));
        m_playButton->setEnabled(true);
    }
}

#include "moc_scdpart.cpp"
