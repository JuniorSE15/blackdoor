#include <unity.h>
#include "config.h"

void testSetupConfig() {
    setupConfig();
}

void setUp() {}

void tearDown() {}

int main( int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(testSetupConfig);

    UNITY_END();
}