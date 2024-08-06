/*
  This is a Smart Tea Machine code written for an Arduino.
  It automates the process of making tea, including dispensing water,
  dipping a tea bag, dispensing sugar, and alerting the user when the tea is ready.
*/

// Libraries
#include <Servo.h> // Library for controlling servo motors
#include <LiquidCrystal_I2C.h> // Library for I2C LCD
#include <Wire.h> // Library for I2C communication
#include <dht.h> // Library for DHT temperature and humidity sensor

// Object instantiation
dht DHT; // DHT sensor object
Servo waterservo; // Servo motor for water dispensing
Servo teaservo; // Servo motor for dipping tea bag
LiquidCrystal_I2C lcd_1(0x27, 16, 2); // LCD object

// Variables initialization
int pos = 0; // Position for water servo motor
int pos2 = 0; // Position for tea servo motor
int count = 0; // Counter for tea dipping (for testing)
int count1 = 0; // Counter for water dispensing (for testing)
#define DHT11_PIN A0 // Pin for DHT sensor
int waterbuttonPin = 2; // Pin for water button
int sugarbuttonPin = 3; // Pin for sugar button
int waterSensorPin = A1; // Pin for water level sensor
const int buzzerPin = 4; // Pin for buzzer
int dcPin = 5; // Pin for DC motor
int motionSensorPin = A2; // Pin for motion sensor
bool waterbuttonPressed = false; // Flag to track water button press
bool sugarbuttonPressed = false; // Flag to track sugar button press
bool userInfront = false; // Flag to track if user is in front of the machine
bool userAccepted = false; // Flag to track RFID acceptance
bool checkRFID = false; // Flag for RFID card check

// RFID
#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN); // MFRC522 instance


void setup() {

  // Initialize the LCD
  lcd_1.init(); // Initialize LCD display
  lcd_1.backlight(); // Turn on LCD backlight
  
  // Attach servo motors
  waterservo.attach(7); // Attach water servo to pin 7
  teaservo.attach(6); // Attach tea servo to pin 6
  
  // Set pin modes
  pinMode(buzzerPin, OUTPUT); // Set the BUZZER pin as output
  pinMode(dcPin, OUTPUT); // Set the DC motor pin as output
  pinMode(waterbuttonPin, INPUT); // Set the water button pin as input
  Serial.begin(9600); // Initialize serial communication for printing to the serial monitor
  
  // Initialize RFID
  SPI.begin(); // Initialize SPI bus
  mfrc522.PCD_Init(); // Initialize MFRC522
  
  // Display title on LCD screen
  lcd_1.clear(); // Clear the LCD screen
  lcd_1.begin(16, 2); // Set LCD size to 16x2
  lcd_1.print("Smart Tea"); // Print "Smart Tea" on the first line
  lcd_1.setCursor(0, 1); // Move cursor to the second line
  lcd_1.print("Machine"); // Print "Machine" on the second line
  delay(3000); // Delay for 3 seconds
  Serial.println("---------------------Start Machine---------------------"); // Print start message to serial monitor
}


void loop() {
  // Read motion sensor
  int sensorState = digitalRead(motionSensorPin);
  
  // If motion sensor detects user, start welcomeUser() function
  if (sensorState = HIGH && userInfront == false) {
    Serial.println("Motion detected!");
    userInfront = true;
    welcomeUser();
  }

  // Check if the water button is pressed
  if (digitalRead(waterbuttonPin) == HIGH && !waterbuttonPressed) {
    waterbuttonPressed = true; // Set buttonPressed to true to indicate the button has been pressed
    Serial.println("Button pressed. Dispensing water.");
    delay(1000);
    dispenseWater(); // Call the function to dispense water 
    delay(2000);
    dipTeaBag(); // Call function to dispense tea bag in 
    
    // Let the user know sugar is going to dispense 
    Serial.println("Tell User to press sugar button!");
    lcd_1.clear(); // Clear the LCD screen
    lcd_1.setCursor(0, 0);
    lcd_1.print("Press ");
    lcd_1.setCursor(0, 1);
    lcd_1.print("sugar button!");
  }
  
  // Check if the sugar button is pressed
  if (digitalRead(sugarbuttonPin) == HIGH && !sugarbuttonPressed) {
    sugarbuttonPressed = true; // Set buttonPressed to true to indicate the button has been pressed
    Serial.println("Button pressed. Sugar Dispense.");
    sugarDispense(); // Call the function to dispense sugar

    // Prompt user to tap card
    lcd_1.clear();
    lcd_1.setCursor(0, 0);
    lcd_1.print("Tap card");
    lcd_1.setCursor(0, 1);
    lcd_1.print(" ");
  }

  // RFID
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  
  // Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  content.toUpperCase();

  // Check RFID content
  if (content.substring(1) == "33 2C B6 FC" && checkRFID == true) { // Change here the UID of card/cards or tag/tags that you want to give access
    userAccepted = true;

    Serial.print("Message: ");
    Serial.println("Access Granted");
    lcd_1.clear();
    lcd_1.setCursor(0, 0);
    lcd_1.print("Card accepted!");
    lcd_1.setCursor(0, 1);
    lcd_1.print(" ");
    Serial.println();
    
    delay(5000);

    lcd_1.clear();
    lcd_1.setCursor(0, 0);
    lcd_1.print("Points");
    lcd_1.setCursor(0, 1);
    lcd_1.print("are added!");

    delay(5000);

    lcd_1.clear(); 
    lcd_1.setCursor(0, 0);
    lcd_1.print("Goodbye!");
    lcd_1.setCursor(0, 1);
    lcd_1.print(":)");

    userInfront = false;
  }
  else if (content.substring(1) != "33 2C B6 FC" && checkRFID == true) {
    Serial.println("Access denied");
    lcd_1.clear();
    lcd_1.setCursor(0, 0);
    lcd_1.print("Card");
    lcd_1.setCursor(0, 1);
    lcd_1.print("NOT accepted!!");
  }
}



// Function that runs when user is in front of the motion sensor
void welcomeUser() {
  // Display welcome message on LCD
  lcd_1.begin(16, 2);
  lcd_1.print("Welcome user");
  delay(5000);
  
  // Check water temperature
  checkWaterTemperature();
  delay(2000); // Delay to display the welcome message for 2 seconds
  
  // Prompt user to press button for water
  Serial.println("Ask user if they want water to dispense.");
  lcd_1.clear(); // Clear the LCD screen
  lcd_1.setCursor(0, 0);
  lcd_1.print("For water,");
  lcd_1.setCursor(0, 1);
  lcd_1.print("Press button."); 
}

// Function that checks water temperature
void checkWaterTemperature() {
  // Read DHT Temp 
  Serial.println("Checking water temperature.");
  int chk = DHT.read11(DHT11_PIN);
  Serial.println("Water temperature = ");
  Serial.print(DHT.temperature);
  delay(1000);
  int temp= DHT.temperature;

  // Display temperature
  lcd_1.clear(); // Clear the LCD screen
  lcd_1.setCursor(0, 0);
  lcd_1.print("Checking water ");
  lcd_1.setCursor(0, 1);
  lcd_1.print("temp: ");
  lcd_1.print(temp);
  delay(5000);
  
  // Continuously check temperature until it's at least 20.0°C
  while (temp < 20.0) {
    lcd_1.clear(); // Clear the LCD screen
    lcd_1.setCursor(0, 0);
    lcd_1.print("Water not hot");
    lcd_1.setCursor(0, 1);
    lcd_1.print("temp: ");
    lcd_1.print(temp);
    delay(5000);

    lcd_1.clear(); // Clear the LCD screen
    lcd_1.setCursor(0, 0);
    lcd_1.print("Replace water");
    lcd_1.setCursor(0, 1);
    lcd_1.print("Add hot water");
    delay(10000);

    int chk = DHT.read11(DHT11_PIN);
    Serial.print("Temperature = ");
    Serial.print(DHT.temperature);
    delay(1000);
    int temp= DHT.temperature;
  }

  // Water temperature is at least 20.0°C
  lcd_1.clear(); // Clear the LCD screen
  lcd_1.setCursor(0, 0);
  lcd_1.print("Water is perfect");
  lcd_1.setCursor(0, 1);
  lcd_1.print("temp: ");
  lcd_1.print(temp);
  delay(5000);
}

// Function that dispenses water
void dispenseWater() {
  Serial.println("Water servo rotating to 90 degrees.");
  
  // Display on LCD that water is being dispensed 
  lcd_1.clear();
  lcd_1.setCursor(0, 0);
  lcd_1.print("Water is");
  lcd_1.setCursor(0, 1);
  lcd_1.print("dispensing now");  
  
  // Turn servo motor for water to pass through
  delay(500); // Delay for 0.5 seconds
  
  int waterLevel = readWaterLevel();  // Assuming readWaterLevel() is a function to read the water level from the sensor
    
  // Print updated water level
  Serial.println("Updated Water Level: " + String(waterLevel));
  
  // Continue dispensing water until water level reaches 0
  while(readWaterLevel()>250) {
    for (pos2 = 0; pos2 <= 30; pos2 += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      waterservo.write(pos2);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    for (pos2 = 30; pos2 >= 0; pos2 -= 1) { // goes from 180 degrees to 0 degrees
      waterservo.write(pos2);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position   
    }
    count1++;
    Serial.println(count1);
    
    // Read updated water level from the sensor
    int waterLevel = readWaterLevel();  // Assuming readWaterLevel() is a function to read the water level from the sensor
    
    // Print updated water level
    Serial.println("Updated Water Level: " + String(waterLevel));
    count1++;
  } 
  
  // Display on LCD that water dispensing is done
  lcd_1.clear();
  lcd_1.setCursor(0, 0);
  lcd_1.print("Water is done");
  lcd_1.setCursor(0, 1);
  lcd_1.print("");
  delay(5000);
  lcd_1.clear();
}

// Function to read water level from sensor 
int readWaterLevel() {
  return analogRead(A1);         // send current reading
}

// Function that dips tea bag in
void dipTeaBag() {
  // Display on LCD that is tea bag is being dispensed 
  lcd_1.clear();
  lcd_1.setCursor(0, 0);
  lcd_1.print("Tea bag is");
  lcd_1.setCursor(0, 1);
  lcd_1.print("dispensing now");  

  // Dip the tea bag into the cup by rotating the servo to 90 degrees
  // teaservo.write(180);
  Serial.println("Tea servo dipping tea bag.");
  delay(500); // Delay for 0.5 seconds

  while(count < 10) {
    for (pos = 60; pos <= 100; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      teaservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
  
    for (pos = 100; pos >= 60; pos -= 1) { // goes from 180 degrees to 0 degrees
      teaservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position   
    }
    count++;
  }

  // Display on LCD that is tea bag is being dispensed 
  lcd_1.clear();
  lcd_1.setCursor(0, 0);
  lcd_1.print("Tea is brewing");
  lcd_1.setCursor(0, 1);
  
  // Call function to display brewing time while tea bag is dipped in 
  displayBrewingTimer();
    
  // After the brewing time is over, lift the tea bag out of the cup
  teaservo.write(90);
  Serial.println("Tea servo lifting tea bag");
  
  // Display on LCD that is tea bag is done brewing 
  lcd_1.clear();
  lcd_1.setCursor(0, 0);
  lcd_1.print("Tea is done");
  
  // Buzzer goes off since Tea is done brewing
  Serial.println("Buzzer turns on to indicate buzzer tea is done brewing");
  tone(buzzerPin, 3000); // Send 1KHz sound signal...
  delay(5000);        // ...for 1 sec
  noTone(buzzerPin);     // Stop sound...
  lcd_1.clear();
}

// This function will display the timer for brewing
void displayBrewingTimer() {
  // Calculate the end time of brewing (10 seconds from now)
  unsigned long endTime = millis() + 10000; // 10 seconds in milliseconds
  unsigned long currentTime = millis();
  int remainingTime = 10; // Initial value in seconds
  
  while (currentTime < endTime) {
    // Update remaining time every second
    lcd_1.setCursor(0, 1); // Set cursor to the position where the timer will be displayed
    if (remainingTime < 10) {
        lcd_1.print("0"); // Print leading zero if remaining time is less than 10
    }
    lcd_1.print(remainingTime); // Print remaining time
    delay(1000); // Delay for 1 second
    currentTime = millis(); // Update current time
    remainingTime = (endTime - currentTime) / 1000; // Calculate remaining time in seconds
    lcd_1.setCursor(0, 1);
    lcd_1.print(" "); // Clear the previous number
  } 
}

// Function that will dispense sugar packet
void sugarDispense() {
  lcd_1.clear(); // Clear the LCD screen
  lcd_1.setCursor(0, 0);
  lcd_1.print("Dispensing");
  lcd_1.setCursor(0, 1);
  lcd_1.print("sugar");
  Serial.println("DC motor is running to dispense the sugar");
  analogWrite(dcPin, 100);
  delay(10000); // Adjust for how long it takes for sugar to finish pushing
  analogWrite(dcPin, 0);
  lcd_1.clear();
  checkRFID = true;
  teaDone();
}

// Function when tea is done
void teaDone() {
  lcd_1.clear(); // Clear the LCD screen
  lcd_1.setCursor(0, 0);
  lcd_1.print("Tea is");
  lcd_1.setCursor(0, 1);
  lcd_1.print("Done!");

  Serial.println("Buzzer turns on to indicate buzzer tea is done");
  tone(buzzerPin, 3000); // Send 1KHz sound signal...
  delay(5000);        // ...for 1 sec
  noTone(buzzerPin);     // Stop sound...
  
  lcd_1.clear(); // Clear the LCD screen
  lcd_1.setCursor(0, 0);
  lcd_1.print("Perfect Tea!");
  lcd_1.setCursor(0, 1);
  lcd_1.print("Thank you!");
  delay(5000);

  lcd_1.clear(); // Clear the LCD screen
  lcd_1.setCursor(0, 0);
  lcd_1.print("Scan Card!");
  lcd_1.setCursor(0, 1);
  lcd_1.print("For Points!!");
  delay(5000);
}
