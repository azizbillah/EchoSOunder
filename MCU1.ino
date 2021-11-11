#include <SD.h>
#include <parsingdata.h>
#define SerialEcho Serial1
#define SerialGPS Serial2
#define SerialGYRO Serial3
#define SerialPC Serial
#define triggerGPS 10
#define triggerGYRO 12
#define cs 53
#define state_SD 7


PARSINGDATA nmea(ALL);
String dt[10],dtt[10];
int i;
boolean parsing=false, parsingGPS=false, parsing_gyro=false;
boolean flag_save_dbt=false, flag_save_xdr=false;
int flag_sdcard = 0;

#define MAX_OUT_CHARS 250  //max nbr of characters to be sent on any one serial command
char   buffer[MAX_OUT_CHARS + 4];

const byte buff_len = 90;
char CRCbuffer[buff_len];
volatile int flagBtn = 0;
int ping_id = 0;
String dataGPS,dataEcho,dataGYRO;
String cmd,cmd1,cmd_hdt;
char Lat[15], Long[15], Time[15], Fix_qty[15]="0";

char Depth_f[15], Depth_M[15], Depth_F[15];
char Temp_H[15], Temp_L[15], Temp_C[15];
char Depth_M_H[15], Depth_M_L[15];
float float_Depth_M_H, float_Depth_M_L;
int minLength = 0;
int num=0,num_xdr=0;
String s_heading;
float f_heading;

void setup(){
  SerialPC.begin(4800);
  SerialEcho.begin(4800);
  SerialGPS.begin(4800);
  SerialGYRO.begin(4800);
  pinMode(triggerGPS, OUTPUT);
  pinMode(triggerGYRO, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(state_SD, INPUT_PULLUP);
  digitalWrite(triggerGPS, HIGH);
  digitalWrite(triggerGYRO, HIGH);
  
  SerialPC.print("Initializing SD card...");
//  if (!SD.begin(cs)) {
//    SerialPC.println("Card failed, or not present");
//    return;
//  }
  SerialPC.println("card initialized.");
//  while(!SerialEcho.available()){} //delay menunggu karakter pembuka dari echo sounder sekitar 2 menit
  SerialPC.println("Start");
  dataEcho.reserve(200);
  unsigned long interrupt_time = 0;
//  dataGYRO.reserve(200);
}

void loop(){
  
  if(parsing){
    digitalWrite(triggerGPS, LOW);
    digitalWrite(triggerGPS, HIGH);
    dataGPS = WaitForInput();
    parsingdataGPS();
    if (f_heading <= 0){
      digitalWrite(triggerGYRO, LOW);
      digitalWrite(triggerGYRO, HIGH);
      s_heading = WaitForInputGyro();
      f_heading = s_heading.toFloat();
    }
    String HDT = "$HEHDT," + String(f_heading) + ",T*";
    outputMsg(HDT);
    memset(CRCbuffer,0,sizeof(CRCbuffer));
    SerialPC.print(dataEcho);
    SerialEcho.print(dataEcho);
    parsingData();
    dataEcho="";
    savedata();
    parsing=false;
  }
  savedata();

}

void serialEvent1() {
  while (SerialEcho.available()) {
    char inChar = (char)SerialEcho.read();
    dataEcho += inChar;
    if (inChar == '\n') {parsing = true;}
  }
}

void parsingData(){
  int j=0;
  int k=0;
  dt[j]="";
  if (dataEcho.length()>60) {minLength = 50;}
  else                      {minLength = 0;}
//  SerialPC.println(dataEcho.length());
  for(k=1; k < dataEcho.length()-minLength; k++){
    if ((dataEcho[k] == '$') || (dataEcho[k] == ',')){
      j++;
      dt[j]=""; 
    } 
    else { dt[j] = dt[j] + dataEcho[k];}
  }
  ping_id++;
  
  if (dt[0]=="SDDBT"){
    cmd = "$DATA1," +  String(Time) + "," + String(Lat) + "," + String(Long) + "," + String(ping_id) + "," + String(Fix_qty) + "," + String(f_heading) + "," + String(dt[3]) + "*";
    flag_save_dbt = true;
  }
  else if (dt[0]=="SDXDR"){
    cmd = "$DATA1," + String(Time) + "," + String(Lat) + "," + String(Long) + "," + String(ping_id) + "," + String(Fix_qty) + "," + String(f_heading) + "," + String(dt[2]) + "," + String(dt[6]) + "*";
    flag_save_xdr = true;
  }
  outputMsg(cmd);
  memset(CRCbuffer,0,sizeof(CRCbuffer)); //clear data
}

void parsingdataGPS(){
  int j=0;
  dtt[j]="";
  for(i=0;i < dataGPS.length();i++){
    if ((dataGPS[i] == ',')){
      j++;
      dtt[j]=""; 
    } 
    else {dtt[j] = dtt[j] + dataGPS[i];}
  }
  dtt[4].trim();
  dtt[0].toCharArray(Time,15);
  dtt[1].toCharArray(Lat,15);
  dtt[2].toCharArray(Long,15);
  dtt[3].toCharArray(Fix_qty,15);
  f_heading = dtt[4].toFloat();
}

String WaitForInput(){
  while(!SerialGPS.available()){}
  return SerialGPS.readStringUntil('\n');
}

String WaitForInputGyro(){
  while(!SerialGYRO.available()){}
  return SerialGYRO.readStringUntil('\n');
}

void outputMsg(String msg) {
  msg.toCharArray(CRCbuffer,sizeof(CRCbuffer));
  byte crc = convertToCRC(CRCbuffer);
  SerialPC.print(msg);
  SerialEcho.print(msg);
  if (crc < 16) {SerialPC.print("0");
  SerialEcho.print("0");}
  SerialPC.println(crc,HEX);
  SerialEcho.println(crc,HEX);
}

byte convertToCRC(char *buff) {
  char c;
  byte i;
  byte start_with = 0;
  byte end_with = 0;
  byte crc = 0;

  for (i = 0; i < buff_len; i++) {
    c = buff[i];
    if(c == '$') {start_with = i;}
    if(c == '*') {end_with = i;}      
  }
  if (end_with > start_with){
    for (i = start_with+1; i < end_with; i++){ crc = crc ^ buff[i] ;}
  }
  else {
    SerialPC.println("CRC ERROR");
    return 0;
  }
  return crc;
}

void savedata(){
  flag_sdcard = digitalRead(state_SD);
  Serial.println(flag_sdcard);
  if (flag_sdcard==HIGH){
    digitalWrite(13, HIGH);
    if (flag_save_dbt){
      if (num == 0){//judul
        cmd1 = "NO, Time, Latitude, Longitude, DEPTH, FIX QUALITY, PING ID";
        num_xdr = 0;
      }
      else {
        cmd1 = String(num) + ", " + String(Time) + ", " + String(Lat) + ", " + String(Long) + ", " + String(dt[3]) + ", " + String(Fix_qty) + ", " + String(ping_id);
      }
      logging(cmd1);
      num++;
      flag_save_dbt = false;
    }
    if (flag_save_xdr){
      digitalWrite(13, HIGH);
      if (num_xdr == 0){
        cmd1 = "NO, Time, Latitude, Longitude, DEPTH1, DEPTH2, FIX QUALITY, PING ID";
        num = 0;
      }
      else{
        cmd1 = String(num_xdr) + ", " + String(Time) + ", " + String(Lat) + ", " + String(Long) + ", " + String(dt[2]) + ", " + String(dt[6]) + ", " + String(Fix_qty) + ", " + String(ping_id);
      }
      logging(cmd1);
      cmd1="";
      num_xdr++;
      flag_save_xdr = false;
    }
  }
  else  {digitalWrite(13, LOW);}
}

void logging(String a){
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println(a);
    dataFile.close();
    SerialPC.println(a);
  }
  else {
    SerialPC.println("error opening datalog.csv");
    SD.begin(cs);
  }
}
