#include "backwalk/version.h"

#include <stddef.h>

int version_test(void) {
    const char* version = bw_version();
    if (version == NULL || *version == '\0') {
        return 1;
    }

    return 0;
}
