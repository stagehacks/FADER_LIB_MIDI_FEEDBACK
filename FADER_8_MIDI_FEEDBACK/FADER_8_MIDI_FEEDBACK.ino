#include <ResponsiveAnalogRead.h>
#include <TeensyThreads.h>

//     _____ ______ _______ _______ _____ _   _  _____  _____ 
//    / ____|  ____|__   __|__   __|_   _| \ | |/ ____|/ ____|
//   | (___ | |__     | |     | |    | | |  \| | |  __| (___  
//    \___ \|  __|    | |     | |    | | | . ` | | |_ |\___ \.
//    ____) | |____   | |     | |   _| |_| |\  | |__| |____) |
//   |_____/|______|  |_|     |_|  |_____|_| \_|\_____|_____/.


// FADER TRIM SETTINGS
#define TOP 960
#define BOT 70
int faderTrimTop[8] = {TOP, TOP, TOP, TOP, TOP, TOP, TOP, TOP}; // ADJUST THIS IF A SINGLE FADER ISN'T READING 255 AT THE TOP OF ITS TRAVEL
int faderTrimBottom[8] = {BOT, BOT, BOT, BOT, BOT, BOT, BOT, BOT}; // ADJUST THIS IF A SINGLE FADER ISN'T READING 0 AT THE BOTTOM OF ITS TRAVEL

#define MOTOR_MIN_SPEED 180
#define MOTOR_MAX_SPEED 250
#define TOUCH_THRESHOLD 30

#define MIDI_LISTEN_CHANNEL 16

#define DEBUG false




elapsedMillis sinceBegin = 0;
elapsedMillis sinceMoved[8];
elapsedMillis sinceSent[8];
byte lastSentValue[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte mode[8] = {1, 1, 1, 1, 1, 1, 1, 1};
int target[8] = {128, 128, 128, 128, 128, 128, 128, 128};
int previous[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte readPins[8] = {A9, A8, A7, A6, A5, A4, A3, A2};
static byte MOTOR_PINS_A[8] = {0, 2, 4, 6, 8, 10, 24, 28};
static byte MOTOR_PINS_B[8] = {1, 3, 5, 7, 9, 12, 25, 29};
ResponsiveAnalogRead faders[8] = {
  ResponsiveAnalogRead(A9, true),
  ResponsiveAnalogRead(A8, true),
  ResponsiveAnalogRead(A7, true),
  ResponsiveAnalogRead(A6, true),
  ResponsiveAnalogRead(A5, true),
  ResponsiveAnalogRead(A4, true),
  ResponsiveAnalogRead(A3, true),
  ResponsiveAnalogRead(A2, true)
};




//  ______      _____  ______ _____    ________      ________ _   _ _______ 
// |  ____/\   |  __ \|  ____|  __ \  |  ____\ \    / /  ____| \ | |__   __|
// | |__ /  \  | |  | | |__  | |__) | | |__   \ \  / /| |__  |  \| |  | |   
// |  __/ /\ \ | |  | |  __| |  _  /  |  __|   \ \/ / |  __| | . ` |  | |   
// | | / ____ \| |__| | |____| | \ \  | |____   \  /  | |____| |\  |  | |   
// |_|/_/    \_\_____/|______|_|  \_\ |______|   \/   |______|_| \_|  |_|   

void faderHasMoved(byte i) {
  usbMIDI.sendControlChange (i, getFaderValue(i)/2, 1);
}

void onControlChange(byte channel, byte fader, byte value) {
  target[fader] = value*2;
}


//    __  __          _____ _   _   _      ____   ____  _____  
//   |  \/  |   /\   |_   _| \ | | | |    / __ \ / __ \|  __ \.
//   | \  / |  /  \    | | |  \| | | |   | |  | | |  | | |__) |
//   | |\/| | / /\ \   | | | . ` | | |   | |  | | |  | |  ___/.
//   | |  | |/ ____ \ _| |_| |\  | | |___| |__| | |__| | |     
//   |_|  |_/_/    \_\_____|_| \_| |______\____/ \____/|_|     

#define REST 0
#define MOTOR 1
#define TOUCH 2
void loop() {

  usbMIDI.read(MIDI_LISTEN_CHANNEL);

  for (byte i = 0; i < 8; i++) {
    faders[i].update();
    int distanceFromTarget = target[i] - getFaderValue(i);
    
    if (faders[i].hasChanged()) {
      sinceMoved[i] = 0;
      if (mode[i] != MOTOR && previous[i] - getFaderValue(i) != 0) {
        target[i] = -1;
        mode[i] = TOUCH;
      }
    } else if (mode[i] == REST && target[i] != -1 && abs(distanceFromTarget) > 4) {
      mode[i] = MOTOR;
    }

    if (mode[i] == TOUCH) {
      if (lastSentValue[i] == getFaderValue(i) && sinceMoved[i] > 900) {
        mode[i] = REST;
        target[i] = -1;
      } else if (sinceSent[i] > 30 && lastSentValue[i] != getFaderValue(i)) {
        sinceMoved[i] = 0;
        sinceSent[i] = 0;
        lastSentValue[i] = getFaderValue(i);
        faderHasMoved(i);
      }
    }

    if (mode[i] == MOTOR) {
      faders[i].disableSleep();
      byte motorSpeed = min(MOTOR_MAX_SPEED, MOTOR_MIN_SPEED + abs(distanceFromTarget / 4));

      if (abs(distanceFromTarget) < 2) {
        analogWrite(MOTOR_PINS_A[i], 255);
        analogWrite(MOTOR_PINS_B[i], 255);
        if (sinceMoved[i] > 10) {
          faders[i].enableSleep();
        }
        if (sinceMoved[i] > 200) {
          mode[i] = REST;
          target[i] = -1;
        }
      } else if (distanceFromTarget > 0) {
        sinceMoved[i] = 0;
        analogWrite(MOTOR_PINS_A[i], motorSpeed);
        analogWrite(MOTOR_PINS_B[i], 0);
      } else if (distanceFromTarget < 0) {
        sinceMoved[i] = 0;
        analogWrite(MOTOR_PINS_A[i], 0);
        analogWrite(MOTOR_PINS_B[i], motorSpeed);
      }
      
    } else {
      faders[i].enableSleep();
      analogWrite(MOTOR_PINS_A[i], 255);
      analogWrite(MOTOR_PINS_B[i], 255);
    }
    if(DEBUG){
      Serial.print(getFaderValue(i));
      Serial.print("\t");
    }
    previous[i] = getFaderValue(i);

  }
  if(DEBUG){
    Serial.println("");
  }
}



//    _____ _   _ _____ _______ 
//   |_   _| \ | |_   _|__   __|
//     | | |  \| | | |    | |   
//     | | | . ` | | |    | |   
//    _| |_| |\  |_| |_   | |   
//   |_____|_| \_|_____|  |_|   

void setup() {
  Serial.begin(9600);
  delay(1500);
  mainInit();
}

byte networkThreadID = 0;
void mainInit() {
  Serial.println("###############################");
  Serial.println("Setting up Faders...");

  for (byte i = 0; i < 8; i++) {
    pinMode(MOTOR_PINS_A[i], OUTPUT);
    pinMode(MOTOR_PINS_B[i], OUTPUT);
    digitalWrite(MOTOR_PINS_A[i], LOW);
    digitalWrite(MOTOR_PINS_B[i], LOW);
    analogWriteFrequency(MOTOR_PINS_A[i], 18000);
    analogWriteFrequency(MOTOR_PINS_B[i], 18000);
    faders[i].setActivityThreshold(TOUCH_THRESHOLD);
  }
  usbMIDI.setHandleControlChange(onControlChange);
}



//    _    _ ______ _      _____  ______ _____   _____ 
//   | |  | |  ____| |    |  __ \|  ____|  __ \ / ____|
//   | |__| | |__  | |    | |__) | |__  | |__) | (___  
//   |  __  |  __| | |    |  ___/|  __| |  _  / \___ \.
//   | |  | | |____| |____| |    | |____| | \ \ ____) |
//   |_|  |_|______|______|_|    |______|_|  \_\_____/.

int getFaderValue(byte fader) {
  return max(0, min(255, map(faders[fader].getValue(), faderTrimBottom[fader], faderTrimTop[fader], 0, 255)));
}
