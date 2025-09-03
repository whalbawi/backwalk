#include "backwalk/version.h"

#include "test.h"  // for TEST, TEST_ASSERT_NE_CHAR, TEST_ASSERT_NONNULL

TEST(populated, {
    const char* version = bw_version();
    TEST_ASSERT_NONNULL(version);
    TEST_ASSERT_NE_CHAR(version[0], '\0');
})

int main(void) {
    TEST_INIT("version");

    TEST_RUN(populated);

    TEST_EXIT();
}
