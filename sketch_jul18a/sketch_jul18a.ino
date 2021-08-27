#include <EMGFilters.h>
#define _DEBUG      0
#define CALIBRATE 1
//int baseline = 200;
long dtime=millis();

long maxtime=millis();
int local_max=0;
EMGFilters myFilter;



SAMPLE_FREQUENCY sampleRate = SAMPLE_FREQ_1000HZ;
NOTCH_FREQUENCY humFreq = NOTCH_FREQ_50HZ;

void setup() {
  // put your setup code here, to run once:
  pinMode(13, OUTPUT);
   myFilter.init(sampleRate, humFreq, false, true, true);
//   myFilter.init(sampleRate, humFreq, true, true, true);

  Serial.begin(115200);
//Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  dtime=millis();
  if(dtime%500==0){
    digitalWrite(13, !digitalRead(13));
   }
  
//  delay(200);  // Wait for 1000 millisecond(s)
int data =analogRead(A0);
int fdata = myFilter.update(data);
int sqdata=sq(fdata);


if(sqdata>local_max||abs(maxtime-dtime)>200){
    local_max=sqdata;
    maxtime=dtime;
  }




Serial.print(local_max);
 Serial.print("\t");
  Serial.print(sqdata);
  Serial.print("\t");
  Serial.print(1000);
  Serial.print("\t");
  Serial.print(1000);
  Serial.print("\t");
  Serial.print(1000);
  Serial.print("\t");
  Serial.print(1000);
  Serial.print("\t");
  Serial.print(1000);
  Serial.print("\t");
  Serial.println(2000);
//  Serial.println(data);
}
