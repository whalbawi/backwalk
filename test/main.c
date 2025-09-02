#include "common.h"
#include "version_test.h"

int main(void) {
    {
        int res = version_test();
        BW_UNUSED(res);
    }

    return 0;
}
