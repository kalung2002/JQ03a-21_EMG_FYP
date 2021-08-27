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

bool findAndConnect(deque<string> &raw_data)
{
    vector<pair<string, string>> list;
    char lpTargetPath[5000]; // buffer to store the path of the COMPORTS
    bool gotPort = false;    // in case the port is not found

    string sinput; //string input from user
    int input = 0; //sinput turned into int

    string raw_data_s;
    HANDLE m_hCommPort = INVALID_HANDLE_VALUE;
    do
    {
        for (int i = 0; i < 255; i++) // checking ports from COM0 to COM255
        {
            // converting to COM0, COM1, COM2
            string str = "COM" + to_string(i);
            LPCSTR lstr = str.c_str();
            //DWORD QueryDosDeviceW(
            //   LPCWSTR lpDeviceName,
            //   LPWSTR  lpTargetPath,
            //   DWORD   ucchMax
            //   );
            DWORD test = QueryDosDevice(lstr, lpTargetPath, 5000);

            // Test the return value and error if any
            if (test != 0) //QueryDosDevice returns zero if it didn't find an object
            {
                // pair<string, string> buffer(lstr, lpTargetPath);
                list.push_back(make_pair(lstr, lpTargetPath));
                gotPort = true;
            }

            // if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            // {
            // }
        }
        if (!gotPort)
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
    } while (!gotPort);

    if (gotPort)
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
        cin >> sinput;
        input = list.size() + 1; //that way input must go through stoi
        input = stoi(sinput);    //sinput convert into int
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
        bool fSuccess = false;
        DCB dcb;
        SecureZeroMemory(&dcb, sizeof(DCB));
        dcb.DCBlength = sizeof(DCB);
        //  Fill in some DCB values and set the com state:
        //  115,200 bps, 8 data bits, no parity, and 1 stop bit.
        dcb.BaudRate = CBR_115200; //  baud rate
        dcb.ByteSize = 8;          //  data size, xmit and rcv
        dcb.Parity = NOPARITY;     //  parity bit
        dcb.StopBits = ONESTOPBIT; //  stop bit

        fSuccess = GetCommState(m_hCommPort, &dcb);
        while (fSuccess)
        {
            COMSTAT comStatus;
            DWORD bytesRead, error;
            char data;
            bool result = false;
            ClearCommError(m_hCommPort, &error, &comStatus);
            if (comStatus.cbInQue)
            {
                result = ReadFile(m_hCommPort, &data, sizeof(data), &bytesRead, NULL);
                if (data == '\n')
                {
                    raw_data.push_back(raw_data_s);
                    // cout << raw_data_s << endl;

                    raw_data_s.clear();
                    if (raw_data.size() > 256)
                    {
                        raw_data.pop_front();
                        return false;
                    }
                }
                else
                {
                    raw_data_s.push_back(data);
                }
            }
        }
        return true;
    }
    return false;
}

main()
{
    clear();
    deque<string> raw_data;

    findAndConnect(raw_data);
    for (int i = 0; i < raw_data.size(); i++)
    {
        cout << raw_data[i] << endl;
    }

    printf("success\n");
    pause();
    return 0;
}