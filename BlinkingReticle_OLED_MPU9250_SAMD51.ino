#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU9250.h>
#include <Adafruit_Sensor.h>
#include <EEPROM.h>

// OLED display parameters
#define SCREEN_WIDTH 128 // OLED display width
#define SCREEN_HEIGHT 64 // OLED display height

// Filter parameter
#define ALPHA 0.1

// Blinking threshold
#define BLINK_THRESHOLD 0.1

// Blinking period (in milliseconds)
#define BLINK_PERIOD 1000

// Button pin
#define BUTTON_PIN 2

// Reticle types
#define RETICLE_TYPE_TRIANGLE 0
#define RETICLE_TYPE_CIRCLE 1
#define RETICLE_TYPE_DOT 2

// Creating display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

// Creating sensor object
Adafruit_MPU9250 mpu;

// Storing the previous acceleration data for hysteresis
float prevX = 0;
float prevY = 0;

// Reticle type
int reticleType = RETICLE_TYPE_TRIANGLE;

// Button press time
unsigned long buttonPressTime = 0;

// Blinking state
bool blinkState = false;
unsigned long lastBlinkTime = 0;

void setup() {
  Serial.begin(9600);

  // Checking if MPU9250 is available
  if (!mpu.begin()) {
    Serial.println("Could not find a valid MPU9250 sensor, check wiring!");
    while (1);
  }

  // Initial magnetometer calibration
  sensors_event_t a, g, m, temp;
  mpu.getEvent(&a, &g, &m, &temp);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  delay(1000);
  display.clearDisplay();

  // Initialize button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Load reticle type from EEPROM
  reticleType = EEPROM.read(0);
}

void loop() {
  // Getting sensor events
  sensors_event_t a, g, m, temp;
  mpu.getEvent(&a, &g, &m, &temp);

  // Checking button state
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (buttonPressTime == 0) {
      buttonPressTime = millis();
    } else if (millis() - buttonPressTime > 3000) {
      // Switching reticle type
      reticleType = (reticleType + 1) % 3;
      EEPROM.write(0, reticleType);
      buttonPressTime = 0;
    }
  } else {
    buttonPressTime = 0;
  }

  // Calculating position of reticle based on acceleration data with hysteresis (low-pass filter)
  float x = ALPHA * (SCREEN_WIDTH / 2 + a.acceleration.x) + (1 - ALPHA) * prevX;
  float y = ALPHA * (SCREEN_HEIGHT / 2 + a.acceleration.y) + (1 - ALPHA) * prevY;
  x = constrain(x, 0, SCREEN_WIDTH);
  y = constrain(y, 0, SCREEN_HEIGHT);

  // Check if the difference is below the threshold
  bool blinkCondition = (abs(a.acceleration.x - prevX) < BLINK_THRESHOLD) && (abs(a.acceleration.y - prevY) < BLINK_THRESHOLD);

  // Blinking
  if (blinkCondition) {
    if (millis() - lastBlinkTime > BLINK_PERIOD) {
      blinkState = !blinkState;
      lastBlinkTime = millis();
    }
  } else {
    blinkState = true;
  }

  // Storing the current data for the next iteration
  prevX = x;
  prevY = y;

  // Clearing the screen
  display.clearDisplay();

  if (blinkState) {
    // Drawing the reticle based on the selected type
    switch (reticleType) {
      case RETICLE_TYPE_TRIANGLE:
        display.drawTriangle(x-5, y-5, x, y+5, x+5, y-5, WHITE);
        break;
      case RETICLE_TYPE_CIRCLE:
        display.drawCircle(x, y, 5, WHITE);
        break;
      case RETICLE_TYPE_DOT:
        display.drawPixel(x, y, WHITE);
        break;
    }
  }

  // Displaying the reticle
  display.display();
}
