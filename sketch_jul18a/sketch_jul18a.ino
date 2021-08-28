//#include <EMGFilters.h>
#include <base64.hpp>

#define _DEBUG      0
#define CALIBRATE 1
//int baseline = 200;

unsigned int dtime = micros();
unsigned int tdelay;

//char char_set[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" ;
char data_before[18];//no need \0 since it is temp
char data_pack[25];
//char data_pack[33];

unsigned int TIM0;//opt time  //32
//
unsigned short EMG0;          //16
unsigned short EMG1;          //16
//
unsigned short EMG2;          //16
unsigned short EMG3;          //16
//

unsigned short IMUX;            //8
unsigned short IMUY;            //8
unsigned short IMUZ;            //8
//
unsigned short IMURY;           //8
unsigned short IMURP;           //8
unsigned short IMURR;           //8
////////////////////
//unsigned short IMUX;
//unsigned short IMUY;
//unsigned short IMUZ;
////
//unsigned short IMURY;
//unsigned short IMURP;
//unsigned short IMURR;
/////////////////////
int data0;
int data1;
int data2;
int data3;

//EMGFilters Filter0;
//EMGFilters Filter1;
//EMGFilters Filter2;
//EMGFilters Filter3;
//SAMPLE_FREQUENCY sampleRate = SAMPLE_FREQ_1000HZ;
//NOTCH_FREQUENCY humFreq = NOTCH_FREQ_50HZ;

void setup() {
  // put your setup code here, to run once:
  //  pinMode(13, OUTPUT);
  
//   Filter0.init(sampleRate, humFreq, false, true, true);
//   Filter1.init(sampleRate, humFreq, false, true, true);
//   Filter2.init(sampleRate, humFreq, false, true, true);
//   Filter3.init(sampleRate, humFreq, false, true, true);
  
  //   myFilter.init(sampleRate, humFreq, true, true, true);

  
//  Serial.begin(115200);
  Serial.begin(256000);
}

void loop() {
  // put your main code here, to run repeatedly:
  //  TIM0=micros();
  dtime = micros();
  EMG0 =analogRead(A0);
  EMG1 =analogRead(A1);
  EMG2 =analogRead(A2);
  EMG3 =analogRead(A3);

//  EMG0=Filter0.update(EMG0);
//  EMG1=Filter1.update(EMG0);
//  EMG2=Filter2.update(EMG0);
//  EMG3=Filter3.update(EMG0);

  EMG3 =analogRead(A4);
  EMG3 =analogRead(A5);

  //144 bit in 6bit base64
  data_before[0]=(TIM0& 0b11111111000000000000000000000000)>>24;
  data_before[1]=(TIM0& 0b00000000111111110000000000000000)>>16;

  data_before[2]=highByte(TIM0);
  data_before[3]=lowByte (TIM0);

  data_before[4]=highByte(EMG0);
  data_before[5]=lowByte (EMG0);

  data_before[6]=highByte(EMG1);
  data_before[7]=lowByte (EMG1);

  data_before[8]=highByte(EMG2);
  data_before[9]=lowByte (EMG2);

  data_before[10]=highByte(EMG3);
  data_before[11]=lowByte (EMG3);
  
  data_before[12]=lowByte (IMUX);
  data_before[13]=lowByte (IMUY);
  data_before[14]=lowByte (IMUZ);
  
  data_before[15]=lowByte (IMURY);
  data_before[16]=lowByte (IMURP);
  data_before[17]=lowByte (IMURR);

  encode_base64(data_before,18,data_pack);
  
//
//  data_pack[0]=highByte();
//  data_pack[1]=lowByte();
//  
//  data_pack[0]=highByte();
//  data_pack[1]=lowByte();
//
//  data_pack[0]=highByte();
//  data_pack[1]=lowByte();



  
//  tdelay =micros()-dtime;

//  dtime = micros();  
  Serial.print(tdelay);
  Serial.println(data_pack);
  tdelay =micros()-dtime;

}
