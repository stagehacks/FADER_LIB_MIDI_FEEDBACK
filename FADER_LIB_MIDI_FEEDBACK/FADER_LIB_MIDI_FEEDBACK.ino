// FADER TRIM SETTINGS
#define TOP 960
#define BOT 70
int faderTrimTop[8] = {TOP, TOP, TOP, TOP, TOP, TOP, TOP, TOP}; // ADJUST THIS IF A SINGLE FADER ISN'T READING 255 AT THE TOP OF ITS TRAVEL
int faderTrimBottom[8] = {BOT, BOT, BOT, BOT, BOT, BOT, BOT, BOT}; // ADJUST THIS IF A SINGLE FADER ISN'T READING 0 AT THE BOTTOM OF ITS TRAVEL

// MOTOR SETTINGS
#define MOTOR_MAX_SPEED 210
#define TOUCH_THRESHOLD 20

// ETHERNET SETTINGS
byte MAC_ADDRESS[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
int IP_ADDRESS[] = {192, 168, 1, 130};

// MIDI SETTINGS
#define MIDI_SEND_CHANNEL 1
#define MIDI_LISTEN_CHANNEL 16
byte MIDI_CONTROLS[8] = {0, 1, 2, 3, 4, 5, 6, 7};

#define FADER_COUNT 8
#define BUTTONS_ENABLED false

#define DEBUG false


void loop() {
  faderLoop();
  usbMIDI.read(MIDI_LISTEN_CHANNEL);

  if(BUTTONS_ENABLED){
    buttonLoop();
  }
  
}

void setup() {
  Serial.begin(9600);
  faderSetup();
  usbMIDI.setHandleControlChange(onControlChange);

  if(BUTTONS_ENABLED){
    buttonSetup();
  }
  
}

void ethernetSetup(){
  //
}

void faderHasMoved(byte i) {
  // Ensure USB Type (Tools > USB Type) is set to MIDI
  usbMIDI.sendControlChange (MIDI_CONTROLS[i], getFaderValue(i) / 4, MIDI_SEND_CHANNEL);
}

void onControlChange(byte channel, byte fader, byte value) {
  setFaderTarget(fader, value * 4);
}
