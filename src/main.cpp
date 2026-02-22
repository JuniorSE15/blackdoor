#include <Arduino.h>
#include "config.h"
#include <stdio.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  setupConfig();
}

void loop() {
  // put your main code here, to run repeatedly:
}