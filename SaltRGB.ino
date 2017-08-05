// Logan Pulley 2017
// t.me/loganpulley

#include <IRremote.h>
#include <IRremoteInt.h>
#include <Adafruit_NeoPixel.h>

#define PIN_LED 3
#define PIN_IR 2
#define OFF 0
#define ON 1
#define SELECT 0
#define PATTERN 1
#define SELECT_RED 0
#define SELECT_GREEN 1
#define SELECT_BLUE 2
#define PATTERN_FADE 0
#define PATTERN_PULSE 1
#define PATTERN_CANDLE 2
#define IR_PREFIX 0x10EF
#define IR_POWER 0xD827
#define IR_A 0xF807
#define IR_B 0x7887
#define IR_C 0x58A7
#define IR_SEL 0x20DF
#define IR_UP 0xA05F
#define IR_DOWN 0x00FF
#define IR_RIGHT 0x807F
#define IR_LEFT 0x10EF

#define DEBUG 0
// 0 -> off
// 1 -> modes
// 2 -> modes, colors/speeds
// 3 -> modes, colors/speeds, IR codes

Adafruit_NeoPixel ring = Adafruit_NeoPixel(12, PIN_LED);
IRrecv ir(PIN_IR);
decode_results irData;
unsigned long lastMillis, sinceLastMillis;
int state = ON, mode = PATTERN, submodeSelect = SELECT_RED, submodePattern = PATTERN_FADE;
const int BRIGHTNESS_STEPS = 8;
const int COLOR_STEPS = 8;
int brightness = 255, brightnessStep = BRIGHTNESS_STEPS;
int selectRed = 255, selectRedStep = COLOR_STEPS, selectGreen = 255, selectGreenStep = COLOR_STEPS, selectBlue = 255, selectBlueStep = COLOR_STEPS;
int patternFadeSpeed = 0, patternPulseSpeed = 0, patternCandleSpeed = 0;
int patternFadeRed, patternFadeGreen, patternFadeBlue;
int patternPulseRed, patternPulseGreen, patternPulseBlue;
int patternCandleRed, patternCandleGreen, patternCandleBlue, patternCandleFlickerTimer = 0;
const int MAX_SPEED_OPTION = 4;

void setup() {
  #if DEBUG >= 1
  Serial.begin(9600);
  Serial.println("Hello!");
  #endif

  lastMillis = 0;

  ring.begin();
  ring.show();
  ir.enableIRIn();

  setAllPixels(0, 0, 0);
}

void loop() {
  sinceLastMillis = millis() - lastMillis;
  lastMillis = millis();
  
  uint16_t code = grabIR();

  if (state == OFF) {
    if (brightness != 0) {
      brightness = 0;
    }
    if (code == IR_POWER) {
      state = ON;
      #if DEBUG >= 1
      Serial.print("status ON (");
      Serial.print( (mode == SELECT) ? "SELECT-" : "PATTERN-" );
      Serial.print( (mode == SELECT) ? submodeSelect : submodePattern );
      Serial.print(")\n");
      #endif
    }
  } else if (state == ON) {
    if (brightness != curve(brightnessStep, BRIGHTNESS_STEPS)) {
      brightness = curve(brightnessStep, BRIGHTNESS_STEPS);
    }
    if (code == IR_POWER) {
      state = OFF;
      #if DEBUG >= 1
      Serial.println("status OFF");
      #endif
    }
    if (code == IR_UP) {
      if (brightnessStep < BRIGHTNESS_STEPS) {
        brightnessStep++;
        brightness = curve(brightnessStep, BRIGHTNESS_STEPS);
      }
    }
    if (code == IR_DOWN) {
      if (brightnessStep > 1) {
        brightnessStep--;
        brightness = curve(brightnessStep, BRIGHTNESS_STEPS);
      }
    }
    if (mode == SELECT) {
      if (code == IR_SEL) {
        mode = PATTERN;
        #if DEBUG >= 1
        Serial.print("mode PATTERN-");
        Serial.print(submodePattern);
        Serial.print("\n");
        #endif
        return;
      }
      if (code == IR_A) {
        submodeSelect = SELECT_RED;
        #if DEBUG >= 1
        Serial.println("submode SELECT_RED");
        #endif
      }
      if (code == IR_B) {
        submodeSelect = SELECT_GREEN;
        #if DEBUG >= 1
        Serial.println("submode SELECT_GREEN");
        #endif
      }
      if (code == IR_C) {
        submodeSelect = SELECT_BLUE;
        #if DEBUG >= 1
        Serial.println("submode SELECT_BLUE");
        #endif
      }
      if (code == IR_RIGHT) {
        if (submodeSelect == SELECT_RED && selectRedStep < COLOR_STEPS) {
          selectRedStep++;
          selectRed = curve(selectRedStep, COLOR_STEPS);
        }
        if (submodeSelect == SELECT_GREEN && selectGreenStep < COLOR_STEPS) {
          selectGreenStep++;
          selectGreen = curve(selectGreenStep, COLOR_STEPS);
        }
        if (submodeSelect == SELECT_BLUE && selectBlueStep < COLOR_STEPS) {
          selectBlueStep++;
          selectBlue = curve(selectBlueStep, COLOR_STEPS);
        }
        #if DEBUG >= 2
        Serial.print("R");
        Serial.print(selectRedStep);
        Serial.print(" G");
        Serial.print(selectGreenStep);
        Serial.print(" B");
        Serial.print(selectBlueStep);
        Serial.print("\n");
        #endif
      }
      if (code == IR_LEFT) {
        if (submodeSelect == SELECT_RED && selectRedStep > 0 && selectRedStep + selectGreenStep + selectBlueStep > 1) {
          selectRedStep--;
          selectRed = curve(selectRedStep, COLOR_STEPS);
        }
        if (submodeSelect == SELECT_GREEN && selectGreenStep > 0 && selectRedStep + selectGreenStep + selectBlueStep > 1) {
          selectGreenStep--;
          selectGreen = curve(selectGreenStep, COLOR_STEPS);
        }
        if (submodeSelect == SELECT_BLUE && selectBlueStep > 0 && selectRedStep + selectGreenStep + selectBlueStep > 1) {
          selectBlueStep--;
          selectBlue = curve(selectBlueStep, COLOR_STEPS);
        }
        #if DEBUG >= 2
        Serial.print("R");
        Serial.print(selectRedStep);
        Serial.print(" G");
        Serial.print(selectGreenStep);
        Serial.print(" B");
        Serial.print(selectBlueStep);
        Serial.print("\n");
        #endif
      }
    }
    if (mode == PATTERN) {
      if (code == IR_SEL) {
        mode = SELECT;
        #if DEBUG >= 1
        Serial.print("mode SELECT-");
        Serial.print(submodeSelect);
        Serial.print("\n");
        #endif
        return;
      }
      if (code == IR_A) {
        submodePattern = PATTERN_FADE;
        #if DEBUG >= 1
        Serial.println("submode PATTERN_FADE");
        #endif
      }
      if (code == IR_B) {
        submodePattern = PATTERN_PULSE;
        #if DEBUG >= 1
        Serial.println("submode PATTERN_PULSE");
        #endif
      }
      if (code == IR_C) {
        submodePattern = PATTERN_CANDLE;
        #if DEBUG >= 1
        Serial.println("submode PATTERN_CANDLE");
        #endif
      }
      if (code == IR_RIGHT) {
        if (submodePattern == PATTERN_FADE && patternFadeSpeed < MAX_SPEED_OPTION) {
          patternFadeSpeed++;
        }
        if (submodePattern == PATTERN_PULSE && patternPulseSpeed < MAX_SPEED_OPTION) {
          patternPulseSpeed++;
        }
        if (submodePattern == PATTERN_CANDLE && patternCandleSpeed < MAX_SPEED_OPTION) {
          patternCandleSpeed++;
        }
        #if DEBUG >= 2
        Serial.print("fade");
        Serial.print(patternFadeSpeed);
        Serial.print(" pulse");
        Serial.print(patternPulseSpeed);
        Serial.print(" candle");
        Serial.print(patternCandleSpeed);
        Serial.print("\n");
        #endif
      }
      if (code == IR_LEFT) {
        if (submodePattern == PATTERN_FADE && patternFadeSpeed > -MAX_SPEED_OPTION) {
          patternFadeSpeed--;
        }
        if (submodePattern == PATTERN_PULSE && patternPulseSpeed > -MAX_SPEED_OPTION) {
          patternPulseSpeed--;
        }
        if (submodePattern == PATTERN_CANDLE && patternCandleSpeed > -MAX_SPEED_OPTION) {
          patternCandleSpeed--;
        }
        #if DEBUG >= 2
        Serial.print("fade");
        Serial.print(patternFadeSpeed);
        Serial.print(" pulse");
        Serial.print(patternPulseSpeed);
        Serial.print(" candle");
        Serial.print(patternCandleSpeed);
        Serial.print("\n");
        #endif
      }
      if (submodePattern == PATTERN_FADE) {
        float b = 1 / (float)(-(patternFadeSpeed - 5) * 400 - 300);
        float C = 255;
        float H = millis() * b;
        float HH = (H / (PI / 3.0));
        float HHH = HH;
        while (HHH > 2) HHH -= 2;
        float X = C * (1 - abs(HHH - 1));
        if ((int)HH % 6 == 0) {
           patternFadeRed = C;
           patternFadeGreen = curve(X, 255);
           patternFadeBlue = 0;
        }
        if ((int)HH % 6 == 1) {
           patternFadeRed = curve(X, 255);
           patternFadeGreen = C;
           patternFadeBlue = 0;
        }
        if ((int)HH % 6 == 2) {
           patternFadeRed = 0;
           patternFadeGreen = C;
           patternFadeBlue = curve(X, 255);
        }
        if ((int)HH % 6 == 3) {
           patternFadeRed = 0;
           patternFadeGreen = curve(X, 255);
           patternFadeBlue = C;
        }
        if ((int)HH % 6 == 4) {
           patternFadeRed = curve(X, 255);
           patternFadeGreen = 0;
           patternFadeBlue = C;
        }
        if ((int)HH % 6 == 5) {
           patternFadeRed = C;
           patternFadeGreen = 0;
           patternFadeBlue = curve(X, 255);
        }
      }
      if (submodePattern == PATTERN_PULSE) {
        double patternPulseValue = 0.5 * sin(millis() / (float)(-(patternPulseSpeed - 5) * 100)) + 0.5;
        patternPulseRed = curve(selectRed * patternPulseValue, 255);
        patternPulseGreen = curve(selectGreen * patternPulseValue, 255);
        patternPulseBlue = curve(selectBlue * patternPulseValue, 255);
      }
      if (submodePattern == PATTERN_CANDLE) {
        patternCandleFlickerTimer -= sinceLastMillis;
        if (patternCandleFlickerTimer <= 0) {
          patternCandleRed = 255;
          patternCandleGreen = random(127 - (patternCandleSpeed + 4) * 3, 127 + (patternCandleSpeed + 4) * 3);
          patternCandleBlue = random(9 - (patternCandleSpeed + 4) * 1, 9 + (patternCandleSpeed + 4) * 1);
          patternCandleFlickerTimer += random(60 - (patternCandleSpeed + 4) * 4, 60 + (patternCandleSpeed + 4) * 4);
        }
      }
    }
  }

  if (ir.isIdle()) {
    updatePixels();
    ring.show();
  }
}

void updatePixels() {
  if (mode == SELECT) {
    setAllPixels(selectRed, selectGreen, selectBlue);
  }
  if (mode == PATTERN) {
    if (submodePattern == PATTERN_FADE) {
      setAllPixels(patternFadeRed, patternFadeGreen, patternFadeBlue);
    }
    if (submodePattern == PATTERN_PULSE) {
      setAllPixels(patternPulseRed, patternPulseGreen, patternPulseBlue);
    }
    if (submodePattern == PATTERN_CANDLE) {
      setAllPixels(patternCandleRed, patternCandleGreen, patternCandleBlue);
    }
  }
  ring.setBrightness(brightness);
}

void setAllPixels(int red, int green, int blue) {
  for (int i = 0; i < 12; i++) {
    ring.setPixelColor(i, red, green, blue);
  }
}

int curve(double value, int full) {
  return (int)(4.9 * pow(value / (full / 8.0), 1.9) + 0.5);
}

uint16_t grabIR() {
  if (ir.decode(&irData)) {
    uint32_t value = irData.value;
    #if DEBUG >= 3
    Serial.println(value & 0xFFFF, HEX);
    #endif
    ir.resume();
    if (value >> 16 == IR_PREFIX) {
      return value & 0xFFFF;
    }
  }
  
  return 0;
}
