#include <parsingdata.h>
#include <gyro.h>
GYRO hdt(ALL);
#define trigger 2
int flagBtn =0;
int c;
String data;
bool parsing = false;
char f_heading;
float lat;

void setup() {
  Serial.begin(4800);
  pinMode(trigger, INPUT);
  attachInterrupt(digitalPinToInterrupt(trigger), state, RISING);
  Serial.println("start");
}

void state(){
flagBtn = 1;
//delay(10);
}

void loop() 
{
  if  (Serial.available()>0){
    if (hdt.decode(Serial.read())){
      f_heading =hdt.term(1);
    }
   }
   if (flagBtn == 1){
      Serial.println(hdt.term(1));
      flagBtn = 0;
    }
}
