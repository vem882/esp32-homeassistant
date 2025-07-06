/*
 * ESP32-2432S028R SD Card Test
 * This is a minimal test to verify SD card functionality
 * Use this if the main sketch fails to initialize the SD card
 */

#include <SPI.h>
#include <SD.h>

// Pin definitions for ESP32-2432S028R
#define SD_CS 15        // Try 15 first, then 5
#define SD_CS_ALT 5     // Alternative CS pin
#define SD_CLK 14
#define SD_MISO 12
#define SD_MOSI 13

SPIClass hspi(HSPI);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== ESP32-2432S028R SD Card Test ===");
  
  // Test pins
  testPins();
  
  // Test SD card initialization
  if (testSDCard()) {
    Serial.println("SD card test PASSED!");
    listFiles();
    testReadWrite();
  } else {
    Serial.println("SD card test FAILED!");
    Serial.println("Check connections and try different CS pin");
  }
}

void loop() {
  // Nothing to do here
  delay(1000);
}

void testPins() {
  Serial.println("=== Pin Test ===");
  
  // Test CS pins
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  delay(10);
  Serial.print("CS pin 15 HIGH: ");
  Serial.println(digitalRead(SD_CS));
  
  digitalWrite(SD_CS, LOW);
  delay(10);
  Serial.print("CS pin 15 LOW: ");
  Serial.println(digitalRead(SD_CS));
  
  pinMode(SD_CS_ALT, OUTPUT);
  digitalWrite(SD_CS_ALT, HIGH);
  delay(10);
  Serial.print("CS pin 5 HIGH: ");
  Serial.println(digitalRead(SD_CS_ALT));
  
  digitalWrite(SD_CS_ALT, LOW);
  delay(10);
  Serial.print("CS pin 5 LOW: ");
  Serial.println(digitalRead(SD_CS_ALT));
  
  // Test other pins
  pinMode(SD_CLK, INPUT_PULLUP);
  pinMode(SD_MISO, INPUT_PULLUP);
  pinMode(SD_MOSI, INPUT_PULLUP);
  
  delay(10);
  Serial.print("CLK pin 14: ");
  Serial.println(digitalRead(SD_CLK));
  Serial.print("MISO pin 12: ");
  Serial.println(digitalRead(SD_MISO));
  Serial.print("MOSI pin 13: ");
  Serial.println(digitalRead(SD_MOSI));
  
  Serial.println("Pin test completed");
}

bool testSDCard() {
  Serial.println("=== SD Card Initialization Test ===");
  
  // Try with primary CS pin (15)
  Serial.println("Trying CS pin 15...");
  if (initSD(SD_CS)) {
    Serial.println("Success with CS pin 15");
    return true;
  }
  
  // Try with alternative CS pin (5)
  Serial.println("Trying CS pin 5...");
  if (initSD(SD_CS_ALT)) {
    Serial.println("Success with CS pin 5");
    return true;
  }
  
  // Try with default SPI (VSPI)
  Serial.println("Trying default SPI...");
  if (SD.begin(SD_CS)) {
    Serial.println("Success with default SPI and CS pin 15");
    return true;
  }
  
  if (SD.begin(SD_CS_ALT)) {
    Serial.println("Success with default SPI and CS pin 5");
    return true;
  }
  
  return false;
}

bool initSD(int csPin) {
  SD.end();
  delay(100);
  
  hspi.end();
  hspi.begin(SD_CLK, SD_MISO, SD_MOSI, csPin);
  
  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, HIGH);
  delay(10);
  
  // Try different frequencies
  if (SD.begin(csPin, hspi, 4000000)) {
    Serial.println("  4 MHz: SUCCESS");
    return true;
  }
  
  SD.end();
  delay(100);
  
  if (SD.begin(csPin, hspi, 1000000)) {
    Serial.println("  1 MHz: SUCCESS");
    return true;
  }
  
  SD.end();
  delay(100);
  
  if (SD.begin(csPin, hspi)) {
    Serial.println("  Default frequency: SUCCESS");
    return true;
  }
  
  return false;
}

void listFiles() {
  Serial.println("=== SD Card Contents ===");
  
  File root = SD.open("/");
  if (!root) {
    Serial.println("Failed to open root directory");
    return;
  }
  
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("DIR: ");
      Serial.println(file.name());
    } else {
      Serial.print("FILE: ");
      Serial.print(file.name());
      Serial.print(" (");
      Serial.print(file.size());
      Serial.println(" bytes)");
    }
    file = root.openNextFile();
  }
  root.close();
}

void testReadWrite() {
  Serial.println("=== Read/Write Test ===");
  
  // Write test
  File testFile = SD.open("/test.txt", FILE_WRITE);
  if (testFile) {
    testFile.println("SD card test - write successful");
    testFile.close();
    Serial.println("Write test: PASSED");
  } else {
    Serial.println("Write test: FAILED");
    return;
  }
  
  // Read test
  testFile = SD.open("/test.txt", FILE_READ);
  if (testFile) {
    Serial.println("Read test content:");
    while (testFile.available()) {
      Serial.write(testFile.read());
    }
    testFile.close();
    Serial.println("Read test: PASSED");
  } else {
    Serial.println("Read test: FAILED");
  }
  
  // Cleanup
  SD.remove("/test.txt");
}
