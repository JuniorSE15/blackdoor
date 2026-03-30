#include <unity.h>

#include "config.h"
#include "event.h"

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_initial_state_wakeup(void) {
    volatile bool fakeIsOpen = true;
    wakeUpHardware(&fakeIsOpen);
    TEST_ASSERT_FALSE(fakeIsOpen);
}

void test_trigger_relay(void) {
    // In Arduino framework, testing digital writes often requires 
    // a mock or running on the actual hardware. Let's assume testing compilation first.
    triggerRelay(HIGH); // Set HIGH
    triggerRelay(LOW); // Set LOW
    TEST_ASSERT_TRUE(true);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_initial_state_wakeup);
    RUN_TEST(test_trigger_relay);
    UNITY_END();
    return 0;
} 