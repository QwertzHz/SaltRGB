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
#define PATTERN_FADE 1
#define PATTERN_PULSE 2
#define PATTERN_CANDLE 3
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
unsigned long lastMillis, sinceLastMillis, patternFadeTimer = 0;
int state = ON, mode = PATTERN_FADE;
const int BRIGHTNESS_STEPS = 8;
const int COLOR_STEPS = 8;
int brightness = 255, brightnessStep = BRIGHTNESS_STEPS;
const int SELECT_HUE_STEPS = 24;
int selectHueStep = 0;
int selectRed = 255, selectGreen = 0, selectBlue = 0;
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
    if (patternFadeTimer != 0) patternFadeTimer = 0;
    if (brightness != 0) {
      brightness = 0;
    }
    if (code == IR_POWER) {
      state = ON;
#if DEBUG >= 1
      Serial.println(mode);
#endif
    }
  } else if (state == ON) {
    patternFadeTimer += sinceLastMillis;
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
    if (code == IR_SEL) {
      mode = SELECT;
#if DEBUG >= 1
      Serial.println("SELECT");
#endif
      return;
    }
    if (code == IR_A) {
      mode = PATTERN_FADE;
      patternFadeTimer = 0;
#if DEBUG >= 1
      Serial.println("PATTERN_FADE");
#endif
    }
    if (code == IR_B) {
      mode = PATTERN_PULSE;
#if DEBUG >= 1
      Serial.println("PATTERN_PULSE");
#endif
    }
    if (code == IR_C) {
      mode = PATTERN_CANDLE;
#if DEBUG >= 1
      Serial.println("PATTERN_CANDLE");
#endif
    }
    if (mode == SELECT) {
      if (code == IR_RIGHT) {
        selectHueStep++;
        if (selectHueStep >= SELECT_HUE_STEPS) {
          selectHueStep -= SELECT_HUE_STEPS;
        }
        hueToRGB(2 * PI * selectHueStep / SELECT_HUE_STEPS, selectRed, selectGreen, selectBlue);
        selectRed = curve(selectRed, 255);
        selectGreen = curve(selectGreen, 255);
        selectBlue = curve(selectBlue, 255);
#if DEBUG >= 2
        Serial.println(selectHueStep);
#endif
      }
      if (code == IR_LEFT) {
        selectHueStep--;
        if (selectHueStep < 0) {
          selectHueStep += SELECT_HUE_STEPS;
        }
        hueToRGB(2 * PI * selectHueStep / SELECT_HUE_STEPS, selectRed, selectGreen, selectBlue);
        selectRed = curve(selectRed, 255);
        selectGreen = curve(selectGreen, 255);
        selectBlue = curve(selectBlue, 255);
#if DEBUG >= 2
        Serial.println(selectHueStep);
#endif
      }
    }
    if (code == IR_RIGHT && mode == PATTERN_FADE && patternFadeSpeed < MAX_SPEED_OPTION) {
      patternFadeSpeed++;
      patternFadeTimer = 0;
#if DEBIG >= 2
      Serial.println(patternFadeSpeed);
#endif
    }
    if (code == IR_RIGHT && mode == PATTERN_PULSE && patternPulseSpeed < MAX_SPEED_OPTION) {
      patternPulseSpeed++;
#if DEBUG >= 2
      Serial.println(patternPulseSpeed);
#endif
    }
    if (code == IR_RIGHT && mode == PATTERN_CANDLE && patternCandleSpeed < MAX_SPEED_OPTION) {
      patternCandleSpeed++;
#if DEBUG >= 2
      Serial.println(patternCandleSpeed);
#endif
    }
    if (code == IR_LEFT && mode == PATTERN_FADE && patternFadeSpeed > -MAX_SPEED_OPTION) {
      patternFadeSpeed--;
      patternFadeTimer = 0;
#if DEBUG >= 2
      Serial.println(patternFadeSpeed);
#endif
    }
    if (code == IR_LEFT && mode == PATTERN_PULSE && patternPulseSpeed > -MAX_SPEED_OPTION) {
      patternPulseSpeed--;
      patternFadeTimer = 0;
#if DEBUG >= 2
      Serial.println(patternPulseSpeed);
#endif
    }
    if (code == IR_LEFT && mode == PATTERN_CANDLE && patternCandleSpeed > -MAX_SPEED_OPTION) {
      patternCandleSpeed--;
#if DEBUG >= 2
      Serial.println(patternCandleSpeed);
#endif
    }
    if (mode == PATTERN_FADE) {
      float b = 1 / (float)(-(patternFadeSpeed - 5) * 400 - 300);
      int wrap = 200 * PI * (17 - 4 * patternFadeSpeed);
      if (patternFadeTimer >= wrap) patternFadeTimer -= wrap;
      hueToRGB(patternFadeTimer * b, patternFadeRed, patternFadeGreen, patternFadeBlue);
      patternFadeRed = curve(patternFadeRed, 255);
      patternFadeGreen = curve(patternFadeGreen, 255);
      patternFadeBlue = curve(patternFadeBlue, 255);
    }
    if (mode == PATTERN_PULSE) {
      double patternPulseValue = 0.5 * sin(millis() / (float)(-(patternPulseSpeed - 5) * 100)) + 0.5;
      patternPulseRed = selectRed * patternPulseValue;
      patternPulseGreen = selectGreen * patternPulseValue;
      patternPulseBlue = selectBlue * patternPulseValue;
    }
    if (mode == PATTERN_CANDLE) {
      patternCandleFlickerTimer -= sinceLastMillis;
      if (patternCandleFlickerTimer <= 0) {
        patternCandleRed = 255;
        patternCandleGreen = random(117 - (patternCandleSpeed + 4) * 3, 117 + (patternCandleSpeed + 4) * 3);
        patternCandleBlue = random(9 - (patternCandleSpeed + 4) * 1, 9 + (patternCandleSpeed + 4) * 1);
        patternCandleFlickerTimer += random(40 - (patternCandleSpeed + 4) * 4, 40 + (patternCandleSpeed + 4) * 4);
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
  if (mode == PATTERN_FADE) {
    setAllPixels(patternFadeRed, patternFadeGreen, patternFadeBlue);
  }
  if (mode == PATTERN_PULSE) {
    setAllPixels(patternPulseRed, patternPulseGreen, patternPulseBlue);
  }
  if (mode == PATTERN_CANDLE) {
    setAllPixels(patternCandleRed, patternCandleGreen, patternCandleBlue);
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

void hueToRGB(float hueAngle, int &red, int &green, int &blue) {
  int C = 255;
  float hueValue = (hueAngle / (PI / 3.0));
  int hueSixth = (int)hueValue;
  float hueValueFraction = hueValue - hueSixth;
  int X = C * (1 - abs(hueValueFraction + hueSixth % 2 - 1));
  if (hueSixth % 6 == 0) {
    red = C;
    green = X;
    blue = 0;
  }
  if (hueSixth % 6 == 1) {
    red = X;
    green = C;
    blue = 0;
  }
  if (hueSixth % 6 == 2) {
    red = 0;
    green = C;
    blue = X;
  }
  if (hueSixth % 6 == 3) {
    red = 0;
    green = X;
    blue = C;
  }
  if (hueSixth % 6 == 4) {
    red = X;
    green = 0;
    blue = C;
  }
  if (hueSixth % 6 == 5) {
    red = C;
    green = 0;
    blue = X;
  }
}

