#include "common.h"
#include "version_test.h"

int main(int argc, char** argv) {
    BW_UNUSED(argc);
    BW_UNUSED(argv);

    int res = version_test();
    BW_UNUSED(res);

    return 0;
}
