#include <fmt/format.h>

#include "gamedata.h"

int main(int argc, char* argv[]) {
    if(argc < 3) {
        fmt::print("novus [sqpack directory] [game path] [out path]");
        return -1;
    }

    GameData data(argv[1]);

    data.extractFile(argv[2], argv[3]);

    return 0;
}