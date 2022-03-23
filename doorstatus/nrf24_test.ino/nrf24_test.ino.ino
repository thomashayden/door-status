#include <SPI.h>
#include <RF24_config.h>
#include <RF24.h>
#include <printf.h>
//#include <nRF24L01.h>

RF24 radio(9, 10);

//const byte address[6] = [;


void setup() {
  radio.begin();
  radio.setPALevel(RF24_PA_MIN);
  //radio.setChannel(0x76);
  radio.openWritingPipe(0xF0F0F0F0E1LL);
  radio.openReadingPipe(1, 0xF0F0F0F0E1LL);
  //radio.enableDynamicPayloads();
  //radio.powerUp();
  //radio.stopListening(); // Set as transmitter
  radio.startListening(); // Set as receiver
  
  Serial.begin(9600);
  
  pinMode(7, OUTPUT);
  pinMode(6, INPUT);
  digitalWrite(7, HIGH);
}

void loop() {
  radio.startListening();
  if (radio.available()) {
    unsigned long data;
    radio.read(&data, sizeof(data));
    Serial.println("Message received " + data);

    radio.stopListening();
    
    if (data == 0) {
      unsigned long return_data = 1;
      if (digitalRead(6) == HIGH) {
        return_data = 2;
      }
      radio.write(&return_data, sizeof(return_data));
      Serial.println("Returned value " + return_data);
    } else {
      Serial.println("Received unknown request");
    }
  }
}


/*

// 18 Mar 2018 - simple program to verify connection between Arduino
//      and nRF24L01+
//  This program does NOT attempt any communication with another nRF24

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <printf.h>

#define CE_PIN   9
#define CSN_PIN 10

const byte thisSlaveAddress[5] = {'R','x','A','A','A'};

RF24 radio(CE_PIN, CSN_PIN);

char dataReceived[10]; // this must match dataToSend in the TX
bool newData = false;


void setup() {
    Serial.begin(9600);
    printf_begin();

    Serial.println("CheckConnection Starting");
    Serial.println();
    Serial.println("FIRST WITH THE DEFAULT ADDRESSES after power on");
    Serial.println("  Note that RF24 does NOT reset when Arduino resets - only when power is removed");
    Serial.println("  If the numbers are mostly 0x00 or 0xff it means that the Arduino is not");
    Serial.println("     communicating with the nRF24");
    Serial.println();
    radio.begin();
    radio.printDetails();
    Serial.println();
    Serial.println();
    Serial.println("AND NOW WITH ADDRESS AAAxR  0x41 41 41 78 52   ON P1");
    Serial.println(" and 250KBPS data rate");
    Serial.println();
    radio.openReadingPipe(1, thisSlaveAddress);
    radio.setDataRate( RF24_250KBPS );
    radio.printDetails();
    Serial.println();
    Serial.println();
}


void loop() {

}
*/
