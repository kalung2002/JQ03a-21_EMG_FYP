#include <Windows.h>
#include <assert.h>

#include <iostream>
#include <stdio.h>

#include <cstdio>
#include <cstdlib>

#include <string>
#include <vector>
#include <queue>

#include <chrono>
#include <thread>
#include <atomic>

using namespace std;
class spin_lock
{
    static constexpr int UNLOCKED = 0;
    static constexpr int LOCKED = 1;

    std::atomic<int> m_value = 0;

public:
    void lock()
    {
        while (true)
        {
            int expected = UNLOCKED;
            if (m_value.compare_exchange_strong(expected, LOCKED))
                break;
        }
    }

    void unlock()
    {
        m_value.store(UNLOCKED);
    }
};
void pause()
{
    std::cout << "Press enter to continue";
    std::cin.get();
}
void clear()
{
    // CSI[2J clears screen, CSI[H moves the cursor to top-left corner
    std::cout << "\x1B[2J\x1B[H" << flush;
}
void PrintCommState(DCB dcb)
{
    //  Print some of the DCB structure values
    printf("\nBaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d\n",
           dcb.BaudRate,
           dcb.ByteSize,
           dcb.Parity,
           dcb.StopBits);
}

//https://renenyffenegger.ch/notes/development/Base64/Encoding-and-decoding-base-64-with-cpp/
//https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static inline bool is_base64(BYTE c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}
static const BYTE from_base64[] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62, 255, 62, 255, 63,
                                   52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255, 255, 255, 255, 255,
                                   255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
                                   15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 63,
                                   255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
                                   41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 255, 255, 255, 255, 255};

std::vector<BYTE> decode(std::string encoded_string)
{
    // Make sure string length is a multiple of 4
    while ((encoded_string.size() % 4) != 0)
        encoded_string.push_back('=');

    size_t encoded_size = encoded_string.size();
    std::vector<BYTE> ret;
    ret.reserve(3 * encoded_size / 4);

    for (size_t i = 0; i < encoded_size; i += 4)
    {
        // Get values for each group of four base 64 characters
        BYTE b4[4];
        b4[0] = (encoded_string[i + 0] <= 'z') ? from_base64[encoded_string[i + 0]] : 0xff;
        b4[1] = (encoded_string[i + 1] <= 'z') ? from_base64[encoded_string[i + 1]] : 0xff;
        b4[2] = (encoded_string[i + 2] <= 'z') ? from_base64[encoded_string[i + 2]] : 0xff;
        b4[3] = (encoded_string[i + 3] <= 'z') ? from_base64[encoded_string[i + 3]] : 0xff;

        // Transform into a group of three bytes
        BYTE b3[3];
        b3[0] = ((b4[0] & 0x3f) << 2) + ((b4[1] & 0x30) >> 4);
        b3[1] = ((b4[1] & 0x0f) << 4) + ((b4[2] & 0x3c) >> 2);
        b3[2] = ((b4[2] & 0x03) << 6) + ((b4[3] & 0x3f) >> 0);

        // Add the byte to the return value if it isn't part of an '=' character (indicated by 0xff)
        if (b4[1] != 0xff)
            ret.push_back(b3[0]);
        if (b4[2] != 0xff)
            ret.push_back(b3[1]);
        if (b4[3] != 0xff)
            ret.push_back(b3[2]);
    }

    return ret;
}

bool findAndConnect(queue<string> &raw_data_bundle, spin_lock &lock)
{
    vector<pair<string, string>> list;
    char lpTargetPath[5000]; // buffer to store the path of the COMPORTS
    bool portExist = false;  // in case the port is not found
    string comName;

    string useInput; //string input from user
    int input = 0;   //useInput turned into int

    string raw_data_s;

    HANDLE m_hCommPort = INVALID_HANDLE_VALUE;

    COMSTAT comStatus;
    DWORD bytesRead, error;
    char data;

    bool fSuccess = false;
    DCB dcb;
    SecureZeroMemory(&dcb, sizeof(DCB));

    do
    {
        for (int i = 0; i < 255; i++) // checking ports from COM0 to COM255
        {
            // converting to COM0, COM1, COM2
            comName = "COM" + to_string(i);
            //DWORD QueryDosDeviceW(
            //   LPCWSTR lpDeviceName,  c_str
            //   LPWSTR  lpTargetPath,  c_str
            //   DWORD   ucchMax
            //   );
            // Test the return value and error if any
            if (QueryDosDevice(comName.c_str(), lpTargetPath, 5000)) //QueryDosDevice returns zero if it didn't find an object
            {
                list.push_back(make_pair(comName, lpTargetPath));
                portExist = true;
            }
            // if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            // {
            // }
        }
        if (!portExist) //load waiting message if port note found
        {
            cout << "waiting device to connect" << flush;
            for (int i = 0; i < 10; i++)
            {
                Sleep(100);
                cout << " ." << flush;
            }
            cout << endl;
            clear();
        }
        else
        {
            clear();
        }
    } while (!portExist);

    if (portExist)
    {
        cout << "Select connection from list\n================" << endl;
        for (auto it = list.begin(); it != list.end(); ++it)
        {
            cout << it - list.begin() << " "
                 << " |Port:" << it->first
                 << " |Location:" << it->second
                 << endl;
        }
        cout << "================" << endl;
        cout << "Enter what device to connect: ";
        cin >> useInput;
        input = list.size() + 1; //that way input must go through stoi
        input = stoi(useInput);  //useInput convert into int
        cout << flush << endl;   //flush just to be safe
        if (input < list.size())
        {
            string deviceName = list[input].first;
            if (list[input].first.size() > 4)
            {
                // cout << list[input].first << "|||" << list[input].first.size() << endl;
                deviceName = "\\\\.\\" + deviceName;
                // cout << deviceName << endl;
            }
            m_hCommPort = ::CreateFile(
                deviceName.c_str(),
                GENERIC_READ | GENERIC_WRITE, //access ( read and write)
                0,                            //(share) 0:cannot share the COM port
                0,                            //security  (None)
                OPEN_EXISTING,                // creation : open_existing
                0,
                // FILE_FLAG_OVERLAPPED,         // we want overlapped operation
                0 // no templates file for COM port...
            );
        }
    }
    clear();

    if (m_hCommPort == INVALID_HANDLE_VALUE)
    {
        cout << "ERROR" << endl;
        return false;
    }
    else
    {
        fSuccess = GetCommState(m_hCommPort, &dcb);
        if (fSuccess)
        {
            dcb.DCBlength = sizeof(DCB);
            //  Fill in some DCB values and set the com state:
            //  115,200 bps, 8 data bits, no parity, and 1 stop bit.
            // dcb.BaudRate = CBR_115200; //  baud rate
            dcb.BaudRate = 500000; //   over_serial_monitor limit
            // dcb.BaudRate = 256000;     //   over_serial_monitor limit
            dcb.ByteSize = 8;          //  data size, xmit and rcv
            dcb.Parity = NOPARITY;     //  parity bit
            dcb.StopBits = ONESTOPBIT; //  stop bit
            fSuccess = SetCommState(m_hCommPort, &dcb);
            if (fSuccess)
            {
                cout << "Start reading data" << endl;
            }
        }
        // COMMPROP infos;
        // GetCommProperties(m_hCommPort, &infos);
        // printf("Speed:%u\n", infos.dwSettableBaud);
        std::this_thread::sleep_for(200ms);
        while (fSuccess)
        {
            // ClearCommError(m_hCommPort, &error, &comStatus);
            // if (comStatus.cbInQue > 1)
            // {
            if (ReadFile(m_hCommPort, &data, sizeof(data), &bytesRead, NULL))
            {
                // return true;
                // cout << data << flush;
                if (data != '\n')
                {
                    raw_data_s.push_back(data);
                }
                else
                {
                    // cout << raw_data_s << endl;
                    lock.lock();
                    raw_data_bundle.push(raw_data_s);

                    raw_data_s.clear();
                    while (raw_data_bundle.size() > 10)
                    {
                        raw_data_bundle.pop();
                        // return true;
                    }
                    lock.unlock();
                }
            }
            else
            {
                cerr << "Readfile error\n"
                     << flush;
                return false;
            }
            // }
        }
        return true;
    }
    return false;
}
void print_raw_data(queue<string> &raw_data_bundle, spin_lock &lock, vector<unsigned> &out)
{
    string buffer;
    vector<BYTE> byte_buffer;
    while (true)
    {
        // lock.lock();
        if (raw_data_bundle.size() > 0)
        {
            byte_buffer.clear();
            fill(out.begin(), out.end(), 0);
            lock.lock();
            buffer = raw_data_bundle.front();
            raw_data_bundle.pop();
            lock.unlock();

            byte_buffer = decode(buffer);

            // for (int i = 0; i < 18; i++)
            // {
            //     cout << ((byte_buffer[i] & 0b10000000) > 0);
            //     cout << ((byte_buffer[i] & 0b01000000) > 0);
            //     cout << ((byte_buffer[i] & 0b00100000) > 0);
            //     cout << ((byte_buffer[i] & 0b00010000) > 0);
            //     cout << ((byte_buffer[i] & 0b00001000) > 0);
            //     cout << ((byte_buffer[i] & 0b00000100) > 0);
            //     cout << ((byte_buffer[i] & 0b00000010) > 0);
            //     cout << ((byte_buffer[i] & 0b00000001) > 0);

            //     cout << endl;
            // }
            // cout << endl;

            out[10] = (unsigned)byte_buffer[17];
            out[9] = (unsigned)byte_buffer[16];
            out[8] = (unsigned)byte_buffer[15];
            out[7] = (unsigned)byte_buffer[14];
            out[6] = (unsigned)byte_buffer[13];
            out[5] = (unsigned)byte_buffer[12];

            out[4] += (unsigned)(byte_buffer[11]);
            out[4] += (unsigned)(byte_buffer[10] << 8);

            out[3] += (unsigned)(byte_buffer[9]);
            out[3] += (unsigned)(byte_buffer[8] << 8);

            out[2] += (unsigned)(byte_buffer[7]);
            out[2] += (unsigned)(byte_buffer[6] << 8);

            out[1] += (unsigned)(byte_buffer[5]);
            out[1] += (unsigned)(byte_buffer[4] << 8);

            out[0] += (unsigned)(byte_buffer[3]);
            out[0] += (unsigned)(byte_buffer[2] << 8);
            out[0] += (unsigned)(byte_buffer[1] << 16);
            out[0] += (unsigned)(byte_buffer[0] << 24);

            for (int i = 0; i < 11; i++)
            {
                printf("%8i : ", out[i]);
            }
            cout << endl;
        }
    }
}
main()
{
    clear();
    static queue<string> raw_data_bundle;
    static spin_lock lock_queue;
    static vector<unsigned> dataset(11);

    std::thread t_rawdata(findAndConnect, ref(raw_data_bundle), ref(lock_queue));
    std::thread t_printdata(print_raw_data, ref(raw_data_bundle), ref(lock_queue), ref(dataset));

    t_printdata.join();
    t_rawdata.join();

    // while (raw_data_bundle.size() > 0)
    // {
    //     cout << raw_data_bundle.front() << endl;
    //     raw_data_bundle.pop();
    // }
    printf("success\n");

    pause();
    return 0;
}