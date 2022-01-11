/**
 * Multuino - Custom Multi-Device Remote
 *
 * Refactored version using a new IR library
 * https://github.com/Arduino-IRremote/Arduino-IRremote
 */

#include <Keyboard.h>
#include <IRremote.hpp>

// When debug mode is enabled, no protocols will be relayed, instead protocol
// information will be output via Serial. This is an easy way to inspect the
// protocol of new devices and adding them to the lists below.
#define DRYRUN_MODE false
#define DEBUG_MODE true

// Define used pins for sending and receiving protocols. Connected to Grove IR
#define IR_RECEIVE_PIN 5
#define IR_SEND_PIN 6

// Global mode switch between TV menu and PC menu
bool pcModeEnabled = true;
bool tvModeEnabled = false;

void setup() {
  Serial.begin(9600);
  Serial.println("Initialized multuino");

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK);

  delay(1000);
}

void loop() {
  if (IrReceiver.decode()) {
    reactToIncoming(IrReceiver.decodedIRData);
    IrReceiver.resume();
  }
}

void reactToIncoming(IRData data) {
  #if DEBUG_MODE == true
    Serial.println("\n\n\nReceived protocol:");
    IrReceiver.printIRResultShort(&Serial);
  #endif
  
  // Ignore repeated protocols
  if ((data.flags & IRDATA_FLAGS_IS_REPEAT) != 0) return;

  // Only listen to XBOX remote
  if (data.protocol != NEC || data.address != 0xD880) return;

  #if DEBUG_MODE == true
    Serial.println("\nHandling protocol:");
    IrReceiver.printIRResultShort(&Serial);
  #endif
  
  #if DRYRUN_MODE == true
    Serial.println("\nDry-run mode, doing nothing!");
    return;
  #endif

  switch(data.command) {
    // Green on/off button -> Send samsung on off
    case 0x64:
      IrSender.sendSamsung(0x707, 0x02, 0);
      break;

    // Source button -> Send source step right to HDMI switch
    //
    // Protocol readout says 1 repeat with 40ms delay but for
    // some reason, that's buggy. 250ms works way better.
    case 0x6E: 
      IrSender.sendNEC(0x00, 0x16, 0);
      delay(250);
      IrSender.sendNEC(0x00, 0x16, 0);
      break;

    // Top menu button -> Toggle mode switch
    case 0x6F:
      pcModeEnabled = !pcModeEnabled;
      tvModeEnabled = !tvModeEnabled;
      break;

    // Arrow up -> Send Samsung up or PC up arrow
    case 0x1E:
      if (pcModeEnabled) Keyboard.write(KEY_UP_ARROW); 
      if (tvModeEnabled) IrSender.sendSamsung(0x707, 0x60, 0);
      break;

    // Arrow right -> Send Samsung right or PC right arrow
    case 0x21:
      if (pcModeEnabled) Keyboard.write(KEY_RIGHT_ARROW); 
      if (tvModeEnabled) IrSender.sendSamsung(0x707, 0x62, 0);
      break;

    // Arrow down -> Send Samsung down or PC down arrow
    case 0x1F:
      if (pcModeEnabled) Keyboard.write(KEY_DOWN_ARROW); 
      if (tvModeEnabled) IrSender.sendSamsung(0x707, 0x61, 0);
      break;

    // Arrow left -> Send Samsung left or PC left arrow
    case 0x20:
      if (pcModeEnabled) Keyboard.write(KEY_LEFT_ARROW); 
      if (tvModeEnabled) IrSender.sendSamsung(0x707, 0x65, 0);
      break;

    // OK Button -> Send Samsung OK or PC enter
    case 0x22:
      if (pcModeEnabled) Keyboard.write(KEY_RETURN); 
      if (tvModeEnabled) IrSender.sendSamsung(0x707, 0x68, 0);
      break;

    // Back button -> Send Samsung Back or PC backspace
    case 0x23:
      if (pcModeEnabled) Keyboard.write(KEY_BACKSPACE); 
      if (tvModeEnabled) IrSender.sendSamsung(0x707, 0x58, 0);
      break;

    // Buttom Menu button -> Send Samsung menu or PC c key
    case 0x26:
      if (pcModeEnabled) Keyboard.write('c'); 
      if (tvModeEnabled) IrSender.sendSamsung(0x707, 0x79, 0);
      break;

    // Vol up button -> Send samsung vol up
    case 0x10:
      IrSender.sendSamsung(0x707, 0x07, 0);
      break;

    // Vol down button -> Send samsung vol down
    case 0x11:
      IrSender.sendSamsung(0x707, 0x0B, 0);
      break;

    // Channel up button -> NOOP
    case 0x12: break;

    // Channel down button -> NOOP
    case 0x13: break;

    // Vol mute button -> Send samsung mute
    case 0x0E:
      IrSender.sendSamsung(0x707, 0x0F, 0);
      break;
    
    // Seek back button -> Send PC left arrow
    case 0x15:
      Keyboard.write(KEY_LEFT_ARROW);
      break;

    // Seek forward button -> Send PC right arrow
    case 0x14:
      Keyboard.write(KEY_RIGHT_ARROW); 
      break;

    // Play/Pause button -> Send PC space
    case 0x70:
      Keyboard.write(' ');
      break;

    // Skip back button
    case 0x1B: break;

    // Skip foreward button
    case 0x1A: break;

    // Stop button
    case 0x19: break;

    // Catchall
    default: break;
  }
}
