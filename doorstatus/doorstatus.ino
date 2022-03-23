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
 *          https://github.com/waveshare/e-Paper/tree/master/Arduino/epd2in9
 *          https://www.gammon.com.au/forum/?id=11497
 */

/*
 * E-paper pins - pro mini pins
 * BUSY - D4
 * RST  - D5
 * DC   - D6
 * CS   - D10
 * DIN  - D11
 * CLK  - D13
 * VCC  - 3.3V
 * GND  - GND
 * 
 * NRF24L pins - pro mini pins
 * CE   - D9
 * CSN  - D10
 * SCK  - D13
 * M0   - D11
 * M1   - D12
 * IRO  - NONE
 * VCC  - 3.3V
 * GND  - GND
 */

//#include "LowPower.h"
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "epd2in9_V2.h"
#include "epdpaint.h"
#include "imagedata.h"

#include <RF24_config.h>
#include <RF24.h>

#define DEBUG 1
#define RECEIVE_TIMEOUT_MS 1000

#define COLORED 0
#define UNCOLORED 1

unsigned char image[1024];
Paint paint(image, 0, 0);
Epd epd;
#define EPD_CONTROL_PIN 8

RF24 radio(9, 10);
bool read_error = false;

unsigned short state;
unsigned short current_state;
#define STATE_UNKNOWN 0
#define STATE_INITIALIZING 1
#define STATE_ENTRY_PERMITTED 2
#define STATE_IN_MEETING 3
#define STATE_READ_ERROR 4

// watchdog interrupt
ISR (WDT_vect) 
{
   wdt_disable();  // disable watchdog
}  // end of WDT_vect

void protected_serial_ln(const char* str) {
  if (DEBUG) {
    Serial.println(str);
  }
}

void sleep_1_8_sec(bool sec_1) {
  byte oldTIMSK0 = TIMSK0;  // save Timer 0 Interrupt mask register
  TIMSK0 = 0;  //  stop Timer 0 interrupts
  
  // disable ADC
  ADCSRA = 0;  

  // clear various "reset" flags
  MCUSR = 0;     
  // allow changes, disable reset
  WDTCSR = bit (WDCE) | bit (WDE);
  // set interrupt mode and an interval 
  if (sec_1) {
    WDTCSR = bit (WDIE) | bit (WDP2) | bit (WDP1);    // set WDIE, and 1 seconds delay
  } else {
    WDTCSR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
  }
  wdt_reset();  // pat the dog
  
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  noInterrupts ();           // timed sequence follows
  sleep_enable();
 
  // turn off brown-out enable in software
  MCUCR = bit (BODS) | bit (BODSE);
  MCUCR = bit (BODS); 
  interrupts ();             // guarantees next instruction executed
  sleep_cpu ();  
  
  // cancel sleep as a precaution
  sleep_disable();
  TIMSK0 = oldTIMSK0;  // This will probably cause pending interrupts to happen immediately.
}

void sleep_seconds() {
  protected_serial_ln("Start sleep");
  radio.powerDown();
  epd.Sleep();
  digitalWrite(EPD_CONTROL_PIN, LOW);
  // This is stupid for some reason and they didn't work in a loop, so this just needs to be manually programmed
  // Current setting: ~64s
  sleep_1_8_sec(false);
  sleep_1_8_sec(false);
  sleep_1_8_sec(false);
  sleep_1_8_sec(false);
  sleep_1_8_sec(false);
  sleep_1_8_sec(false);
  sleep_1_8_sec(false);
  sleep_1_8_sec(false);
//  delay(8000);
  digitalWrite(EPD_CONTROL_PIN, HIGH);
  radio.powerUp();
  delay(5); // Wait up to 5ms for power up https://nrf24.github.io/RF24/classRF24.html#aa0a51923a09ba4f3478aba9be0f8a6a1
  protected_serial_ln("End sleep");
}


void send_data(unsigned long data) {
  radio.stopListening(); // Set as transmitter
  radio.write(&data, sizeof(data));
  protected_serial_ln("Sent " + data);
}

unsigned long receive() {
  radio.startListening(); // Set as receiver
  if (radio.available()) {
    unsigned long data;
    radio.read(&data, sizeof(data));
    protected_serial_ln("Received " + data);
    read_error = false;
    return data;
  } else {
    read_error = true;
  }
}

void paint_status() {
  /*
   * Can have up to 17 characters in the text (at 24pt font)
   */

  Serial.println(state);
  Serial.println(current_state);
  
  const char* text = "";
  bool light = true;
  if (state == STATE_UNKNOWN || current_state == state) {
    return;
  } else {
    if (state == STATE_INITIALIZING) { // Initialization
      text = "Initializing";
      light = true;
    } else if (state == STATE_ENTRY_PERMITTED) { // Entry Permitted
      text = "Entry Permitted";
      light = true;
    } else if (state == STATE_IN_MEETING) { // In Meeting
      text = "In Meeting";
      light = false;
    } else if (state = STATE_READ_ERROR) { // Read Error
      text = "Read Error";
      light = false;
    } else { // Error case
      text = "Unknown Status";
      light = false;
    }
  }

  
  if (epd.Init() != 0) {
    protected_serial_ln("e-Paper init failed\n");
    return;
  }

  int background;
  int content;
  if (light) {
    background = UNCOLORED;
    content = COLORED;
  } else {
    background = COLORED;
    content = UNCOLORED;
  }
  
  epd.ClearFrameMemory(0xFF);
  epd.DisplayFrame();
  //epd.ClearFrameMemory(0xFF);
  //epd.DisplayFrame();

  paint.SetRotate(ROTATE_270); // Y should be the old X value, and X should be HEIGHT - old Y value
  paint.SetWidth(24);
  paint.SetHeight(296);

  paint.Clear(background);
  for (unsigned short i = 0; i < 10; i++) {
      int center_x = i*(paint.GetHeight()/10)+(paint.GetHeight()/20);
      int center_y = 12;
    if (light) {
      paint.DrawCircle(center_x, center_y, 4, content);
    } else {
      paint.DrawLine(center_x - 4, center_y - 4, center_x + 5, center_y + 5, content);
      paint.DrawLine(center_x - 4, center_y + 4, center_x + 5, center_y - 5, content);
    }
  }
  epd.SetFrameMemory(paint.GetImage(), 128-paint.GetWidth()-0, 296-paint.GetHeight()-0, paint.GetWidth(), paint.GetHeight());
  epd.SetFrameMemory(paint.GetImage(), 0, 296-paint.GetHeight()-0, paint.GetWidth(), paint.GetHeight());

  paint.Clear(background);
  paint.DrawStringAt((paint.GetHeight()-strlen(text)*17)/2, 0, text, &Font24, content); // 24pt font is 17px wide
  epd.SetFrameMemory(paint.GetImage(), (128/2)-(24/2), 0, paint.GetWidth(), paint.GetHeight());
  
  epd.DisplayFrame();
  current_state = state;
}

void setup() {
  if (DEBUG) {
    Serial.begin(9600);
  }

  pinMode(EPD_CONTROL_PIN, OUTPUT);
  digitalWrite(EPD_CONTROL_PIN, HIGH);
  
  if (epd.Init() != 0) {
    protected_serial_ln("e-Paper init failed\n");
    return;
  }

  protected_serial_ln("Starting init\n");

  radio.begin();
  radio.setPALevel(RF24_PA_MIN);
  radio.openWritingPipe(0xF0F0F0F0E1LL);
  radio.openReadingPipe(1, 0xF0F0F0F0E1LL);
  radio.stopListening(); // Set as transmitter
  //radio.startListening(); // Set as receiver

  state = STATE_INITIALIZING;
  paint_status();

  //if (epd.Init() != 0) {
  //  Serial.print("e-Paper init failed\n");
  //  return;
  //}

  //epd.SetFrameMemory_Base(IMAGE_DATA);
  //epd.DisplayFrame();
  //epd.SetFrameMemory(IMAGE_DATA);
  //epd.DisplayFrame();

  protected_serial_ln("Setup completed\n");
}

void loop() {
  // put your main code here, to run repeatedly:

  // TODO: Gather sensor readings

  // Transmit data
  send_data(0); // Request update

  // Wait for response
  unsigned long data;
  unsigned short retry_count = 100;
  do {
    data = receive();
    if (read_error) {
      delay(10);
    }
    if (retry_count-- <= 0) {
      break;
    }
  } while (read_error);

  // Take action based on response
//    paint.SetWidth(32);
//    paint.SetHeight(96);
//    paint.SetRotate(ROTATE_270);
//    paint.Clear(UNCOLORED);
  
  if (read_error) {
    protected_serial_ln("Read timeout/read_error still true");
    state = STATE_READ_ERROR;
    paint_status();
//    paint.SetWidth(32);
//    paint.SetHeight(96);
//    paint.SetRotate(ROTATE_270);
//    paint.Clear(UNCOLORED);
//    paint.DrawStringAt(0, 4, "rerror", &Font24, COLORED);
//    epd.SetFrameMemory_Partial(paint.GetImage(), 80, 72, paint.GetWidth(), paint.GetHeight());
//    epd.DisplayFrame_Partial();
  } else if (data == 1) {
    protected_serial_ln("Interpreted response as 1");
    state = STATE_ENTRY_PERMITTED;
    paint_status();
//      paint.DrawStringAt(0, 4, "Got 1", &Font24, COLORED);
  } else if (data == 2) {
    protected_serial_ln("Interpreted response as 2");
    state = STATE_IN_MEETING;
    paint_status();
//      paint.DrawStringAt(0, 4, "Got 2", &Font24, COLORED);
  } else {
    protected_serial_ln("Interpreted response as something other than 1 or 2");
    state = STATE_UNKNOWN;
    paint_status();
//      paint.DrawStringAt(0, 4, "Got other", &Font24, COLORED);
  }

//    epd.SetFrameMemory_Partial(paint.GetImage(), 80, 72, paint.GetWidth(), paint.GetHeight());
//    epd.DisplayFrame_Partial();
  
  // Sleep
  sleep_seconds();
  //delay(500);

}
