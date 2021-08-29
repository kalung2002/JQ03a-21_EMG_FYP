//#include <EMGFilters.h>
//#define _DEBUG      0
//#define CALIBRATE 1
//int baseline = 200;
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#include <base64.hpp>

unsigned int duration = micros();
unsigned int counter = 0;
//char char_set[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" ;
char data_before[18]; //no need \0 since it is temp
char data_pack[25];
//char data_pack[33];

unsigned long TIM0; //opt time  //32
//
unsigned short EMG0; //16
unsigned short EMG1; //16
//
unsigned short EMG2; //16
unsigned short EMG3; //16
//

unsigned short IMUX; //8
unsigned short IMUY; //8
unsigned short IMUZ; //8
//
unsigned short IMURY; //8
unsigned short IMURP; //8
unsigned short IMURR; //8
////////////////////
//unsigned short IMUX;
//unsigned short IMUY;
//unsigned short IMUZ;
////
//unsigned short IMURY;
//unsigned short IMURP;
//unsigned short IMURR;
/////////////////////

//SAMPLE_FREQUENCY sampleRate = SAMPLE_FREQ_1000HZ;
//NOTCH_FREQUENCY humFreq = NOTCH_FREQ_50HZ;

void setup()
{
  // put your setup code here, to run once:
  pinMode(13, OUTPUT);

  //   myFilter.init(sampleRate, humFreq, true, true, true);

  //  Serial.begin(115200);
  // set prescale to 16
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);

  //  Serial.begin(256000);
  Serial.begin(500000);
}

void loop()
{
  // put your main code here, to run repeatedly:

  TIM0 = micros();

  EMG0 = analogRead(A0);
  EMG1 = analogRead(A1);
  EMG2 = analogRead(A2);
  EMG3 = analogRead(A3);

  EMG3 = analogRead(A4);
  EMG3 = analogRead(A5);
  IMURP = IMURY = IMURR = IMUX = IMUY = IMUZ = EMG3 = EMG2 = EMG1 = EMG0;

  //144 bit in 6bit base64
  data_before[0] = ((TIM0 & 0b11111111000000000000000000000000)>>24);
  data_before[1] = ((TIM0 & 0b111111110000000000000000)>>16);
  data_before[2] = highByte(TIM0);
  data_before[3] = lowByte(TIM0);

  data_before[4] = highByte(EMG0);
  data_before[5] = lowByte(EMG0);

  data_before[6] = highByte(EMG1);
  data_before[7] = lowByte(EMG1);

  data_before[8] = highByte(EMG2);
  data_before[9] = lowByte(EMG2);

  data_before[10] = highByte(EMG3);
  data_before[11] = lowByte(EMG3);

  data_before[12] = lowByte(IMUX);
  data_before[13] = lowByte(IMUY);
  data_before[14] = lowByte(IMUZ);

  data_before[15] = lowByte(IMURY);
  data_before[16] = lowByte(IMURP);
  data_before[17] = lowByte(IMURR);

  encode_base64(data_before, 18, data_pack);

//    Serial.print(micros());
//    Serial.print(TIM0);
  Serial.println(data_pack);
  duration = micros() - TIM0;

  if (duration < 900)
  {
    counter++;
    counter = counter % 500;
    if (counter > 250)
    {

      digitalWrite(13, HIGH);
    }
    else
    {
      digitalWrite(13, LOW);
    }

  }
}
