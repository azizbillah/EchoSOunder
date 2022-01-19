#include <SD.h>
//#include <parsingdata.h>
#define SerialEcho Serial1
#define SerialThirdparty Serial2
#define SerialGYRO Serial3
#define SerialPC Serial
#define triggerGPS 10
#define triggerGYRO 12
#define cs 53
#define state_Anotasi_pin 7

//PARSINGDATA nmea(ALL);
String dt[20], dtt[16];
int i;
boolean parsing = false, parsingGPS = false, parsing_gyro = false;
boolean flag_save_dbt = false, flag_save_xdr = false;
int flag_Anotasi = 0;

#define MAX_OUT_CHARS 250  //max nbr of characters to be sent on any one serial command
char   buffer[MAX_OUT_CHARS + 4];

const byte buff_len = 120;
char CRCbuffer[buff_len];
volatile int flagBtn = 0;
unsigned long ping_id = 0;
String dataGPS, dataEcho, dataGYRO;
String cmd, cmd1, cmd_hdt;
char Lat[15], Long[15], Time[15], Fix_qty[15] = "0";

char Depth_f[15], Depth_M[15], Depth_F[15];
char Temp_H[15], Temp_L[15], Temp_C[15];
char Depth_M_H[15], Depth_M_L[15];
float float_Depth_M_H, float_Depth_M_L;
int minLength = 0;
int num = 0, num_xdr = 0;
String s_heading;
float f_heading = 0;
int i_Fix_qty;
float depth_cc, depth, depth_H, depth_L;
float temp_dbt, temp_xdr;

void setup() {
  SerialPC.begin(4800);
  SerialEcho.begin(38400);
  SerialGYRO.begin(4800);
  SerialThirdparty.begin(38400);
  pinMode(triggerGPS, OUTPUT);
  pinMode(triggerGYRO, OUTPUT);
  pinMode(state_Anotasi_pin, INPUT_PULLUP);
  digitalWrite(triggerGPS, HIGH);
  digitalWrite(triggerGYRO, HIGH);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  dataEcho.reserve(200);
  unsigned long interrupt_time = 0;
}

void loop() {
    if(parsing){
      digitalWrite(triggerGYRO, LOW);
      digitalWrite(triggerGYRO, HIGH);
      s_heading = WaitForInputGyro();
      f_heading = s_heading.toFloat();
      flag_Anotasi = digitalRead(state_Anotasi_pin);
      parsingData();
      dataEcho="";
      parsing=false;
      if (flag_Anotasi==HIGH){ //anotasi internal
        digitalWrite(13,LOW);
        ping_id++;
      }
      else {                  //anotasi thirdparty
        digitalWrite(13,HIGH);
        while (SerialThirdparty.available()) {
          char inChar = (char)SerialThirdparty.read();
          SerialEcho.print(inChar);
          delay(10);
          //SerialEcho.print('\n');
        }
        ping_id = 0;
      }
    }
}

void serialEvent1() {
  while (SerialEcho.available()) {
    char inChar = (char)SerialEcho.read();
    dataEcho += inChar;
    if (inChar == '\n') {
      parsing = true;
    }
  }
}

void parsingData() {
  int j = 0;
  int k = 0;
  dt[j] = "";
  int minLength;
  if (dataEcho.length() > 70) {
    minLength = 20;
  }
  else                      {
    minLength = 0;
  }
  for (k = 1; k < dataEcho.length() - minLength ; k++) {
    if ((dataEcho[k] == '$') || (dataEcho[k] == ',' ) || (dataEcho[k] == '*' ) ) {
      j++;
      dt[j] = "";
    }
    else {
      dt[j] = dt[j] + dataEcho[k];
    }
  }

  if (dt[0] == "SDDPT") {
    depth_cc = dt[2].toFloat();
  }
  if (dt[0] == "SDMTW") {
    temp_dbt = dt[1].toFloat();

  }

  if (dt[0] == "SDDBT") {
    depth = dt[3].toFloat() + depth_cc;
    cmd = "$DATA1," + String(ping_id) + "," + String(f_heading) + "," + String(depth) + "," + "0" + "," + String(temp_dbt) + "*";
    cmd1 = "$AN," + String(ping_id) + "*";
    flag_save_dbt = true;
    outputMsg(cmd);
    delay(50);
    outputMsg(cmd1);
    memset(CRCbuffer, 0, sizeof(CRCbuffer)); //clear data
  }
  else if (dt[0] == "SDXDR") {
    depth_H = dt[2].toFloat() + depth_cc;
    depth_L = dt[6].toFloat() + depth_cc;
    temp_xdr = dt[10].toFloat();
    cmd = "$DATA1," + String(ping_id) + "," + String(f_heading) + "," + String(depth_H) + "," + String(depth_L) + "," + String(temp_xdr) + "*";
    cmd1 = "$AN," + String(ping_id) + "*";;
    flag_save_xdr = true;
    outputMsg(cmd);
    delay(50);
    outputMsg(cmd1);
    //delay(50);
    memset(CRCbuffer, 0, sizeof(CRCbuffer)); //clear data
  }
}

String WaitForInputGyro() {
  while (!SerialGYRO.available()) {}
  return SerialGYRO.readStringUntil('\n');
}

//void dataGyro()
//{ 
//  while(SerialGYRO.available())
//  {
//    char hdt = (char)SerialGYRO.read();
//    SerialPC.print(hdt);
//  }
//
//  
//}

void outputMsg(String msg) {
  msg.toCharArray(CRCbuffer, sizeof(CRCbuffer));
  byte crc = convertToCRC(CRCbuffer);
 // SerialPC.print(msg);
  SerialEcho.print(msg);
  if (crc < 20) {
  //  SerialPC.print("0");
    SerialEcho.print("0");
  }
 // SerialPC.println(crc, HEX);
  SerialEcho.println(crc, HEX);
}

byte convertToCRC(char *buff) {
  char c;
  byte i;
  byte start_with = 0;
  byte end_with = 0;
  byte crc = 0;

  for (i = 0; i < buff_len; i++) {
    c = buff[i];
    if (c == '$') {
      start_with = i;
    }
    if (c == '*') {
      end_with = i;
    }
  }
  if (end_with > start_with) {
    for (i = start_with + 1; i < end_with; i++) {
      crc = crc ^ buff[i] ;
    }
  }
  else {
    SerialPC.println("CRC ERROR");
    return 0;
  }
  return crc;
}
