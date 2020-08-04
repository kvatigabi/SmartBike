#include <NMEAGPS.h>
#include <SdFat.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

NMEAGPS gps;
gps_fix fix;
SdFat sd;
File file;
//GLOBAL VARS
unsigned long currentT;
volatile bool flagISR; //ISR flag
volatile unsigned long captured;//time when ISR was called
float tireL = 2 * 3.14 * 26; //2*pi*tire radiuc. all in centimeters
float kmh;
bool isFileCreated = false;//this will make sure that we check at least one time the file exists

void setup() {
#define CS_PIN 53
#define HALL_PIN 2
  pinMode(CS_PIN, OUTPUT);
  pinMode(HALL_PIN, INPUT);

  lcd.init();
  lcd.backlight();
  attachInterrupt(digitalPinToInterrupt(HALL_PIN), rpmISR, RISING);
  Serial.begin(9600);
  Serial1.begin(9600); //serial for gps board

  Serial.println(F("Init SD"));
  if (!sd.begin(CS_PIN)) {
    Serial.println(F("Init failed"));
    lcd.setCursor(0, 1);
    lcd.print(F("SD ERR"));
  }
  //  do {
  //    gps.available(Serial1);
  //    fix = gps.read();
  //    Serial.println("do..while loop");
  //  } while (!fix.valid.date);
  //
  //  char fileName[15];
  //  sprintf(fileName, "%d-%d-%d.csv", fix.dateTime.date, fix.dateTime.month, fix.dateTime.year);
  //  CreateFilename(fileName);
}

void loop() {
  currentT = millis();

  while (gps.available(Serial1)) {
    fix = gps.read();
    if (fix.valid.date && !isFileCreated) {
      char fileName[15];
      sprintf(fileName, "%d-%d-%d.csv", fix.dateTime.date, fix.dateTime.month, fix.dateTime.year);
      if (!sd.exists(fileName)) {
        Serial.println("FileName doesnt exist. Creating");
        CreateFilename(fileName);
        isFileCreated = true;
      }
    }
    SDwrite();
  }
  GetPrintTime();
  GetPrintPdop();
  GetPrintKmh();
}

void SDwrite() {
  if (fix.valid.location) {
    file.print(fix.dateTime.hours);
    file.print(F(":"));
    file.print(fix.dateTime.minutes);
    file.print(F(":"));
    file.print(fix.dateTime.seconds);
    file.print(F(","));
    file.print(fix.latitudeL());
    file.print(F(","));
    file.print(fix.longitudeL());
    file.print(F(","));
    file.println(fix.altitude());
    file.flush();
    //TODO: check if fix.altitude is valid and then write it else will print 0
  }
}
void GetPrintTime() {
  if (fix.valid.time) {
    lcd.setCursor(8, 1);
    lcd.print(fix.dateTime.hours);
    lcd.print(":");
    lcd.print(fix.dateTime.minutes);
  }
}
void GetPrintPdop() {
  if (fix.valid.pdop) {
    lcd.setCursor(12, 0);
    lcd.print(fix.pdop);
  }
}
void GetPrintKmh() { //calculate kmh and print it
  if (flagISR) {
    static float diff;
    diff = captured - diff;
    float rpm = (60000) / diff;
    float cmPerM = rpm * tireL; //cm per minute
    float kmPerM = cmPerM / 100000; //km per minute
    kmh = kmPerM * 60; //kmph per hour
    diff = currentT;
    flagISR = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(kmh);
    lcd.setCursor(7, 0);
    lcd.print("Kmh");
  }
}
void rpmISR() {
  captured = currentT;
  flagISR = true;
}

void CreateFilename(char fileName[15]) {
  //  if (!sd.exists(fileName)) { //if file doesnt exist
  //    Serial.println(F("fileName doesnt exists. creating"));
  file = sd.open(fileName, FILE_WRITE);//create/open it
  if (file) { //if it opened corerectly write this below
    file.print(F("Time"));
    file.print(F(","));
    file.print(F("Lat"));
    file.print(F(","));
    file.print(F("Long"));
    file.print(F(","));
    file.println(F("Alt(M)"));
    file.flush();
    Serial.println("FileName opened and written");
  }
  else {
    Serial.println(F("could not create/open file"));
}

//  else {//if it exists just open it lol
//    file = sd.open(fileName, FILE_WRITE);
//    if (!file) {
//      Serial.println(F("file exist but couldnt open"));
//    }
//  }
}
