#include <cstdlib>
#include <stdio.h>
#include <Windows.h>
#include <assert.h>

#include <iostream>
#include <cstdio>

#include <string>
#include <vector>
#include <deque>

using namespace std;
void pause()
{
    std::cout << "Press enter to continue";
    std::cin.get();
}
void clear()
{
    // CSI[2J clears screen, CSI[H moves the cursor to top-left corner
    std::cout << "\x1B[2J\x1B[H";
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
std::vector<BYTE> base64_decode(std::string const &encoded_string)
{
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    BYTE char_array_4[4], char_array_3[3];
    std::vector<BYTE> ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
    {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4)
        {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret.push_back(char_array_3[i]);
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++)
            ret.push_back(char_array_3[j]);
    }

    return ret;
}

bool findAndConnect(deque<string> &raw_data_bundle)
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
            dcb.BaudRate = 256000;     //   over_serial_monitor limit
            dcb.ByteSize = 8;          //  data size, xmit and rcv
            dcb.Parity = NOPARITY;     //  parity bit
            dcb.StopBits = ONESTOPBIT; //  stop bit
            fSuccess = SetCommState(m_hCommPort, &dcb);
        }
        // COMMPROP infos;
        // GetCommProperties(m_hCommPort, &infos);
        // printf("Speed:%u\n", infos.dwSettableBaud);
        while (fSuccess)
        {
            // ClearCommError(m_hCommPort, &error, &comStatus);
            // if (comStatus.cbInQue > 1)
            // {

            if (ReadFile(m_hCommPort, &data, sizeof(data), &bytesRead, NULL))
            {
                // return true;
                if (data != '\n')
                {
                    raw_data_s.push_back(data);
                }
                else
                {
                    raw_data_bundle.push_back(raw_data_s);
                    // cout << raw_data_s << endl;
                    raw_data_s.clear();
                    if (raw_data_bundle.size() > 64)
                    {
                        raw_data_bundle.pop_front();
                        return true;
                    }
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

main()
{
    clear();
    deque<string> raw_data_bundle;

    if (findAndConnect(raw_data_bundle))
    {
        for (int i = 0; i < raw_data_bundle.size(); i++)
        {
            cout << raw_data_bundle[i] << endl;
        }
        printf("success\n");
    }

    pause();
    return 0;
}