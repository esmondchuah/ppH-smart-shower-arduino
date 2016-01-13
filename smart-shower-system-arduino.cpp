// include the library code:
#include <LiquidCrystal.h>  // display unit library
#include <Keypad.h> // keypad library
#include <Servo.h>  // servo library

#define trigPin1 10
#define echoPin1 11
#define ledShower 12
#define ledAuto 9
#define echoPin2 24
#define trigPin2 22


long duration1, distance;
long duration2, phReading;
int buffer = 0;
int mode = 1; // manual = 0; auto = 1
int phTime = 0; // time since pH becomes neutral again
int phTurnOff = 0; // boolean value to overwrite the proximity system

Servo myservo;  // create servo object to control a servo
                // twelve servo objects can be created on most boards
int pos = 0;    // variable to store the servo position


// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 6, 2, 3, 4, 5);


const byte ROWS = 4; // Four rows
const byte COLS = 3; // Three columns

// Define the Keymap
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'#','0','*'}
};

// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte rowPins[ROWS] = { 21, 20, 19, 18 };
// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte colPins[COLS] = { 17, 16, 15  }; 

// Create the Keypad
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );



int seconds = 5; //start seconds

void timer() {
  lcd.begin(16, 2);
  lcd.print("  Shower Timer ");

  delay(150);

  for(int i=seconds; i>=0; i--){
    lcd.setCursor(4, 2);
    lcd.print("00:00:0");
    lcd.print(i);
    delay(1000);
  }
  trigger();
}


void trigger() {
  lcd.clear();         // clears the screen and buffer
  lcd.setCursor(3,0);
  lcd.print("Time is up");
  
  lcd.setCursor(1, 1); // set timer position on lcd for end.
  lcd.print("END OF SHOWER");

  turnDownServo();
  delay(1000);
  lcd.display();
}


void keyPresets() {
  char key = kpd.getKey();
 
  if(key=='1'){
    turnUpServo();
    seconds = 5;   
    timer();
    mode = 0;
  }
  if(key=='2'){
    turnUpServo();
    seconds = 15;    
    timer(); 
    mode = 1; 
  }
  if(key=='3'){
    turnUpServo();
    seconds = 35;      
    timer(); 
    mode = 1;
  }
  if(key=='0'){
    if(mode == 0){
      phTurnOff = 0;
      mode = 1;
      Serial.println("AUTO mode activated");
    }
    else if(mode == 1){
      mode = 0;
      myservo.write(180);
      Serial.println("MANUAL mode activated");
    }
  }
}


void turnUpServo(){
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);                // tell servo to go to position in variable 'pos'
    delay(20);                         // waits 30 ms for the servo to reach the position
  }
}


void turnDownServo() {
  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);                // tell servo to go to position in variable 'pos'
    delay(30);                         // waits 30 ms for the servo to reach the position
  }
}


void proxSensor() {
  // The following trigPin/echoPin cycle is used to determine the
  // distance of the nearest object by bouncing sound waves off of it.
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  
  digitalWrite(trigPin1, LOW);
  duration1 = pulseIn(echoPin1, HIGH);
  
  // Calculate the distance (in cm) based on the speed of sound.
  distance = duration1/58.2;
  Serial.print("proximity: ");
  Serial.println(distance);
  
  // Shower system when soaping
  // demonstrated using LED
  if (distance < 7 && buffer == 0) {
    Serial.println("shower turn back on gradually");
    buffer = 1;
    turnUpServo();
  }
  if (distance >= 7) {
    Serial.println("trickle shower");
    buffer = 0;
    myservo.write(170);
  }
}


void phSensor() {
  // The following trigPin/echoPin cycle is used to determine the
  // distance of the nearest object by bouncing sound waves off of it.
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  
  digitalWrite(trigPin2, LOW);
  duration2 = pulseIn(echoPin2, HIGH);
  
  // Calculate the distance (in cm) based on the speed of sound.
  phReading = duration2/58.2;
  Serial.print("pH: ");
  Serial.println(phReading);
  Serial.println(phTime);

  if (phReading < 7) {
    phTime = 0;
    phTurnOff = 0;
  }
  else {
    phTime += 1;
  }

  // if pH of waste water is neutral for 50 seconds, system overwrites to turn off the shower
  if (phTime > 40 && phTurnOff == 0) {
    phTurnOff = 1;
    turnDownServo();
    digitalWrite(ledShower, LOW);
  }
}


void setup() {
  // initialize the serial communications:
  Serial.begin(9600);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

 // proximity sensor
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);

  // LEDs
  pinMode(ledShower, OUTPUT);
  pinMode(ledAuto, OUTPUT);

  // pH sensor
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  myservo.attach(53);  // attaches the servo on pin 53 to the servo object
}


void loop() {
  if(mode == 0){
    digitalWrite(ledAuto, LOW);
  }
  else if(mode == 1){
    digitalWrite(ledAuto, HIGH);
  }

  keyPresets();
  if(mode == 1) {
    phSensor();
  }
  if(mode == 1 && phTurnOff == 0) { // proximity sensor enabled if auto mode and phTurnOff is not activated
    proxSensor();
  }
  delay(500);
}