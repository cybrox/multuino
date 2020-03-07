/**
 * Multuino - Custom Multi-Device Remote
 *
 * I hate harmonry remotes, they're clunky, complex and expensive.
 * This is my cheaper solution to solving my problem. In short, I
 * bought 5 XBOX remotes from china (since I don't own an XBOX, most
 * likely never will and like their design).
 *
 * This sketch is intended to run on a SparkFun Arduino Micro with
 * a SeeedStudio Grove IR receiver and Grove IR emitter. It will simply
 * receive signals from the XBOX remote and send out the signals that
 * the respective devices expect.
 *
 * This is not a plug and play project. It is optimized for my use case
 * but the code can be re-arranged and new protocol patterns can be added
 * to support pretty much any combination of remote and target devices.
 *
 * Please note: For running this on the SparkFun Arduino Micro, a fork
 * of the IRSendRev library is required, since the pin used by the original
 * library is not easily accessible on that board. Download the fork as
 * zip from the master branch and install it via the Arduino IDE.
 * Fork at: https://github.com/cybrox/IRSendRev
 */


#include <Keyboard.h>
#include <IRSendRev.h>

// When debug mode is enabled, no protocols will be relayed, instead protocol
// information will be output via Serial. This is an easy way to inspect the
// protocol of new devices and adding them to the lists below.
#define DEBUG_MODE      false


// IR Protocol Parts
#define BIT_LEN         0
#define BIT_START_H     1
#define BIT_START_L     2
#define BIT_DATA_H      3
#define BIT_DATA_L      4
#define BIT_DATA_LEN    5
#define BIT_DATA        6

// Input Remote Buttons
#define BTN_UNKNOWN     0
#define BTN_ONOFF       1
#define BTN_SOURCE      2
#define BTN_MENU        3
#define BTN_UP          4   
#define BTN_DOWN        5
#define BTN_LEFT        6
#define BTN_RIGHT       7
#define BTN_ENTER       8
#define BTN_MORE        9
#define BTN_BACK       10
#define BTN_VOLUP      11
#define BTN_VOLDOWN    12
#define BTN_CHANUP     13
#define BTN_CHANDOWN   14
#define BTN_MUTE       15
#define BTN_PLAYPAUSE  16
#define BTN_STOP       17
#define BTN_SEEKBW     18
#define BTN_SEEKFW     19
#define BTN_JMPBW      20
#define BTN_JMPFW      21

// Output Remote Actions
#define IRC_ONOFF       1
#define IRC_SOURCE      2
#define IRC_VOLUP       3
#define IRC_VOLDOWN     4
#define IRC_VOLMUTE     5
#define IRC_MENU        6
#define IRC_BACK        7

// Pin Definitions
#define PIN_IR_RX       5
#define PIN_IR_TX       6  // NOP

// IRSendRev Settings
#define IR_TX_FRQ       38 // 38kHz


// Target Device Protocols, these have been analyzed using the debug mode and
// an the original remote of the target device but you can find most of them
// online as well, in case you lost the remote or don't want to go get it.
const int samsungOnOff[]   = {91,  90, 10, 34, 0xE0, 0xE0, 0x40, 0xBF};
const int samsungVolUp[]   = {91,  90, 10, 34, 0xE0, 0xE0, 0xE0, 0x1F};
const int samsungVolDn[]   = {91,  90, 10, 34, 0xE0, 0xE0, 0xD0, 0x2F};
const int samsungVolMute[] = {91,  90, 10, 34, 0xE0, 0xE0, 0xF0, 0x0F};
const int samsungHome[]    = {91,  90, 10 ,34, 0xE0, 0xE0, 0x9E, 0x61};
const int samsungUp[]      = {91,  90, 10 ,34, 0xE0, 0xE0, 0x06, 0xF9};
const int samsungDown[]    = {91,  90, 10, 34, 0xE0, 0xE0, 0x86, 0x79};
const int samsungLeft[]    = {91,  90, 10 ,34, 0xE0, 0xE0, 0xA6, 0x59};
const int samsungRight[]   = {91,  90, 10 ,34, 0xE0, 0xE0, 0x46, 0xB9};
const int samsungOk[]      = {91,  90, 10, 34, 0xE0, 0xE0, 0x16, 0xE9};
const int samsungMenu[]    = {91,  90, 10, 34, 0xE0, 0xE0, 0x9E, 0x61};
const int samsungBack[]    = {91,  90, 10, 34, 0xE0, 0xE0, 0x1A, 0xE5};
const int onkyoOnOff[]     = {180, 88, 11, 33, 0x4B, 0x36, 0xD3, 0x2C};
const int onkyoVolUpR[]    = {180, 88, 11, 33, 0x4B, 0xB6, 0x40, 0xBF};
const int onkyoVolUpL[]    = {180, 88, 11, 33, 0x20, 0xDF, 0x40, 0xBF};
const int onkyoVolDnR[]    = {180, 88, 11, 33, 0x4B, 0xB6, 0xC0, 0x3F};
const int onkyoVolDnL[]    = {180, 88, 11, 33, 0x20, 0xDF, 0xC0, 0x3F};
const int hdmiOnOff[]      = {181, 90, 11, 33, 0x00, 0xFF, 0x00, 0xFF};
const int hdmiOne[]        = {181, 90, 11, 33, 0x00, 0xFF, 0x10, 0xEF};
const int hdmiTwo[]        = {181, 90, 11, 33, 0x00, 0xFF, 0x90, 0x6F};
const int hdmiThree[]      = {181, 90, 11, 33, 0x00, 0xFF, 0x50, 0xAF};
const int hdmiFour[]       = {181, 90, 11, 33, 0x00, 0xFF, 0x30, 0xCF};
const int hdmiFive[]       = {181, 90, 11, 33, 0x00, 0xFF, 0x70, 0x8F};
const int hdmiLeft[]       = {181, 90, 11, 33, 0x00, 0xFF, 0x28, 0xD7};
const int hdmiRight[]      = {181, 90, 11, 33, 0x00, 0xFF, 0x68, 0x97};

// Mode switch for switching between TV arrow control
// and computer arrow + enter + back control
bool modeSwitchOn = 0;

// Buffers
unsigned char rxBuffer[20];
unsigned char txBuffer[20];
int serialRxBuffer;

void setup() {
  IR.Init(PIN_IR_RX);

  Serial.begin(9600);
  Serial.println("Initialized multuino");
}

void loop() {
  int pressedButton;

  if(IR.IsDta()) {
    IR.Recv(rxBuffer);

    #if DEBUG_MODE == true
      debugIrBuffer();
    #else
      pressedButton = getPhysicalButton();
    #endif
  }

  if (pressedButton != BTN_UNKNOWN) {
    pressVirtualButton(pressedButton);
    pressedButton = BTN_UNKNOWN;
  }
}

// Physical button mapper
// In this case: XBOX ONE media remote
int getPhysicalButton () {
  if (rxBuffer[BIT_DATA_H] <  9 || rxBuffer[BIT_DATA_H] > 13) return BTN_UNKNOWN;
  if (rxBuffer[BIT_DATA_L] < 31 || rxBuffer[BIT_DATA_L] > 35) return BTN_UNKNOWN;
  if (rxBuffer[BIT_DATA_LEN] != 4) return BTN_UNKNOWN;

  if (bufferMatches(0x01, 0x1B, 0x26, 0xD9)) return BTN_ONOFF;
  if (bufferMatches(0x01, 0x1B, 0x76, 0x89)) return BTN_SOURCE;
  if (bufferMatches(0x01, 0x1B, 0xF6, 0x09)) return BTN_MENU;
  if (bufferMatches(0x01, 0x1B, 0x78, 0x87)) return BTN_UP;
  if (bufferMatches(0x01, 0x1B, 0xF8, 0x07)) return BTN_DOWN;
  if (bufferMatches(0x01, 0x1B, 0x04, 0xFB)) return BTN_LEFT;
  if (bufferMatches(0x01, 0x1B, 0x84, 0x7B)) return BTN_RIGHT;
  if (bufferMatches(0x01, 0x1B, 0x44, 0xBB)) return BTN_ENTER;
  if (bufferMatches(0x01, 0x1B, 0xC4, 0x3B)) return BTN_BACK;
  if (bufferMatches(0x01, 0x1B, 0x64, 0x9B)) return BTN_MORE;
  if (bufferMatches(0x01, 0x1B, 0x08, 0xF7)) return BTN_VOLUP;
  if (bufferMatches(0x01, 0x1B, 0x88, 0x77)) return BTN_VOLDOWN;
  if (bufferMatches(0x01, 0x1B, 0x48, 0xB7)) return BTN_CHANUP;
  if (bufferMatches(0x01, 0x1B, 0xC8, 0x37)) return BTN_CHANDOWN;
  if (bufferMatches(0x01, 0x1B, 0x0E, 0xF1)) return BTN_PLAYPAUSE;
  if (bufferMatches(0x01, 0x1B, 0xA8, 0x57)) return BTN_SEEKBW;
  if (bufferMatches(0x01, 0x1B, 0x28, 0xD7)) return BTN_SEEKFW;
  if (bufferMatches(0x01, 0x1B, 0xD8, 0x27)) return BTN_JMPBW;
  if (bufferMatches(0x01, 0x1B, 0x58, 0xA7)) return BTN_JMPFW;
  if (bufferMatches(0x01, 0x1B, 0x98, 0x67)) return BTN_STOP;
  if (bufferMatches(0x01, 0x1B, 0x70, 0x8F)) return BTN_MUTE;

  return BTN_UNKNOWN;
}

// Virtual Remote representing all buttons
void pressVirtualButton (int button) {
  switch(button) {
    case BTN_ONOFF:     sendIrWithParams(samsungOnOff);       break;
    case BTN_SOURCE:    pressSwitchButton(BTN_SOURCE);        break;
    case BTN_MENU:      modeSwitchOn = !modeSwitchOn;         break;
    case BTN_UP:        pressSwitchButton(BTN_UP);            break;
    case BTN_DOWN:      pressSwitchButton(BTN_DOWN);          break;
    case BTN_LEFT:      pressSwitchButton(BTN_LEFT);          break;
    case BTN_RIGHT:     pressSwitchButton(BTN_RIGHT);         break;
    case BTN_ENTER:     pressSwitchButton(BTN_ENTER);         break;
    case BTN_MORE:      pressSwitchButton(BTN_MORE);          break;
    case BTN_BACK:      pressSwitchButton(BTN_BACK);          break;
    case BTN_VOLUP:     sendIrWithParams(samsungVolUp);       break;
    case BTN_VOLDOWN:   sendIrWithParams(samsungVolDn);       break;
    case BTN_PLAYPAUSE: Keyboard.write(' ');                  break;
    case BTN_SEEKBW:    Keyboard.write(KEY_LEFT_ARROW);       break;
    case BTN_SEEKFW:    Keyboard.write(KEY_RIGHT_ARROW);      break;
    case BTN_JMPBW:     /* NOP */                             break;
    case BTN_JMPFW:     /* NOP */                             break;
    case BTN_STOP:      /* NOP */                             break;
    case BTN_MUTE:      sendIrWithParams(samsungVolMute);     break;
  }
}

void pressSwitchButton(int button) {
  if (button == BTN_SOURCE) {
    // I don't even want to know what the f*ck is going on here
    // This is an HDMI switch and I did not get it to work any
    // other way https://www.delock.com/produkt/18685/merkmale.html
    sendIrWithParams(hdmiRight);
    delay(250);
    sendIrWithParams(hdmiRight);
    return;
  }
  
  if(modeSwitchOn == 0) {
    switch(button) {
      case BTN_UP:        Keyboard.write(KEY_UP_ARROW);         break;
      case BTN_DOWN:      Keyboard.write(KEY_DOWN_ARROW);       break;
      case BTN_LEFT:      Keyboard.write(KEY_LEFT_ARROW);       break;
      case BTN_RIGHT:     Keyboard.write(KEY_RIGHT_ARROW);      break;
      case BTN_ENTER:     Keyboard.write(KEY_RETURN);           break;
      case BTN_BACK:      Keyboard.write(KEY_BACKSPACE);        break;
      case BTN_MORE:      Keyboard.write('c');                  break;
    }
  } else {
    switch(button) {
      case BTN_UP:        sendIrWithParams(samsungUp);          break;
      case BTN_DOWN:      sendIrWithParams(samsungDown);        break;
      case BTN_LEFT:      sendIrWithParams(samsungLeft);        break;
      case BTN_RIGHT:     sendIrWithParams(samsungRight);       break;
      case BTN_ENTER:     sendIrWithParams(samsungOk);          break;
      case BTN_BACK:      sendIrWithParams(samsungBack);        break;
      case BTN_MORE:      sendIrWithParams(samsungHome);        break;
    }
  }
}

// Buffer match helper
bool bufferMatches(int bit1, int bit2, int bit3, int bit4) {
  int compare[] = {bit1, bit2, bit3, bit4};

  for (int i = 0; i < rxBuffer[BIT_DATA_LEN]; i++) {
    if (compare[i] != rxBuffer[i + BIT_DATA]) return false;
  }

  return true;
}

// Infrared send helper
void sendIrWithParams(const int* cs) {
  txBuffer[BIT_LEN]        = 9;
  txBuffer[BIT_START_H]    = cs[0];
  txBuffer[BIT_START_L]    = cs[1];
  txBuffer[BIT_DATA_H]     = cs[2];
  txBuffer[BIT_DATA_L]     = cs[3];
  txBuffer[BIT_DATA_LEN]   = 4;
  txBuffer[BIT_DATA+0]     = cs[4];
  txBuffer[BIT_DATA+1]     = cs[5];
  txBuffer[BIT_DATA+2]     = cs[6];
  txBuffer[BIT_DATA+3]     = cs[7];

  IR.Send(txBuffer, IR_TX_FRQ);

  for (int i = 0; i < 20; i++) txBuffer[i] = 0x00;
  IR.Init(PIN_IR_RX);
}

// Debug method for printing received IR information
void debugIrBuffer() {
  Serial.println("+------------------------------------------------------+");
  Serial.print("LEN = ");
  Serial.println(rxBuffer[BIT_LEN]);
  Serial.print("START_H: ");
  Serial.print(rxBuffer[BIT_START_H]);
  Serial.print("\tSTART_L: ");
  Serial.println(rxBuffer[BIT_START_L]);

  Serial.print("DATA_H: ");
  Serial.print(rxBuffer[BIT_DATA_H]);
  Serial.print("\tDATA_L: ");
  Serial.println(rxBuffer[BIT_DATA_L]);

  Serial.print("\r\nDATA_LEN = ");
  Serial.println(rxBuffer[BIT_DATA_LEN]);

  Serial.print("DATA: ");
  for(int i=0; i<rxBuffer[BIT_DATA_LEN]; i++) {
    Serial.print("0x");
    Serial.print(rxBuffer[i+BIT_DATA], HEX);
    Serial.print(", ");
  }

  Serial.println();
  Serial.println("+------------------------------------------------------+\r\n\r\n");
}
