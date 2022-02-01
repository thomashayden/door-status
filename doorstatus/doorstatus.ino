/*
 * General program setup:
 * 
 * Every N seconds do:
 *   - Gather sensor readings
 *   - Transmit data to hub and request response
 *   - Wait for response
 *   - Take action based on response
 *   - Sleep for M seconds
 * 
 * 
 * Potential additional features:
 *   - If no response, cache data and try again next loop (with new data as well)
 */

/*
 * Utilizes https://github.com/rocketscream/Low-Power
 */


#include "LowPower.h"

#define PIN_TX 0
#define PIN_RX 0

#define SLEEP_TIME_S 60



void sleep_seconds(unsigned short seconds) {
  for (unsigned short i = 0; i < seconds / 8; i++) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
  for (unsigned short i = 0; i < seconds % 8; i++) {
    LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
  }
}


void setup() {
  // put your setup code here, to run once:

  pinMode(PIN_TX, OUTPUT);
  pinMode(PIN_RX, INPUT);

}

void loop() {
  // put your main code here, to run repeatedly:

  bool running = true;

  while (running) {
    // TODO: Gather sensor readings

    // TODO: Transmit data

    // TODO: Wait for response

    // TODO: Take action based on response

    // Sleep
    sleep_seconds(SLEEP_TIME_S);
  }

}
