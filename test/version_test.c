#include "backwalk/version.h"

#include <stddef.h>

#include "test.h"

TEST(populated) {
    const char* version = bw_version();
    TEST_ASSERT_NONNULL(version);
    TEST_ASSERT_NE_CHAR(version[0], '\0');

    TEST_OK();
}

int main(void) {
    TEST_INIT("version");

    TEST_RUN(populated);

    TEST_EXIT();
}
