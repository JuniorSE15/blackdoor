#include <unity.h>
#include "config.h"

void test_setupConfig() {
    setupConfig();
}

void setUp() {}

void tearDown() {}

int main( int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_setupConfig);

    UNITY_END();
}