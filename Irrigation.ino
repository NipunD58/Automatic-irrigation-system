#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// Define the pins for the sensors and actuators
#define SOIL_MOISTURE_PIN A1
#define WATER_PUMP_PIN 7
#define BUZZER_PIN 8
#define DHT_PIN 2
#define RAIN_SENSOR_PIN 3 // Digital pin for the HW-028 rain sensor

// Initialize the I2C LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Initialize the DHT sensor
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// Thresholds
const int soilMoistureLowThreshold = 30; // Adjust this value based on your sensor calibration
const int soilMoistureHighThreshold = 70; // Adjust this value based on your sensor calibration
const float temperatureThreshold = 30.0; // Adjust this value as needed
const float humidityThreshold = 60.0; // Adjust this value as needed

bool isBuzzerOn = false;
bool isWaterPumpOn = false;

void setup() {
  // Initialize the serial communication
  Serial.begin(9600);

  // Initialize the LCD
  lcd.init();
  lcd.backlight();

  // Display startup message
  lcd.setCursor(0, 0);
  lcd.print("MARS");
  delay(3000); // Display for 3 seconds

  // Initialize the DHT sensor
  dht.begin();

  // Initialize the rain sensor pin as input
  pinMode(RAIN_SENSOR_PIN, INPUT);

  // Initialize the water pump and buzzer pins as outputs
  pinMode(WATER_PUMP_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Turn off the water pump and buzzer initially
  digitalWrite(WATER_PUMP_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
}

void loop() {
  // Read the soil moisture value
  int sensor_analog = analogRead(SOIL_MOISTURE_PIN);
  float moisture_percentage = (100 - ((sensor_analog / 1023.0) * 100));
  Serial.print("Moisture Percentage = ");
  Serial.print(moisture_percentage);
  Serial.println("%");

  // Read the temperature and humidity values
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Check if any reads failed and exit early (to try again).
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);

  // Check for rain
  bool isRaining = digitalRead(RAIN_SENSOR_PIN) == LOW; // LOW means rain is detected
  if (isRaining) {
    Serial.println("Rain detected!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Rain detected!");
    digitalWrite(BUZZER_PIN, HIGH); // Turn on the buzzer to alert the farmer
    delay(2000); // Keep the message and buzzer on for 2 seconds
    digitalWrite(BUZZER_PIN, LOW); // Turn off the buzzer
  }

  // Display temperature and humidity on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.setCursor(6, 0);
  lcd.print(temperature);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Hum: ");
  lcd.setCursor(6, 1);
  lcd.print(humidity);
  lcd.print("%");
  delay(2000);

  // Check the soil moisture level
  if (moisture_percentage < soilMoistureLowThreshold) {
    // Turn on the water pump
    digitalWrite(WATER_PUMP_PIN, HIGH);
    if (!isWaterPumpOn) {
      Serial.println("Water Pump: ON");
      isWaterPumpOn = true;
    }
  } else if (moisture_percentage > soilMoistureHighThreshold) {
    // Turn off the water pump if soil is too wet
    digitalWrite(WATER_PUMP_PIN, LOW);
    if (isWaterPumpOn) {
      Serial.println("Water Pump: OFF");
      isWaterPumpOn = false;
    }
  }

  // Check the temperature and humidity
  if (moisture_percentage < soilMoistureLowThreshold || moisture_percentage > soilMoistureHighThreshold ||
      temperature > temperatureThreshold || humidity > humidityThreshold) {
    // Turn on the buzzer to alert the farmer
    digitalWrite(BUZZER_PIN, HIGH);
    if (!isBuzzerOn) {
      Serial.println("Buzzer: ON");
      isBuzzerOn = true;
    }
  } else {
    // Turn off the buzzer
    digitalWrite(BUZZER_PIN, LOW);
    if (isBuzzerOn) {
      Serial.println("Buzzer: OFF");
      isBuzzerOn = false;
    }
  }

  // Display soil moisture on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("soilM: ");
  lcd.setCursor(6, 0);
  lcd.print(moisture_percentage);
  lcd.print("%");
  delay(10000);


  // Display the water pump status on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pump: ");
  lcd.setCursor(6, 0);
  if (isWaterPumpOn) {
    lcd.print("ON");
  } else {
    lcd.print("OFF");
  }

  // Wait for a short period before the next loop
  delay(2000);
}
