// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QDebug>
#include <QString>
#include <QStringRef>

#include <physis.hpp>

// from xivdev
static char ChecksumTable[] = {'f', 'X', '1', 'p', 'G', 't', 'd', 'S', '5', 'C', 'A', 'P', '4', '_', 'V', 'L'};

inline char GetChecksum(unsigned int key) {
    auto value = key & 0x000F0000;
    return ChecksumTable[value >> 16];
}

uint32_t TickCount();

inline QString encryptGameArg(QString arg) {
    unsigned int rawTicks = TickCount();
    unsigned int ticks = rawTicks & 0xFFFFFFFFu;
    unsigned int key = ticks & 0xFFFF0000u;

    char buffer[9] = {};
    sprintf(buffer, "%08x", key);

    Blowfish* session = physis_blowfish_initialize(reinterpret_cast<uint8_t *>(buffer), 9);

    uint8_t* out_data = nullptr;
    uint32_t out_size = 0;

    QByteArray toEncrypt = (QStringLiteral(" /T =%1").arg(ticks) + arg).toUtf8();

    physis_blowfish_encrypt(session,
                            reinterpret_cast<uint8_t*>(toEncrypt.data()), toEncrypt.size(), &out_data, &out_size);

    QByteArray encryptedArg = QByteArray::fromRawData(
        reinterpret_cast<const char*>(out_data), out_size);

    QString base64 = QString::fromUtf8(encryptedArg.toBase64(QByteArray::Base64Option::Base64UrlEncoding | QByteArray::Base64Option::OmitTrailingEquals));
    char checksum = GetChecksum(key);

    return QStringLiteral("//**sqex0003%1%2**//").arg(base64, QLatin1String(&checksum, 1));
}

inline QString decryptGameArg(uint32_t tickCount, QString sqexString) {
    unsigned int ticks = tickCount & 0xFFFFFFFFu;
    unsigned int key = ticks & 0xFFFF0000u;

    char buffer[9] = {};
    sprintf(buffer, "%08x", key);

    Blowfish* session = physis_blowfish_initialize(reinterpret_cast<uint8_t *>(buffer), 9);

    QStringRef base64String(&sqexString, 12, sqexString.length() - 5 - 12);
    QByteArray base64Decoded = QByteArray::fromBase64(base64String.toUtf8(), QByteArray::Base64Option::Base64UrlEncoding | QByteArray::Base64Option::OmitTrailingEquals);

    uint8_t* out_data = nullptr;
    uint32_t out_size = 0;

    physis_blowfish_decrypt(session,
                            reinterpret_cast<uint8_t*>(base64Decoded.data()), base64Decoded.size(), &out_data, &out_size);

    QByteArray decrypted = QByteArray::fromRawData(
        reinterpret_cast<const char*>(out_data), out_size).trimmed();

    return QString::fromUtf8(decrypted);
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        qInfo() << "Usage: novus-argcracker [sqexarg string] [tick range] [known arg]";
        return 1;
    }

    const char* toCrack = argv[1];
    int tickRange = atoi(argv[2]);
    const char* knownArg = argv[3];

    uint32_t bottom = TickCount() - tickRange;

    qInfo() << "Beginning to crack" << toCrack << "...";

    for(uint32_t i = bottom; i < TickCount(); i++) {
        const QString decrypted = decryptGameArg(i, QLatin1String(toCrack));

        if (decrypted.contains(QLatin1String(knownArg))) {
            qInfo() << "Decrypted successfully:" << decrypted;
            return 0;
        }
    }

    return -1;
}