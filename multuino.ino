// MULTUINO
//
// Arduino Micro based software for relaying commands from one remote, received
// via an IR receiver to either IR devices (via IR transmitter) or computer (via
// HID keyboard)

#include <Keyboard.h>
#include <IRSendRev.h>


#define DEBUG_MODE      false

#define BIT_LEN         0
#define BIT_START_H     1
#define BIT_START_L     2
#define BIT_DATA_H      3
#define BIT_DATA_L      4
#define BIT_DATA_LEN    5
#define BIT_DATA        6

#define BTN_UNKNOWN     0
#define BTN_ONOFF       1
#define BTN_ONOFF_TV    2
#define BTN_ONOFF_RCV   3
#define BTN_UP          4   
#define BTN_DOWN        5
#define BTN_LEFT        6
#define BTN_RIGHT       7
#define BTN_ENTER       8
#define BTN_MORE        9
#define BTN_BACK       10
#define BTN_VOLUP      11
#define BTN_VOLDOWN    12
#define BTN_PLAYPAUSE  13
#define BTN_SEEKBW     14
#define BTN_SEEKFW     15
#define BTN_JMPBW      16
#define BTN_JMPFW      17
#define BTN_STOP       18

#define IRC_ONOFF       1
#define IRC_ONOFF_TV    2
#define IRC_ONOFF_RCV   3
#define IRC_SOURCE      4
#define IRC_VOLUP       5
#define IRC_VOLDOWN     6

#define KBM_MUSIC       1
#define KBM_KODI        2
#define KBM_EMPTY       3

#define PIN_IR_RX       5
#define PIN_IR_TX       6  // NOP

#define IR_TX_FRQ       38 // 38kHz


unsigned char rxBuffer[20];
unsigned char txBuffer[20];


void setup() {
  IR.Init(PIN_IR_RX);

  #if DEBUG_MODE == true
    Serial.begin(9600);
    Serial.println("Initialized multuino in debug mode!");
  #endif
}


void loop() {
  if(IR.IsDta()) {
    IR.Recv(rxBuffer);

    #if DEBUG_MODE == true
      debugIrBuffer();
    #else
      int pressedButton = getPhysicalButton();
      if (pressedButton != BTN_UNKNOWN) {
        bool sentIr = pressVirtualButton(pressedButton);
        if (sentIr == true) {
          for (int i = 0; i < 20; i++) txBuffer[i] = 0x00;
          IR.Init(PIN_IR_RX);
        }
      }
    #endif
  }
}


// Physical button mapper
// In this case: XBOX ONE media remote
int getPhysicalButton () {
  if (rxBuffer[BIT_DATA_H] <  9 || rxBuffer[BIT_DATA_H] > 13) return BTN_UNKNOWN;
  if (rxBuffer[BIT_DATA_L] < 31 || rxBuffer[BIT_DATA_L] > 35) return BTN_UNKNOWN;
  if (rxBuffer[BIT_DATA_LEN] != 4) return BTN_UNKNOWN;

  if (bufferMatches(0x01, 0x1B, 0x26, 0xD9)) return BTN_ONOFF;
  if (bufferMatches(0x01, 0x1B, 0x76, 0x89)) return BTN_ONOFF_TV;
  if (bufferMatches(0x01, 0x1B, 0xF6, 0x09)) return BTN_ONOFF_RCV;
  if (bufferMatches(0x01, 0x1B, 0x78, 0x87)) return BTN_UP;
  if (bufferMatches(0x01, 0x1B, 0xF8, 0x07)) return BTN_DOWN;
  if (bufferMatches(0x01, 0x1B, 0x04, 0xFB)) return BTN_LEFT;
  if (bufferMatches(0x01, 0x1B, 0x84, 0x7B)) return BTN_RIGHT;
  if (bufferMatches(0x01, 0x1B, 0x44, 0xBB)) return BTN_ENTER;
  if (bufferMatches(0x01, 0x1B, 0xC4, 0x3B)) return BTN_BACK;
  if (bufferMatches(0x01, 0x1B, 0x64, 0x9B)) return BTN_MORE;
  if (bufferMatches(0x01, 0x1B, 0x08, 0xF7)) return BTN_VOLUP;
  if (bufferMatches(0x01, 0x1B, 0x88, 0x77)) return BTN_VOLDOWN;
  if (bufferMatches(0x01, 0x1B, 0x0E, 0xF1)) return BTN_PLAYPAUSE;
  if (bufferMatches(0x01, 0x1B, 0xA8, 0x57)) return BTN_SEEKBW;
  if (bufferMatches(0x01, 0x1B, 0x28, 0xD7)) return BTN_SEEKFW;
  if (bufferMatches(0x01, 0x1B, 0xD8, 0x27)) return BTN_JMPBW;
  if (bufferMatches(0x01, 0x1B, 0x58, 0xA7)) return BTN_JMPFW;
  if (bufferMatches(0x01, 0x1B, 0x98, 0x67)) return BTN_STOP;

  return BTN_UNKNOWN;
}


// Buffer match helper
bool bufferMatches(int bit1, int bit2, int bit3, int bit4) {
  int compare[] = {bit1, bit2, bit3, bit4};

  for (int i = 0; i < rxBuffer[BIT_DATA_LEN]; i++) {
    if (compare[i] != rxBuffer[i + BIT_DATA]) return false;
  }

  return true;
}


// Virtual Remote representing all buttons
bool pressVirtualButton (int button) {
  switch(button) {
    case BTN_ONOFF:     dispatchInfraRed(IRC_ONOFF);          return true;  break;
    case BTN_ONOFF_TV:  dispatchInfraRed(IRC_ONOFF_TV);       return true;  break;
    case BTN_ONOFF_RCV: dispatchInfraRed(IRC_ONOFF_RCV);      return true;  break;
    case BTN_UP:        Keyboard.write(KEY_UP_ARROW);         return false; break;
    case BTN_DOWN:      Keyboard.write(KEY_DOWN_ARROW);       return false; break;
    case BTN_LEFT:      Keyboard.write(KEY_LEFT_ARROW);       return false; break;
    case BTN_RIGHT:     Keyboard.write(KEY_RIGHT_ARROW);      return false; break;
    case BTN_ENTER:     Keyboard.write(KEY_RETURN);           return false; break;
    case BTN_MORE:      Keyboard.write('c');                  return false; break;
    case BTN_BACK:      Keyboard.write(KEY_BACKSPACE);        return false; break;
    case BTN_VOLUP:     dispatchInfraRed(IRC_VOLUP);          return true;  break;
    case BTN_VOLDOWN:   dispatchInfraRed(IRC_VOLDOWN);        return true;  break;
    case BTN_PLAYPAUSE: Keyboard.write(' ');                  return false; break;
    case BTN_SEEKBW:    Keyboard.write(KEY_LEFT_ARROW);       return false; break;
    case BTN_SEEKFW:    Keyboard.write(KEY_RIGHT_ARROW);      return false; break;
    case BTN_JMPBW:     dispatchKeyboardModified(KBM_MUSIC);  return false; break;
    case BTN_JMPFW:     dispatchKeyboardModified(KBM_KODI);   return false; break;
    case BTN_STOP:      dispatchKeyboardModified(KBM_EMPTY);  return false; break;
  }
}


// Keyboard modify dispatcher
void dispatchKeyboardModified (int command) {
  Keyboard.press(KEY_LEFT_GUI);
  delay(50);

  switch (command) {
    case KBM_MUSIC:  Keyboard.write('8');                     break;
    case KBM_KODI:   Keyboard.write('9');                     break;
    case KBM_EMPTY:  Keyboard.write('d');                     break;
  }

  delay(50);
  Keyboard.release(KEY_LEFT_GUI);
}


// Infrared Dispatcher
void dispatchInfraRed (int command) {
  const int samsungTvOnOff[] = {91, 90, 10, 34, 0xE0, 0xE0, 0x40, 0xBF};
  const int onkyoRcvOnOff[]  = {180, 88, 11, 33, 0x4B, 0x36, 0xD3, 0x2C};
  const int onkyoRcvVolUpR[] = {180, 88, 11, 33, 0x4B, 0xB6, 0x40, 0xBF};
  const int onkyoRcvVolUpL[] = {180, 88, 11, 33, 0x20, 0xDF, 0x40, 0xBF};
  const int onkyoRcvVolDnR[] = {180, 88, 11, 33, 0x4B, 0xB6, 0xC0, 0x3F};
  const int onkyoRcvVolDnL[] = {180, 88, 11, 33, 0x20, 0xDF, 0xC0, 0x3F};

  switch (command) {
    case IRC_ONOFF:
      sendIrWithParams(onkyoRcvOnOff);
      delay(50);
      sendIrWithParams(samsungTvOnOff);
      delay(50);
    break;
    case IRC_ONOFF_TV:  sendIrWithParams(samsungTvOnOff);     break;
    case IRC_ONOFF_RCV: sendIrWithParams(onkyoRcvOnOff);      break;
    case IRC_SOURCE:    /* NOP */                             break;
    case IRC_VOLUP:     sendIrWithParams(onkyoRcvVolUpR);     break;
    case IRC_VOLDOWN:   sendIrWithParams(onkyoRcvVolDnR);     break;
  } 
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



