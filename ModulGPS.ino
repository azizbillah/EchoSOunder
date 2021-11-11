#include <parsingdata.h>
#define trigger 2
volatile int flagBtn = 0;
PARSINGDATA nmea(HEHDT);

float lat, lon;
int Time;
float f_Time;
char fix_qty;
char sz[32];
float f_heading;

int flag = 1;

void setup() {
  Serial.begin(4800);
  pinMode(trigger, INPUT);
  attachInterrupt(digitalPinToInterrupt(trigger), state, RISING);
  flagBtn = 0;
  Serial.println("start");
//  dataGyro.reserve(200);
}

void state(){
  flagBtn = 1;
}

void loop() {
  while (Serial.available()>0) {
    if (nmea.decode(Serial.read())){
      f_heading = nmea.hehdt_heading();

      lon = nmea.gpgga_longitude();
      lat = nmea.gpgga_latitude();
      fix_qty = nmea.gpgga_status();
      byte Hour, Minute, Second;
      crack_time(&Hour, &Minute, &Second);   
      sprintf(sz, "%02d/%02d/%02d",Hour, Minute, Second);
//      Serial.print(sz);Serial.print(",");
//      Serial.print(lat,5);Serial.print(",");
//      Serial.print(lon,5);Serial.print(",");
//      Serial.print(fix_qty);Serial.print(",");
//      Serial.println(f_heading,3);
    }
  }
  if (flagBtn == 1){
    Serial.print(sz);Serial.print(",");
    Serial.print(lat,5);Serial.print(",");
    Serial.print(lon,5);Serial.print(",");
    Serial.print(fix_qty);Serial.print(",");
    Serial.println(f_heading,3);
    delay(100);
    flagBtn = 0;
  }
}

void crack_time(byte *Hour, byte *Minute, byte *Second)
{
  unsigned long Time;
  Time = nmea.gpgga_utc();
  if (Hour) *Hour = Time / 10000;
  if (Minute) *Minute = (Time / 100) % 100;
  if (Second) *Second = (Time) % 100;
}
