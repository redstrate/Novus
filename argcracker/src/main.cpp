#include <fmt/core.h>
#include <QString>

#include "blowfish.h"

// from xivdev
static char ChecksumTable[] = {
        'f', 'X', '1', 'p', 'G', 't', 'd', 'S',
        '5', 'C', 'A', 'P', '4', '_', 'V', 'L'
};

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

    Blowfish session(QByteArray(buffer, 8));
    QByteArray encryptedArg = session.Encrypt((QString(" /T =%1").arg(ticks) + arg).toUtf8());
    QString base64 = encryptedArg.toBase64(QByteArray::Base64Option::Base64UrlEncoding | QByteArray::Base64Option::OmitTrailingEquals);
    char checksum = GetChecksum(key);

    return QString("//**sqex0003%1%2**//").arg(base64, QString(checksum));
}

inline QString decryptGameArg(uint32_t tickCount, QString sqexString) {
    unsigned int ticks = tickCount & 0xFFFFFFFFu;
    unsigned int key = ticks & 0xFFFF0000u;

    char buffer[9] = {};
    sprintf(buffer, "%08x", key);

    Blowfish session(QByteArray(buffer, 8));

    QStringRef base64String(&sqexString, 12, sqexString.length() - 5 - 12);
    QByteArray base64Decoded = QByteArray::fromBase64(base64String.toUtf8(), QByteArray::Base64Option::Base64UrlEncoding | QByteArray::Base64Option::OmitTrailingEquals);

    auto decrypted = session.Decrypt(base64Decoded).trimmed();

    return decrypted;
}

int main(int argc, char* argv[]) {
    const char* toCrack = argv[1];
    int tickRange = atoi(argv[2]);
    const char* knownArg = argv[3];

    uint32_t bottom = TickCount() - tickRange;

    fmt::print("Beginning to crack {}...\n", toCrack);

    for(uint32_t i = bottom; i < TickCount(); i++) {
        QString decrypted = decryptGameArg(i, toCrack);

        if(decrypted.contains(knownArg)) {
            fmt::print("Decrypted successfully: {}\n", decrypted.toStdString());
            return 0;
        }
    }

    return -1;
}