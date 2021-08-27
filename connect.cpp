#include <cstdlib>
#include <stdio.h>
#include <Windows.h>
#include <assert.h>

#include <iostream>
#include <cstdio>

#include <string>
#include <vector>

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

bool SelectComPort(vector<pair<string, string>> &list) //added function to find the present serial
{
    char lpTargetPath[5000]; // buffer to store the path of the COMPORTS
    bool gotPort = false;    // in case the port is not found
    list.clear();
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

        if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
        }
    }
    return gotPort;
}
int listAndChoose(vector<pair<string, string>> &list)
{
    int in = -1;
    bool mode = 0;
    do
    {
        if (SelectComPort(list)) //if got port
        {
            mode = 1;
            cout << "Select connection from list\n================" << endl;
            for (auto it = list.begin(); it != list.end(); ++it)
            {
                cout << it - list.begin() << " "
                     << " |Port:" << it->first
                     << " |Location:" << it->second
                     << endl;
            }
            cout << "================" << endl;
        }
        else
        {
            cout << "waiting device to connect" << flush;
            Sleep(100);
            for (int i = 0; i < 10; i++)
            {
                Sleep(100);
                cout << " ." << flush;
            }
            cout << endl;
            clear();
        }

    } while (!mode);
    if (mode)
    {
        string input;
        cout << "Enter what device to connect: ";
        cin >> input;
        in = stoi(input);
        cout << flush << endl;
        clear();
    }
    return in;
}
bool connectToPort(vector<pair<string, string>> &list, int i)
{
    if (i >= list.size())
    {
        return false;
    }
    string deviceName = list[i].first;
    if (list[i].first.size() > 4)
    {
        cout << list[i].first << "|||" << list[i].first.size() << endl;
        deviceName = "\\\\.\\" + deviceName;
        cout << deviceName << endl;
    }
    HANDLE m_hCommPort = ::CreateFile(
        deviceName.c_str(),
        GENERIC_READ | GENERIC_WRITE, //access ( read and write)
        0,                            //(share) 0:cannot share the COM port
        0,                            //security  (None)
        OPEN_EXISTING,                // creation : open_existing
        0,
        // FILE_FLAG_OVERLAPPED,         // we want overlapped operation
        0 // no templates file for COM port...
    );
    if (m_hCommPort == INVALID_HANDLE_VALUE)
    {
        cout << "ERROR" << endl;
        return false;
    }
    else
    {
        bool fSuccess = false;
        cout << "EEEEEEEEEEEEEEEEEEEEEE" << endl;
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
            DWORD bytesRead, eventMask;
            char data;
            bool result = false;
            result = ReadFile(m_hCommPort, &data, sizeof(data), &bytesRead, NULL);
            if (data == '\t')
            {
                cout << endl
                     << endl;
            }
            else
            {
                printf("%c", data);
            }
        }
        return true;
    }
    return false;
}
main()
{
    clear();
    vector<pair<string, string>> list;
    int i = listAndChoose(list);
    if (!list.empty() && i >= 0 && i < list.size())
    {
        cout << "connecting to port " << i << "|" << list[i].first << endl;
    }
    connectToPort(list, i);
    printf("success\n");
    pause();
    return 0;
}