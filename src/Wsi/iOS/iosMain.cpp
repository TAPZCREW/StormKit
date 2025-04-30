#include "iosMain.hpp"

#include <cstdlib>

[[gnu::weak]]
int iosMain(int argc, char** argv) {
    return iosMain();
}

[[gnu::weak]]
int iosMain() {
    return EXIT_SUCCESS;
}
