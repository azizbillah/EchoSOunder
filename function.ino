void float_to_lcd(float ff){
  char char_ff[10];
  dtostrf(ff,3,2,char_ff);
  if (ff < 10 && ff >= 0){
    u8g.drawStr( 90, 20, "000");
    u8g.drawStr( 96, 20, char_ff);
  }
  else if (ff < 100 && ff >= 10){
    u8g.drawStr( 90, 20, "00");
    u8g.drawStr( 96, 20, char_ff);
  }
  else if (ff < 1000 && ff >= 100){
    u8g.drawStr( 90, 20, "0");
    u8g.drawStr( 96, 20, char_ff);
  }
  else {u8g.drawStr( 90, 20, char_ff);}
}
    
void send_echo(int Btn){
  if (Btn == 1){//request data XDR
    SerialPC.println("$PAMTC,EN,ALL,0*1D\r\n");     delay(500);
    char buffer[100];
    sprintf(buffer,"$PAMTC,EN,XDRX,1,%d*",interval);
    cmd = buffer;
    outputMsg(cmd); delay(500);
    memset(CRCbuffer,0,sizeof(CRCbuffer)); //clear data
    SerialPC.println("$PAMTC,EN,S*13\r\n");         delay(500);
    //SerialPC.println("c");
    flag_draw = 1;
    flagBtn = 0;
  }
  else if (Btn == 2){//request data DBT
    SerialPC.println("$PAMTC,EN,ALL,0*1D\r\n");     delay(500); 
    char buffer[100];
    sprintf(buffer,"$PAMTC,EN,DBT,1,%d*",interval);
    cmd = buffer;
    outputMsg(cmd); delay(500);
    memset(CRCbuffer,0,sizeof(CRCbuffer)); //clear data
    SerialPC.println("$PAMTC,EN,S*13\r\n");         delay(500);
   // SerialPC.println("b");
    flag_draw = 2;
    flagBtn = 0;
  }
  else if (Btn == 3){//request data STN
    SerialPC.println("$PAMTC,EN,ALL,0*1D\r\n");     delay(500);
    char buffer[100];
    sprintf(buffer,"$PAMTC,EN,XDRX,1,%d*",interval);
    cmd = buffer;
    outputMsg(cmd); delay(500);
    memset(CRCbuffer,0,sizeof(CRCbuffer)); //clear data
    SerialPC.println("$PAMTC,EN,S*13\r\n");         delay(500);
   //SerialPC.println("c");
    flag_draw = 3;
    flagBtn = 0;
  }
  else if (Btn == 0){}
}

char *gps_fix_status(int c){
  char *char_sat_fix;
  if      (c == 0){char_sat_fix = "no Gps";}
  else if (c == 1){char_sat_fix = "Gps fix";}
  else if (c == 2){char_sat_fix = "D Gps fix";}
  else if (c == 3){char_sat_fix = "gps fix";}
  else if (c == 4){char_sat_fix = "RTK, fix integers";}
  else if (c == 5){char_sat_fix = "RTK, float integers";}
  else if (c == 6){char_sat_fix = "Gps fix";}
  else if (c == 7){char_sat_fix = "Gps fix";}
  else if (c == 8){char_sat_fix = "Gps fix";}
  return char_sat_fix;
}

void edit_input(int c){
  char buffer[100];
   if (c == 1){
      float c_sow = sow*100;
      dtostrf(c_sow,3,2,char_sow);
      sprintf(buffer,"$PAMTC,OPTION,SET,SOSTW,%s,M*",char_sow);
      cmd = buffer;
      outputMsg(cmd);
    }
    else if (c == 2){
      float c_draft = draft*100;
      dtostrf(c_draft,3,2,char_draft);
      sprintf(buffer,"$PAMTC,OPTION,SET,DOFFSET,%s,M*",char_draft);
      cmd = buffer;
      outputMsg(cmd);
    }
    else{}
    memset(CRCbuffer,0,sizeof(CRCbuffer)); //clear data
    flag_edit_input = 0;
}

int digit_max(int c, int min_d, int max_d){
  int result;
  if      (c < min_d) {result = max_d;}
  else if (c > max_d) {result = min_d;}
  else                {result = c;}
  return result;
}

void initialButton(){
  ENTER = digitalRead(buttonENTER);
  UP = digitalRead(buttonUP);
  DOWN = digitalRead(buttonDOWN);
  MENU = digitalRead(buttonMENU);
}

float edit_digitf_plus(float f, int c){
  float result;
  if      (c == 0){result = f+1000.00;}
  else if (c == 1){result = f+0100.00;}
  else if (c == 2){result = f+0010.00;}
  else if (c == 3){result = f+0001.00;}
  else if (c == 4){result = f+0000.10;}
  else if (c == 5){result = f+0000.01;}
  return result;
}

float edit_digitf_min(float f, int c){
  float result;
  if      (c == 0){result = f-1000.00;}
  else if (c == 1){result = f-0100.00;}
  else if (c == 2){result = f-0010.00;}
  else if (c == 3){result = f-0001.00;}
  else if (c == 4){result = f-0000.10;}
  else if (c == 5){result = f-0000.01;}
  return result;
}

int edit_digiti_min(int f, int c){
  int result;
  if      (c == 0){result = f-1.00;}
  return result;
}
int edit_digiti_plus(int f, int c){
  int result;
  if      (c == 0){result = f+1.00;}
  return result;
}

char* string2char(String command){
  if(command.length()!=0){
    char *p = const_cast<char*>(command.c_str());
    return p;
  }
}

void outputMsg(String msg) {
  msg.toCharArray(CRCbuffer,sizeof(CRCbuffer));
  byte crc = convertToCRC(CRCbuffer);
  SerialPC.print(msg);
  if (crc < 16) SerialPC.print("0");
  SerialPC.println(crc,HEX);
}

byte convertToCRC(char *buff) {
  char c;
  byte i;
  byte start_with = 0,end_with = 0;
  byte crc = 0;

  for (i = 0; i < buff_len; i++) {
    c = buff[i];
    if(c == '$') {start_with = i;}
    if(c == '*') {end_with = i;}      
  }
  if (end_with > start_with){
    for (i = start_with+1; i < end_with; i++){ crc = crc ^ buff[i] ;}
  }
  else {//SerialPC.println("CRC ERROR");
    return 0;
  }
  return crc;
}
