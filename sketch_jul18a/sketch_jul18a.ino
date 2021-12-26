#include <Ewma.h>
#include <EwmaT.h>

// #include <NRF52_MBED_TimerInterrupt.h>
// #include <NRF52_MBED_ISR_Timer.h>

#include <ArduinoBLE.h>
#include <base64.hpp>
// #include <Mutex.h>
// #include <EMGFilters.h>
#include <Scheduler.h>

class mutex
{
private:
  bool locked = false;

public:
  int try_lock()
  {
    if (this->locked == false)
    {
      this->locked = !this->locked;
      return 1;
    }
    else
    {
      return 0;
    }
  }
  void lock()
  {
    while (!this->locked)
    {
      ;
    }
    this->locked = true;
  }
  void unlock()
  {
    if (this->locked)
    {
      this->locked = !this->locked;
    }
  }
};
class cir_buf
{
private:
  // int capacity = 0;
  // int data_count = 0;
  // uint16_t *array = nullptr;
  // uint16_t *head = nullptr;
  // uint16_t *now = nullptr;
  // uint16_t *tail = nullptr; //tail = next one(empty)

public:
  int capacity = 0;
  int data_count = 0;
  uint16_t *array = nullptr;
  uint16_t *head = nullptr;
  uint16_t *now = nullptr;
  uint16_t *tail = nullptr; //tail = next one(empty)
  cir_buf(int capacity = 2000)
  {
    if (capacity > 0)
    {
      this->array = new uint16_t[capacity];
      head = this->array;
      now = this->array;
      tail = this->array;
    }
    this->capacity = capacity;
  }
  void add(uint16_t data)
  {
    if (this->isEmpty())
    {
      data_count++;
      *this->now = data;
      this->tail = nextptr(this->tail);
    }
    else if (this->isFull())
    {
      // cout << "****FULLED" << flush;
      *this->tail = data;
      this->now = this->tail;
      this->head = nextptr(this->head);
      this->tail = nextptr(this->tail);
    }
    else
    {
      data_count++;
      *this->tail = data;
      this->now = this->tail;
      this->tail = nextptr(this->tail);
    }
  }
  uint16_t remove()
  {
    uint16_t out = 0;
    if (isEmpty())
    {
      return out;
    }

    if (data_count == 1)
    {
      data_count--;
      this->tail = prev(this->tail);
      out = *this->now;
      *this->now = 64; ////////////////////
                       // cout << "empty" << endl;
    }
    else
    {
      data_count--;
      out = *this->now;
      *this->now = 64; ////////////////////
      this->now = prev(this->now);
      this->tail = prev(this->tail);
    }
    return out;
  }
  uint16_t pop_front()
  {
    if (!isEmpty())
    {
      data_count--;
      uint16_t out = *this->head;
      *this->head = 64;
      this->head = nextptr(this->head);
      if (data_count == 0)
      {
        this->now = nextptr(this->now);
      }
      return out;
    }
    return 0;
  }
  uint16_t *nextptr(uint16_t *pos)
  {
    if (pos + 1 < this->array + this->capacity)
    {
      return pos + 1;
    }
    else
    {
      return this->array;
    }
  }
  uint16_t *prev(uint16_t *pos)
  {
    if (pos - 1 >= this->array)
    {
      return pos - 1;
    }
    else
    {
      return this->array + this->capacity - 1;
    }
  }
  bool isFull() { return (this->head == this->tail) && (this->head != this->now); }
  bool isEmpty() { return (this->head == this->tail) && (data_count == 0); }
  unsigned int getSize()
  {
    if (isFull())
    {
      return this->capacity;
    }
    if (this->now - this->head >= 0)
    {
      return this->now - this->head + 1;
    }
    else
    {
      return ((this->now - this->array) + 1 + (this->array + capacity - this->head));
    }
  }
  uint16_t getLatest()
  {
    if (!isEmpty())
    {
      return *this->now;
    }
    return 0;
  }
  uint16_t getItem(int index)
  {
    uint16_t *out = nullptr;
    if (index < capacity && index >= 0 && index < getSize())
    {
      if (this->head + index < this->array + capacity)
      {
        out = (this->head + index);
      }
      else
      {
        out = (this->array + index - (this->array + capacity - this->head));
      }
    }
    if (out)
    {
      return *out;
    }
    return 0;
  }
  void debug(int i)
  {
    // cout << setw(4) << i << "***";
    // cout << "|" << head << "|" << now << "|" << tail << "|";
    // cout << "isFULL:" << isFull() << " is Empty:" << isEmpty();
    // cout << "size:" << getSize();
    // cout << "data_count" << data_count;
    // cout << endl;
  }
  void printArray(bool abs = true)
  {
    // for (int i = 0; i < capacity; i++)
    // {
    //   if (abs)
    //   {
    //     cout << (char)this->array[i] << " | ";
    //   }
    //   else
    //   {
    //     cout << (char)getItem(i) << " | ";
    //   }
    // }
    // cout << endl;
  }
}; /////////////////////
const unsigned channel = 1;
unsigned long TIM0, TIM1, TIM2, counter, counter1, BLEcounter;

// NRF52_MBED_Timer ITimer(NRF_TIMER_3);
// mutex mutex;
// cir_buf *buffer = new cir_buf[channel];
cir_buf *buffer = new cir_buf(500);
uint16_t ADCdata;
EwmaT<uint16_t> Mfilter(1, 100);
int baseline = 0;
// EMGFilters myFilter;
// const int base64len = 15;
uint8_t dataRAW[100];
short base64data0, base64data1;
uint8_t dataB64[100];

///////////////////////////

int encode_64(cir_buf *buffer, uint8_t *output)
{
  if (buffer->getSize() > 52)
  {
    for (int i = 0; i < 25; i++)
    {
      base64data0 = buffer->pop_front();
      base64data1 = buffer->pop_front();
      dataRAW[i * 3] = (base64data0 & 111111110000) > 4;
      dataRAW[i * 3 + 1] = ((base64data0 & 1111) < 4) | ((base64data1 & 111100000000) > 8);
      dataRAW[i * 3 + 2] = (base64data1 & 11111111);
    }
    encode_base64(dataRAW, 75, output);

    return 1;
  }
  return 0;
  // encode_base64(dataPack, num_channel * 2, dataOut);
}

void TimerHandler0()
{
  // Doing something here inside ISR
  // No Serial.print() can be used
  // digitalWrite(13,!digitalRead(13));
  // buffer->add(analogRead(0));
}
// SAMPLE_FREQUENCY sampleRate = SAMPLE_FREQ_500HZ;
// NOTCH_FREQUENCY humFreq = NOTCH_FREQ_50HZ;

BLEService EMGService("1101");
BLECharacteristic EEMG0("0A81", BLERead | BLENotify, 100);
BLEUnsignedLongCharacteristic COUNTER("0A82", BLERead | BLENotify);
void setup()
{
  // put your setup code here, to run once:
  pinMode(13, OUTPUT);
  Serial.begin(115200);
  // BLE.setConnectionInterval(0x0006, 0x0007);

  // while (!Serial)
  // {
  //   delay(200);
  // }
  while (!BLE.begin())
  {
    delay(200);
  };
  BLE.setLocalName("EMG_Test");
  BLE.setAdvertisedService(EMGService);
  EMGService.addCharacteristic(EEMG0);
  EMGService.addCharacteristic(COUNTER);
  // BLE.setConnectionInterval(0x0006, 0x0007);
  BLE.addService(EMGService);
  BLE.advertise();

  // myFilter.init(sampleRate, humFreq, false, false, true);
  // Scheduler.start(loopBLE);
  Scheduler.startLoop(loopADC);
}

void loop()
{
  // Serial.println(buffer->pop_front());
  //  Serial.print ("NOW");
  //  Serial.println(buffer->getLatest());
  //  Serial.print ("OLD");

  BLEDevice central = BLE.central();
  // Serial.println(micros());
  if (central)
  {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected())
    {
      //////////////////////////////////////////////////////////////

      if (buffer->getLatest() - baseline > 20)
      {
        while (encode_64(buffer, dataB64))
        {
          if (millis() - TIM1 > 1000)
          {
            COUNTER.writeValue(BLEcounter);

            BLEcounter = 0;
            TIM1 = millis();
          }
          EEMG0.writeValue(dataB64, 100, false);
          BLEcounter++;
        }
      }

      yield();
    }
    Serial.println("central disconnected");
  }

  // if(buffer->getSize()>1){
  //   Serial.print("1000\t");
  //   Serial.print(buffer->getSize());
  //   Serial.print("\t");

  //  Serial.println(buffer->getLatest());
  //  Serial.println(buffer->pop_front());
  //  Serial.println(buffer->remove());
}

void loopADC()
{
  // Serial.println(buffer->pop_front());
  if (micros() - TIM0 >= 2000)
  {
    TIM0 = micros();
    ADCdata = analogRead(0);
    buffer->add(ADCdata);
    baseline = Mfilter.filter(ADCdata);

    // Serial.print(buffer->getLatest());
    // Serial.print(ADCdata);
    //       Serial.print("\t 1000 \t");
    //       Serial.println(baseline);
    // Serial.println(micros()-TIM0);
  }

  yield();
}
void loopBLE()
{
  // put your main code here, to run repeatedly:
  // BLEDevice central = BLE.central();
  // Serial.println(micros());
  // if (central)
  // {
  //   Serial.print("Connected to central: ");
  //   Serial.println(central.address());

  //   while (central.connected())
  //   {
  //     //////////////////////////////////////////////////////////////
  //     // EEMG0.writeValue();
  //     ///////////////////////////////////////////////////////////////
  //     //      EEMG0.writeValue(String((char*)dataSent));
  //     //      EEMG0.writeValue((char *)dataSent, true);
  //     //      EEMG0.writeValue((char*)dataBig,true);
  //     // /////////////////////////////////////////////////////////////
  //   }
  // }
}
