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
#include <RCSwitch.h>

#define PIN_TX 2
#define PIN_RX 1

#define SLEEP_TIME_S 5
#define RECEIVE_TIMEOUT_MS 1000

RCSwitch rxtx_switch = RCSwitch();

void sleep_seconds(unsigned short seconds) {
  Serial.print("Start sleep\n");
  for (unsigned short i = 0; i < seconds / 8; i++) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
  for (unsigned short i = 0; i < seconds % 8; i++) {
    LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
  }
  Serial.print("End sleep\n");
}

void send_data(int data) {
  rxtx_switch.send(data, 24);
  Serial.print("Data sent\n");
}

unsigned long receive() {
  Serial.print("Beginning receive\n");
  unsigned long start_time = millis();
  while (millis() < start_time + RECEIVE_TIMEOUT_MS && millis() >= start_time) {
    //if (rxtx_switch.available()) {
    //  Serial.print("Avail");
    //  unsigned long data = rxtx_switch.getReceivedValue();
    //  unsigned int length = rxtx_switch.getReceivedBitlength();
    //  rxtx_switch.resetAvailable();
    //  Serial.print("Data received ... ");
    //  Serial.print(data);
    //  return data;
    //  // TODO: Add error checking for both bad messages and ones that have already been seen
    //}
  }
  Serial.print("Read timeout\n");
  return 0;
}

void setup() {
  // put your setup code here, to run once:

  pinMode(PIN_TX, OUTPUT);
  //pinMode(PIN_RX, INPUT);

  rxtx_switch.enableTransmit(PIN_TX);
  //rxtx_switch.enableReceive(PIN_RX);
  rxtx_switch.setProtocol(1);

  Serial.begin(9600);
  Serial.print("Initialized\n");
}

void loop() {
  // put your main code here, to run repeatedly:

  bool running = true;

  while (running) {
    // TODO: Gather sensor readings

    // TODO: Transmit data
    send_data(0);

    // TODO: Wait for response
    unsigned long data = receive();

    // TODO: Take action based on response

    // Sleep
    sleep_seconds(SLEEP_TIME_S);
  }

}
