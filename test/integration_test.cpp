#include <unity.h>

#include "config.h"
#include "event.h"

void setUp() {}
void tearDown() {}

int main( int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(testSetupConfig);
    RUN_TEST(test_initial_state_wakeup);

    UNITY_END();
}

void testSetupConfig() {
    setupConfig();
}

void test_initial_state_wakeup(void) {
    volatile bool fakeIsOpen = true;
    wakeUpHardware(&fakeIsOpen);
    TEST_ASSERT_FALSE(fakeIsOpen);
}


