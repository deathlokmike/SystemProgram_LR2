// Lab1.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include "framework.h"
#include "Lab1.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Единственный объект приложения
string message = "";
CWinApp theApp;
HANDLE hMutex;
HANDLE hEventClose;
vector <HANDLE> vEventClose;

extern "C" {
    __declspec(dllimport) void __stdcall createEvents();
    __declspec(dllimport) int __stdcall getCurrentEvent();
    __declspec(dllimport) void __stdcall setEventConfirm();
    __declspec(dllimport) void __stdcall closeAllHandles();
    __declspec(dllimport) void __stdcall addMessageEvent();
    __declspec(dllimport) HANDLE __stdcall getMessageEvent(int thread_num);
    __declspec(dllimport) char* __stdcall getMessage(int& thread_num);
    __declspec(dllimport) void __stdcall setThreadEventMessage(int thread_num);
    __declspec(dllimport) void __stdcall setAllThreadEventMessage();
}

void ToFile(int num, string& message)
{
    string fname = to_string(num + 1) + ".txt";
    ofstream fout(fname);
    fout << message;
    fout.close();
}

UINT MyThread(LPVOID LPParametr)
{
    int num = (int)LPParametr;
    WaitForSingleObject(hMutex, INFINITE);
    cout << "Поток № " << num + 1 << " создан." << endl;
    ReleaseMutex(hMutex);
    HANDLE hEvents[] = { getMessageEvent(num), vEventClose[num] };
    bool flag = false;
    while (1)
    {
        switch (WaitForMultipleObjects(2, hEvents, FALSE, INFINITE) - WAIT_OBJECT_0)
        {
        case 0:
        {
            WaitForSingleObject(hMutex, INFINITE);
            ToFile(num, message);
            ReleaseMutex(hMutex);
            break;
        }
        case 1:
            WaitForSingleObject(hMutex, INFINITE);
            cout << "Поток № " << num + 1 << " завершен." << endl;
            ReleaseMutex(hMutex);
            flag = true;
            break;
        }
        if (flag)
            break;
    }
    return 0;
}

void start()
{
    int i = 0;
    hMutex = CreateMutex(NULL, FALSE, "mutex");   
    createEvents();
    bool cmdNotClosed = true;
    while (cmdNotClosed)
    {
        switch (getCurrentEvent())
        {
        case 0:
        {
            AfxBeginThread(MyThread, (LPVOID)(i++));
            hEventClose = CreateEvent(NULL, TRUE, FALSE, NULL);
            vEventClose.push_back(hEventClose);
            addMessageEvent();
            break;
        }
        case 1:
        {
            if (!vEventClose.empty()) 
            {
                PulseEvent(vEventClose[--i]); 
                CloseHandle(vEventClose[i]);
                vEventClose.pop_back(); 
                break;
            }
        }
        case 2:
        {
            cmdNotClosed = false;
            for (auto ev : vEventClose)
            {
                CloseHandle(ev);
            }
            break;
        }
        case 3:
        {
            int thread_num;
            char* tmp = getMessage(thread_num);
            message = string(tmp);
            if (thread_num == -1)
                setAllThreadEventMessage();
            else if (thread_num == -2)
                cout << message << endl;
            else
                setThreadEventMessage(thread_num);
        }
        }
        setEventConfirm();
    }
    CloseHandle(hMutex);
    closeAllHandles();
}

int main()
{
    setlocale(LC_ALL, "");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    int nRetCode = 0;
    HMODULE hModule = ::GetModuleHandle(nullptr);
    cout << "Консольное приложение запущено. " << endl;
    if (hModule != nullptr)
    {
        // инициализировать MFC, а также печать и сообщения об ошибках про сбое
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            wprintf(L"Критическая ошибка: сбой при инициализации MFC\n");
            nRetCode = 1;
        }
        else
        {
            start();
        }
    }
    else
    {
        wprintf(L"Критическая ошибка: сбой GetModuleHandle\n");
        nRetCode = 1;
    }

    return nRetCode;
}