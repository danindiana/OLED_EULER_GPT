#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU9250.h>
#include <Adafruit_Sensor.h>

// OLED display parameters
#define SCREEN_WIDTH 128 // OLED display width
#define SCREEN_HEIGHT 64 // OLED display height

// Filter parameter
#define ALPHA 0.1

// Creating display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

// Creating sensor object
Adafruit_MPU9250 mpu;

// Storing the previous acceleration data for hysteresis
float prevX = 0;
float prevY = 0;

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
}

void loop() {
  // Getting sensor events
  sensors_event_t a, g, m, temp;
  mpu.getEvent(&a, &g, &m, &temp);

  // Calculating position of reticle based on acceleration data with hysteresis (low-pass filter)
  float x = ALPHA * (SCREEN_WIDTH / 2 + a.acceleration.x) + (1 - ALPHA) * prevX;
  float y = ALPHA * (SCREEN_HEIGHT / 2 + a.acceleration.y) + (1 - ALPHA) * prevY;
  x = constrain(x, 0, SCREEN_WIDTH);
  y = constrain(y, 0, SCREEN_HEIGHT);

  // Storing the current data for the next iteration
  prevX = x;
  prevY = y;

  // Clearing the screen
  display.clearDisplay();

  // Drawing the triangle reticle
  display.drawTriangle(x-5, y-5, x, y+5, x+5, y-5, WHITE);
  
  // Displaying the reticle
  display.display();
}
