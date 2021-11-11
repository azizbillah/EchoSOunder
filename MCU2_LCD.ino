#include "U8glib.h"
#include <echo.h>
#define SerialData Serial3 // RX
#define SerialPC Serial2   // RX TX (Serial2
#define buttonMENU 9
#define buttonUP  5
#define buttonDOWN 3
#define buttonENTER 7
#define buttonSD 21
int state_SD =A6;

ECHO nmea(DATA1);
U8GLIB_ST7920_128X64_1X u8g(52, 51, 53);  // SPI Com: SCK = en = 18, MOSI = rw = 16, CS = di = 17
const byte buttonSTN = 18;
const byte buttonDBT = 19;
const byte buttonXDR = 20;
String cmd;

int i;
int n = 0;
String data,data1;
float sow = 0, draft = 0;

char char_pps[4]="0000";
char char_sow[10]="00.00", char_draft[10]="00.00";
char char_sow_lcd[10]="00.00", char_draft_lcd[10]="00.00";

char char_time[10], char_lat[10]="0.00";
char char_long[10]="0.00", char_ping[10]="0.00";
char char_depth[10]="0.00", char_depth_l[10]="0.00";
char char_sat_fix[10]="0", char_heading[10];
char hh[2],mm[2],ss[10];

String s_time,s_lat, s_long, s_ping, s_depth, s_depth_l, s_sat;
String s_heading,s_sd;
int slide=0;
int dfilter=0,sfilter=0,dblank=0,interval = 10;
char char_dfilter[2]="0", char_sfilter[2]="0", char_dblank[2]="0",char_interval[4]="10";

int ENTER = 0, UP = 0, DOWN = 0, MENU = 0;
int Status_SD=0, flag_SD = 0;
const byte buff_len = 90;
char CRCbuffer[buff_len];
volatile int flagBtn = 0;
volatile int flagMenu = 0;
int counter = 0;
int flag_edit_menu = 0;
int flag_edit_input = 0;
int flag_edit_mode = 0;
int int_sat=0;

int Time = 0.0;
boolean flag_editSOW = false, flag_editDraft = false, flag_editPps = false;
boolean parsing = false;
String dt[10];
int flag_draw, Cursor=0, Cursor1 = 0;

#define LIST_SLIDE1 12
#define LIST_SLIDE2 12
#define LIST_MENU 6
const char *label_slide1[LIST_SLIDE1] = { "TIME UTC", "LAT", "LONG", "PING ID", "SAT FIX", "HEADING",
                                char_time, char_lat, char_long, char_ping, char_sat_fix, char_heading};
const char *label_slide2[LIST_SLIDE2] = { "DRAFT", "SOW", "DFILTER", "SFILTER", "DBLANK", "INTERVAL",
                                char_draft_lcd, char_sow_lcd, char_dfilter, char_sfilter, char_dblank, char_interval};
const char *label_menu_edit[LIST_MENU] = { "Edit Nilai Draft", "Edit Nilai SOW", "Edit Nilai DFILTER",
                                "Edit Nilai SFILTER", "Edit Nilai DBLANK", "Edit Interval"};

void draw(void) {
  if (slide == 2){//SLIDE 2
    u8g.setFont(u8g_font_04b_03);
    for(int i = 0; i < (LIST_SLIDE2/2); i++ ) {
      u8g.drawStr(5, (i+1)*7, label_slide2[i]);
      u8g.drawStr(60, (i+1)*7, label_slide2[i+6]);
      u8g.drawStr(40, (i+1)*7,  " :");
    }
    u8g.drawStr(90, 6, "m");
    u8g.drawStr(90, 12, "m/s");
    u8g.drawStr(90, 36, "cm");//dblank
    u8g.drawStr(90, 42, "hz");//interval
    if (flag_SD == 1){u8g.drawStr( 5, 64, "SAVE DATA");}
    else            {u8g.drawStr( 5, 64, "NO SAVE DATA");}
  }
  else if (slide == 1){//SLIDE 1
    String a = gps_fix_status(int_sat);
    s_time.toCharArray(char_time, 10);
    s_ping.toCharArray(char_ping, 10);
    s_lat.toCharArray(char_lat, 10);
    s_long.toCharArray(char_long, 10);
    s_depth.toCharArray(char_depth, 10);
    s_depth_l.toCharArray(char_depth_l, 10);
    a.toCharArray(char_sat_fix, 10);
    s_heading.toCharArray(char_heading, 10);
    u8g.setFont(u8g_font_04b_03);
    for(int i = 0; i < (LIST_SLIDE1/2); i++ ) {
      u8g.drawStr(5, (i+1)*8, label_slide1[i]);
      u8g.drawStr(50, (i+1)*8, label_slide1[i+6]);
      u8g.drawStr(40, (i+1)*8,  " :");
    }
    u8g.drawStr(90, 16, "deg");
    u8g.drawStr(90, 24, "deg");
    u8g.drawStr(90, 48, "deg");
    u8g.drawStr(90, 56, "m");
    if (flag_draw == 3){//display data xdr  
      u8g.drawStr( 110, 56, "XDR");
      u8g.drawStr( 5, 56, "DEPTH_H");  u8g.drawStr(40, 56, " :"); u8g.drawStr( 50, 56, char_depth);
      u8g.drawStr( 5, 64, "DEPTH_L");  u8g.drawStr(40, 64, " :"); u8g.drawStr( 50, 64, char_depth_l); u8g.drawStr( 90, 64, "m");
    }
    else if (flag_draw == 1) {//display data stn
      u8g.drawStr( 110, 56, "STN");
      u8g.drawStr( 5, 56, "DA"); u8g.drawStr(40, 56, " :"); u8g.drawStr( 50, 56, char_depth);
      u8g.drawStr( 5, 64, "DB"); u8g.drawStr(40, 64, " :"); u8g.drawStr( 50, 64, char_depth_l); u8g.drawStr(90, 64, "m");
    }
  
    else if (flag_draw == 2) {//display data dbt
      u8g.drawStr( 110, 56, "DBT");
      u8g.drawStr( 5, 56, "DEPTH"); u8g.drawStr(40, 56, " :");u8g.drawStr( 50, 56, char_depth); 
    }
    else if (flag_draw == 0) {//display data dbt
      u8g.drawStr( 110, 56, "***");
      u8g.drawStr( 5, 56, "DEPTH"); u8g.drawStr(40, 56, " :");
    }
  }
  else if (slide == 0){
    u8g.setFont(u8g_font_ncenB12);
    u8g.drawStr( 0, 20, "SBES AH15 DF");
    u8g.setFont(u8g_font_ncenR12);
    u8g.drawStr( 5, 40, "Dual Frequency");
    u8g.setFont(u8g_font_4x6);
    u8g.drawStr( 5, 54, "Geosains Enjiniring Nusantara");
  }
}

void draw1(void){
  int y = (flag_edit_menu+1)*8;
  u8g.setFont(u8g_font_04b_03);
  u8g.drawStr( 0, y, ">");
  for (int i=0;i<LIST_MENU;i++){
    u8g.drawStr( 5, (i+1)*8, label_menu_edit[i]);
  }
}

void draw2(void){
  u8g.setFont(u8g_font_helvB08);
  if(flag_edit_menu == 0){//draft
    u8g.drawStr( 0, 20, "Nilai DRAFT : ");
    float_to_lcd(draft);
    if (Cursor == 0){    u8g.drawStr( 80, 10, "V");}
    else if (Cursor == 1){    u8g.drawStr( 85, 10, "V");}
    else if (Cursor == 2){    u8g.drawStr( 90, 10, "V");}
    else if (Cursor == 3){    u8g.drawStr( 95, 10, "V");}
    else if (Cursor == 4){    u8g.drawStr( 104, 10, "V");}
    else if (Cursor == 5){    u8g.drawStr( 110, 10, "V");}
  }
  else if(flag_edit_menu == 1) {//sow
    u8g.drawStr( 0, 20, "Nilai SOW : ");
    float_to_lcd(sow);
    if (Cursor == 0){    u8g.drawStr( 80, 10, "V");}
    else if (Cursor == 1){    u8g.drawStr( 85, 10, "V");}
    else if (Cursor == 2){    u8g.drawStr( 90, 10, "V");}
    else if (Cursor == 3){    u8g.drawStr( 95, 10, "V");}
    else if (Cursor == 4){    u8g.drawStr( 104, 10, "V");}
    else if (Cursor == 5){    u8g.drawStr( 110, 10, "V");}
  }
  else if(flag_edit_menu == 2) {//dfilter
    String str = String(dfilter);
    str.toCharArray(char_dfilter,2);
    u8g.drawStr( 0, 20, "Nilai Dfilter : ");
    u8g.drawStr( 90, 20, char_dfilter);
  }

  else if(flag_edit_menu == 3) {//sfilter
    String str = String(sfilter);
    str.toCharArray(char_sfilter,2);
    u8g.drawStr( 0, 20, "Nilai Sfilter : ");
    u8g.drawStr( 96, 20, char_sfilter);
  }
  else if(flag_edit_menu == 4) {//dblank
    String str = String(dblank);
    str.toCharArray(char_dblank,2);
    u8g.drawStr( 0, 20, "Nilai Dblank : ");
    u8g.drawStr( 90, 20, char_dblank);
  }
  else if(flag_edit_menu == 5) {//interval
    String str = String(interval);
    str.toCharArray(char_interval,3);
    u8g.drawStr( 0, 20, "Nilai Interval : ");
    u8g.drawStr( 90, 20, char_interval);
  }
}

void setup(){
  SerialPC.begin(4800);
  SerialData.begin(4800);
  Serial.begin(4800);
  pinMode(buttonMENU,INPUT);
  pinMode(buttonUP,INPUT);
  pinMode(buttonDOWN,INPUT);
  pinMode(buttonENTER,INPUT);
  pinMode(buttonSD,INPUT);
  pinMode(state_SD,OUTPUT);
  attachInterrupt(digitalPinToInterrupt(buttonSTN), stateSTN, RISING);
  attachInterrupt(digitalPinToInterrupt(buttonDBT), stateDBT, RISING);
  attachInterrupt(digitalPinToInterrupt(buttonXDR), stateXDR, RISING);
  attachInterrupt(digitalPinToInterrupt(buttonSD), state_sdcard, CHANGE);
  digitalWrite(state_SD,LOW);
  flagBtn = 0;
  if      ( u8g.getMode() == U8G_MODE_R3G3B2 )  {u8g.setColorIndex(255);}     // white
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ){u8g.setColorIndex(3);}         // max intensity
  else if ( u8g.getMode() == U8G_MODE_BW )      {u8g.setColorIndex(1);}         // pixel on
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {u8g.setHiColorByRGB(255,255,255);}
  flag_draw = 0;
}

void stateSTN()   {flagBtn = 1;}
void stateDBT()   {flagBtn = 2;}
void stateXDR()   {flagBtn = 3;}
void state_sdcard(){
  flag_SD++;
  if (flag_SD>1){flag_SD=0;}
  if (flag_SD==1){  analogWrite(state_SD,255);}
  else{analogWrite(state_SD,0);}
}

void loop(){
  menu:
  while(1){   
    while (SerialPC.available()) {
      char inChar = (char)SerialPC.read();
      if      (inChar == 'A') {flagBtn = 1;}
      else if (inChar == 'B') {flagBtn = 2;}
      else if (inChar == 'C') {flagBtn = 3;}
      else if (inChar == 'S') {sow = data.toDouble();
                               flag_edit_input = 1;
                               data="";}
      else if (inChar == 'D') {draft = data.toDouble();
                               flag_edit_input = 2;
                               data="";}
      else    {data.concat(inChar);}
    }
    while (SerialData.available()>0) {
      if (nmea.decode(SerialData.read())){
        s_time = nmea.term(1);
        s_lat = nmea.term(2);
        s_long = nmea.term(3);
        s_ping = nmea.term(4);
        s_sat = nmea.term(5);
        int_sat = s_sat.toInt();
        s_depth = nmea.term(7);
        s_depth_l = nmea.term(8);
        s_heading = nmea.term(6);
      }
    }
    u8g.firstPage();  
    do { draw();} 
    while(u8g.nextPage());
    initialButton();
    send_echo(flagBtn);
    edit_input(flag_edit_input);
//    Status_SD = digitalRead(buttonSD);
//    if (Status_SD == HIGH){flag_SD=1;}
//    else {flag_SD=0;}
    if (MENU == LOW){delay(500); Cursor = 0;
      goto menu1;
    }
    if (UP == LOW)  {slide++;delay(200);}
    if (DOWN == LOW){slide--;delay(200);}
    if  (slide <0) {slide=2;}
    if  (slide >2) {slide=0;}
  }
  
  menu1:
  while(1){
    u8g.firstPage();
    do {draw1();}
    while( u8g.nextPage() );
    initialButton();
    if (UP == LOW)   {flag_edit_menu--;if(flag_edit_menu<0){flag_edit_menu=5;}}
    if (DOWN == LOW) {flag_edit_menu++;if(flag_edit_menu>5){flag_edit_menu=0;}}
    delay(100);
    if (ENTER == LOW){
      Cursor1=0;
      delay(500);
      if      (flag_edit_menu==0)  {goto EditDraft;}
      else if (flag_edit_menu==1)  {goto EditSOW;}
      else if (flag_edit_menu==2)  {goto EditDFILTER;}
      else if (flag_edit_menu==3)  {goto EditSFILTER;}
      else if (flag_edit_menu==4)  {goto EditDBLANK;}
      else if (flag_edit_menu==5)  {goto EditINTERVAL;}
    }
  }

  EditDraft:
  while(1){
    u8g.firstPage();
      do {draw2();}
      while( u8g.nextPage() );
    initialButton();
    delay(10);
    if (MENU == LOW)  {
      Cursor++;
      if (Cursor == 6){Cursor = 0;}
    }
    if (UP == LOW)   {draft = edit_digitf_plus(draft, Cursor);}
    if (DOWN == LOW) { draft = edit_digitf_min(draft, Cursor); }
    delay(200);
    if (draft < 0){draft = 0;}
    if (ENTER == LOW){
      char buffer[100];
      float c_draft = draft*1000;
      dtostrf(draft,5,2,char_draft_lcd);
      dtostrf(c_draft,5,2,char_draft);
      sprintf(buffer,"$PAMTC,OPTION,SET,DOFFSET,%s,S*",char_draft);
      cmd = buffer;
      outputMsg(cmd);//      memset(CRCbuffer,0,sizeof(CRCbuffer)); //clear data
      delay(500);
      goto menu;
    }
  }
  
  EditSOW:
  while(1){
    u8g.firstPage();  
      do {draw2();}
      while( u8g.nextPage() );
    initialButton();
    delay(10);
    if (MENU == LOW)  {
      Cursor++;
      if (Cursor == 6){Cursor = 0;}
      delay(200);
    }
    if (UP == LOW)   {sow = edit_digitf_plus(sow, Cursor);}
    if (DOWN == LOW) {sow = edit_digitf_min(sow, Cursor);}
    delay(200);
    if (sow < 0){sow = 0;}
    if (ENTER == LOW){
      char buffer[100];
      float c_sow = sow*10;
      dtostrf(sow,5,2,char_sow_lcd);
      dtostrf(c_sow,5,2,char_sow);
      sprintf(buffer,"$PAMTC,OPTION,SET,SOSTW,%s,S*",char_sow);
      cmd = buffer;
      outputMsg(cmd);
      memset(CRCbuffer,0,sizeof(CRCbuffer)); //clear data
      delay(500);
      goto menu;
    }
  }

  EditDFILTER:
  while(1){
    u8g.firstPage();
      do {draw2();}
      while( u8g.nextPage() );
    initialButton();
    delay(10);
    if (UP == LOW)   {dfilter++;delay(200);}
    if (DOWN == LOW) {dfilter--;delay(200);}
    if (dfilter > 4){dfilter = 0;}
    if (dfilter < 0){dfilter = 4;}
    if (ENTER == LOW){
      char buffer[100];
      sprintf(buffer,"$PAMTC,OPTION,SET,DFILTER,%d,M*",dfilter);
      cmd = buffer;
      outputMsg(cmd);
      memset(CRCbuffer,0,sizeof(CRCbuffer)); //clear data
      delay(500);
      goto menu;
    }
    delay(200);
  }

  EditSFILTER:
  while(1){
    u8g.firstPage();
      do {draw2();}
      while( u8g.nextPage() );
    initialButton();
    delay(10);
    Cursor = 0;
    if (UP == LOW)   {sfilter = sfilter+2;if (sfilter > 8){sfilter = 0;}}
    if (DOWN == LOW) {sfilter = sfilter-2;if (sfilter < 0){sfilter = 8;}}
    delay(200);
    if (ENTER == LOW){
      char buffer[100];
      sprintf(buffer,"$PAMTC,OPTION,SET,SFILTER,%d,M*",sfilter);
      cmd = buffer;
      outputMsg(cmd);
      memset(CRCbuffer,0,sizeof(CRCbuffer)); //clear data
      delay(500);
      goto menu;
    }
    delay(200);
  }
  
  EditDBLANK:
  while(1){
    u8g.firstPage();
      do {draw2();}
      while( u8g.nextPage() );
    initialButton();
    delay(10);
    Cursor = 0;
    if (UP == LOW)   {dblank++; if (dblank > 50){dblank = 0;}}
    if (DOWN == LOW) {dblank--;if (dblank < 0){dblank = 50;}}
    delay(200);
    if (ENTER == LOW){
      int tenth = dblank*10;
      char buffer[100];
      sprintf(buffer,"$PAMTC,OPTION,SET,DBLANK,%d,M*",tenth);
      cmd = buffer;
      outputMsg(cmd);
      memset(CRCbuffer,0,sizeof(CRCbuffer)); //clear data
      delay(500);
      goto menu;
    }
    delay(200);
  }
  EditINTERVAL:
  while(1){
    u8g.firstPage();
      do {draw2();}
      while( u8g.nextPage() );
    initialButton();
    delay(10);
    Cursor = 0;
    if (UP == LOW)   {interval++; if (interval > 10){interval = 0;}}
    if (DOWN == LOW) {interval--;if (interval < 0){interval = 10;}}
    delay(200);
    if (ENTER == LOW){
      delay(500);
      goto menu;
    }
    delay(200);
  }
  
}

//ModeSinc:
//  while(1){
//    u8g.firstPage();
//      do {draw3();}
//      while( u8g.nextPage() );
//    initialButton();
//    delay(10);
//    Cursor = 0;
//    if (UP == LOW)   {flag_edit_mode++; if (flag_edit_mode > 2){flag_edit_mode = 0;}delay(200);}
//    if (DOWN == LOW) {flag_edit_mode--;if (flag_edit_mode < 0){flag_edit_mode = 2;}delay(200);}
//    if (ENTER == LOW){
//      delay(500);
//      if (flag_edit_mode==2)  {
//        SerialPC.println("$PAMTC,OPTION,SET,PING,OFF*55");  delay(500);
//        SerialPC.println("$PAMTC,OPTION,SET,OUTPUTMC,PING*0B"); delay(500);
//        SerialPC.println("$PAMTC,OPTION,SET,SLAVE,ON*46");       delay(500);
//        SerialPC.println("$PAMTC,OPTION,SET,SYNCMODE,OVERLAP*5D"); delay(500);
//        SerialPC.println("$PAMTC,OPTION,SET,PINGSPS,AUTO*45");     delay(500);//8,8,4,3  
//        SerialPC.println("$PAMTC,OPTION,SET,PULSESPP,2,12,18,24,S*6B");delay(500);
//        SerialPC.println("$PAMTC,OPTION,SET,PULSESPP,2,20,40,60,M*79");delay(500);
//        SerialPC.println("$PAMTC,OPTION,SET,RANGE,0*65");              delay(500);
//        SerialPC.println("$PAMTC,EEC,SUBSET,0,299*0C");                delay(500);
//        SerialPC.println("$PAMTC,OPTION,SET,PING,ON,M*7A");            delay(500);
//        SerialPC.println("$PAMTC,OPTION,SET,PING,OFF,S*2A");
//        goto menu;
//      }
//    }
//    delay(200);
//  }
//sinc mode
//      char buffer[100];
//      sprintf(buffer,"$PAMTC,OPTION,SET,PINGSPS,%d,%d,%d,%d,S*",pps_s,pps_l,pps_m,pps_v);
//      cmd = buffer;
//      //outputMsg(cmd);
//      memset(CRCbuffer,0,sizeof(CRCbuffer)); //clear data
//      
//      sprintf(buffer,"$PAMTC,OPTION,SET,PINGSPS,%d,%d,%d,%d,M*",pps_s,pps_l,pps_m,pps_v);
//      cmd = buffer;
//      //outputMsg(cmd);
//      memset(CRCbuffer,0,sizeof(CRCbuffer)); //clear data
//      delay(500);

//      char buffer[100];
//      sprintf(buffer,"$PAMTC,OPTION,SET,PINGSPS,%d,%d,%d,%d,S*",pps_s,pps_l,pps_m,pps_v);
//      cmd = buffer;
//      //outputMsg(cmd);
//      memset(CRCbuffer,0,sizeof(CRCbuffer)); //clear data
//      sprintf(buffer,"$PAMTC,OPTION,SET,PINGSPS,%d,%d,%d,%d,M*",pps_s,pps_l,pps_m,pps_v);
//      cmd = buffer;
//      //outputMsg(cmd);
//      memset(CRCbuffer,0,sizeof(CRCbuffer)); //clear data
//      delay(500);

//SerialPC.println("$PAMTC,OPTION,SET,PING,OFF");
//SerialPC.println("$PAMTC,OPTION,SET,OUTPUTMC,PING");
//SerialPC.println("$PAMTC,OPTION,SET,SLAVE,ON");
//SerialPC.println("$PAMTC,OPTION,SET,SYNCMODE,OVERLAP");
//SerialPC.println("$PAMTC,OPTION,SET,PINGSPS,AUTO");//8,8,4,3
//SerialPC.println("$PAMTC,OPTION,SET,PULSESPP,2,12,18,24,S");
//SerialPC.println("$PAMTC,OPTION,SET,PULSESPP,2,20,40,60,M");
//SerialPC.println("$PAMTC,OPTION,SET,RANGE,0");
//SerialPC.println("$PAMTC,EEC,SUBSET,0,299");
//SerialPC.println("$PAMTC,OPTION,SET,PING,ON M");
//SerialPC.println("$PAMTC,OPTION,SET,PING,OFF,S");
