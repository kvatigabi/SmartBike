#include <NMEAGPS.h>
#include <SdFat.h>

NMEAGPS gps;
gps_fix fix;
SdFat sd;
File file;

#define CS_PIN 10
#define buz 7

void setup() {
  delay(2000);
  pinMode(10, OUTPUT);
  pinMode(buz, OUTPUT);

  Serial.begin(9600);
  Serial1.begin(9600);

  Serial.println(F("Init SD"));
  if (!sd.begin(CS_PIN)) {
    Serial.println(F("Init failed"));
    while (true) {
      Errsound();
    }
  }
  successSound();

  do {
    gps.available(Serial1);
    fix = gps.read();
    Serial.println("do..while loop");
  } while (!fix.valid.date);

  char fileName[15];
  sprintf(fileName, "%d-%d-%d.csv", fix.dateTime.date, fix.dateTime.month, fix.dateTime.year);
  if (!sd.exists(fileName)) { //if file doesnt exist
    Serial.println(F("fileName doesnt exists. creating"));
    file = sd.open(fileName, FILE_WRITE);//create it
    if (file) { //if it opened corerectly write this below
      file.print(F("Time"));
      file.print(F(","));
      file.print(F("Lat"));
      file.print(F(","));
      file.print(F("Long"));
      file.print(F(","));
      file.println(F("Alt(M)"));
      file.flush();
    }
    else {
      Serial.println(F("file err in creating"));
      while (true) {
        Errsound();
      }
    }
  }
  else {//if it exists just open it lol
    file = sd.open(fileName, FILE_WRITE);
    if (!file) {//if its error dont continue and leave err message
      Serial.println(F("file err in last else"));
      while (true) {
        Errsound();
      }
    }
  }

}

void loop() {
  while (gps.available(Serial1)) {
    fix = gps.read();
    SDwrite();
  }


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
    //    file.print(",");
    //    file.println(fix.altitude());

  }
  if (fix.valid.altitude) {
    file.print(F(","));
    file.print(F(","));
    file.println(fix.altitude());
    file.flush();
  }
}

void Errsound() {
  /* bool change = false;
    if (now - prev >= intervalBuz) {
     // save the last time you blinked the LED
     prev = now;


     if (!change) {
       change = true;
       noTone(buz);
       tone(buz, 2000);
     }
     else {
       change = false;
       noTone(buz);
       tone(buz, 3000);
     }
    }
  */
  tone(buz, 2000);
  delay(300);
  noTone(buz);
  tone(buz, 3000);
  delay(300);
  noTone(buz);
}
void successSound() {
  tone(buz, 2000);
  delay(300);
  noTone(buz);
  delay(100);
  tone(buz, 2000);
  delay(300);
  noTone(buz);

}
