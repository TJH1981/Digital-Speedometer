/*********************************************************************
Triumph Digital Speedometer
TJH1981 2018
Image code via LCD assistant
eeprom code from https://playground.arduino.cc/Code/EEPROMReadWriteLong
Code set to use motor encoder pulse output with 256 pulses / motor revolution
Further work: set trip reading to eeprom, smooth stepper motor etc.
Code will need calibrating to specific vehicle gear ratio, trigger wheel teeth, tyre diameter etc..
*********************************************************************/
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <EEPROM.h> 

float pulses = 0;
int dir = 8;
int stp = 9;
int stopinput = 3;
int encoderPin = 2; //arduino pin used for encoder signal
const int rpmmin = 0; // 
const int rpmmax = 10000;
long vehicleSpeed;
int speedoreading;
int speedoreadingold;
int speedoreadingvalue;
int startmileage;
int mileageincrement;
unsigned long mileage;
unsigned long newmileage;
float mileagenew;
unsigned int rpm;
unsigned long timeold;
const int pulsesperturn = 256; // insert pulses per revolution

void EEPROMWritelong(int address, long value)
      {
      //Decomposition from a long to 4 bytes by using bitshift.
      //One = Most significant -> Four = Least significant byte
      byte four = (value & 0xFF);
      byte three = ((value >> 8) & 0xFF);
      byte two = ((value >> 16) & 0xFF);
      byte one = ((value >> 24) & 0xFF);

      //Write the 4 bytes into the eeprom memory.
      EEPROM.write(address, four);
      EEPROM.write(address + 1, three);
      EEPROM.write(address + 2, two);
      EEPROM.write(address + 3, one);
      }
//This function will return a 4 byte (32bit) long from the eeprom
//at the specified address to address + 3.
long EEPROMReadlong(long address)
      {
      //Read the 4 bytes from the eeprom memory.
      long four = EEPROM.read(address);
      long three = EEPROM.read(address + 1);
      long two = EEPROM.read(address + 2);
      long one = EEPROM.read(address + 3);

      //Return the recomposed long by using bitshift.
      return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
      }


void counter()
{
//Update count
pulses++;
}

#define OLED_RESET 4

Adafruit_SSD1306 display(OLED_RESET);  // image created in LCD assistant

const unsigned char PROGMEM Emblem [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x01, 0xE0, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x73, 0xC0, 0x03, 0xCE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0xC4, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x0B, 0x83, 0xF8, 0x1F, 0x81, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x32, 0x78, 0x00, 0x02, 0x1E, 0x4C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x70, 0xFC, 0x00, 0x00, 0x3E, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x06, 0x1F, 0x80, 0x00, 0x00, 0x01, 0xF8, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x0E, 0x18, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x05, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x7F, 0xBF, 0x8C, 0xC3, 0x38, 0x73, 0xF9, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x18, 0x30, 0xCD, 0xC3, 0x28, 0xB3, 0x19, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x18, 0x7F, 0x19, 0x86, 0x69, 0xB7, 0xF3, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x38, 0x66, 0x19, 0x8E, 0x6B, 0x66, 0x03, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x30, 0xC7, 0x38, 0xF0, 0xE6, 0xEC, 0x03, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x08, 0x38, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x0F, 0x32, 0x00, 0x00, 0x00, 0x00, 0x9D, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x03, 0x83, 0x80, 0x00, 0x00, 0x01, 0x83, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x03, 0x90, 0x00, 0x00, 0x0B, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0xF9, 0x8C, 0x00, 0x00, 0x71, 0x9E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x3E, 0x0F, 0x70, 0x0E, 0xF1, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x07, 0x1E, 0x78, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0xFC, 0x06, 0x60, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x7E, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xF8, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void setup()   {                
  
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.drawBitmap(32, 0, Emblem, 128, 64, WHITE);
  display.setTextColor(WHITE);
  display.setTextSize(1.0);
  display.setCursor(0,0);
  display.print("");
  display.display();  
  
  // speedopin setup 
             
  pinMode(dir, OUTPUT);
  pinMode(stp, OUTPUT);
  pinMode(encoderPin, INPUT);
  pinMode(stopinput, INPUT);

//Stepper motor initial full sweep code

  digitalWrite(dir, HIGH); 
  for (int i = 0; i < 3718; i++){
    digitalWrite(stp, HIGH);
    delayMicroseconds(100);
    digitalWrite(stp, LOW);
    delayMicroseconds(100);
  }
 
  delay(500);
  
  digitalWrite(dir, LOW); 
  for (int i = 0; i < 3718; i++){
    digitalWrite(stp, HIGH);
    delayMicroseconds(100);
    digitalWrite(stp, LOW);
    delayMicroseconds(100);
  }
  
  // Initialize
  
pulses = 0;
rpm = 0;
timeold = 0;
display.clearDisplay();
mileage = (EEPROMReadlong(0)); 

// End of set up functions only runs once on powerup
 
}

void loop() {

 long address=0;

 if (millis() - timeold>=1000){
 detachInterrupt(0);
  display.display();
  display.clearDisplay();
  display.setTextSize(2.5);
  display.setCursor(32,15);

  Serial.print("Pulses per second: "); 
  Serial.println(pulses);
  
rpm = 60*pulses/pulsesperturn;

  Serial.print("rpm: "); 
  Serial.println(rpm);
  
vehicleSpeed = (60*1.816*(rpm/10.44))/1609; //calculate vehicle speed in mph. 1 mile = 1609 meters. wheel circumference = 1.816m. Drive Ratio = 10.44.

  Serial.print("mph: "); 
  Serial.println(vehicleSpeed);

if (mileageincrement >= 1 && pulses == 0)  // && digitalRead(stopinput) == HIGH  if required as input to initiate eeprom write

{

 EEPROMWritelong(address, newmileage);
      address+=4;

}

mileagenew += (vehicleSpeed/3600.0);
//newmileage = 89941;    // sets original mileage if line below // and code if statement run to reset EEPROM.
newmileage = mileage + mileagenew;
mileageincrement = newmileage - (EEPROMReadlong(0));

  Serial.print("Mileagenew: "); 
  Serial.println(mileagenew);
  Serial.print("Mileage: ");
  Serial.println(EEPROMReadlong(0));
  Serial.print("Mileageincrement: "); 
  Serial.println(mileageincrement);
   
  display.setCursor(26,0);
  display.setTextSize(1);
  display.print("mph");
  display.setTextSize(2);
  display.setCursor(0,0);
  display.print(vehicleSpeed);
  display.setTextSize(2.5);
  display.setCursor(25,15);
  display.print(newmileage);
  display.setTextSize(1);
  display.setCursor(90,20);
  display.print("miles");
  display.setTextSize(0.3);
  display.setCursor(103,0);
  display.print("trip");
  display.setTextSize(1);
  display.setCursor(65,4);
  display.print(mileagenew);

// Speed sensor to stepper motor control code

speedoreading = map(rpm, 0, 10000, 0, 2500); // the instantaneous speed

  Serial.print("Speedoreading: "); 
  Serial.println(speedoreading);

speedoreadingvalue = abs(speedoreadingold-speedoreading);

  Serial.print("Speedoreadingvalue: "); 
  Serial.println(speedoreadingvalue);

if (speedoreading > speedoreadingold-1)
{
  digitalWrite(dir, HIGH);
for (int i = 0; i < speedoreadingvalue; i++){
    digitalWrite(stp, HIGH);
    delayMicroseconds(100);
    digitalWrite(stp, LOW);
    delayMicroseconds(100);
}
}
else if (speedoreading < speedoreadingold+1)
{
  digitalWrite(dir, LOW); 
for (int i = 0; i < speedoreadingvalue; i++){
    digitalWrite(stp, HIGH);
    delayMicroseconds(100);
    digitalWrite(stp, LOW);
    delayMicroseconds(100);
}
}

pulses=0;
timeold = millis();
attachInterrupt(0, counter, RISING);  
}

speedoreadingold=speedoreading;
}






 











    
 



