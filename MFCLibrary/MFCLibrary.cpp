// MFCLibrary.cpp: определяет процедуры инициализации для библиотеки DLL.
//

#include "pch.h"
#include "framework.h"
#include "MFCLibrary.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: если эта библиотека DLL динамически связана с библиотеками DLL MFC,
//		все функции, экспортированные из данной DLL-библиотеки, которые выполняют вызовы к
//		MFC, должны содержать макрос AFX_MANAGE_STATE в
//		самое начало функции.
//
//		Например:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// тело нормальной функции
//		}
//
//		Важно, чтобы данный макрос был представлен в каждой
//		функции до вызова MFC.  Это означает, что
//		должен стоять в качестве первого оператора в
//		функции и предшествовать даже любым объявлениям переменных объекта,
//		поскольку их конструкторы могут выполнять вызовы к MFC
//		DLL.
//
//		В Технических указаниях MFC 33 и 58 содержатся более
//		подробные сведения.
//

// CMFCLibraryApp

struct Events
{
	HANDLE hEventStart = NULL;
	HANDLE hEventStop = NULL;
	HANDLE hEventCloseApp = NULL;
	HANDLE hEventConfirm = NULL;
	HANDLE hEventMessage = NULL;
};

struct Header
{
	int m_To;
	int m_Size;
};

Events ev;
vector <HANDLE> vEventMessage;

extern "C"
{
	__declspec(dllexport)
		void _stdcall createEvents()
	{
		ev.hEventStart = CreateEvent(NULL, FALSE, FALSE, "event_start");
		ev.hEventStop = CreateEvent(NULL, FALSE, FALSE, "event_stop");
		ev.hEventConfirm = CreateEvent(NULL, FALSE, FALSE, "event_confirm");
		ev.hEventCloseApp = CreateEvent(NULL, FALSE, FALSE, "event_close_app");
		ev.hEventMessage = CreateEvent(NULL, FALSE, FALSE, "event_message");
	}
	__declspec(dllexport)
		void _stdcall setEventStart()
	{
		SetEvent(ev.hEventStart);
	}

	__declspec(dllexport)
		void _stdcall setEventStop()
	{
		SetEvent(ev.hEventStop);
	}

	__declspec(dllexport)
		void _stdcall setEventConfirm()
	{
		SetEvent(ev.hEventConfirm);
	}

	__declspec(dllexport)
		void _stdcall setEventExit()
	{
		SetEvent(ev.hEventCloseApp);
	}

	__declspec(dllexport)
		void _stdcall setEventMessage()
	{
		SetEvent(ev.hEventMessage);
	}

	__declspec(dllexport)
		void _stdcall waitEventConfirm()
	{
		WaitForSingleObject(ev.hEventConfirm, INFINITE);
	}

	__declspec(dllexport)
		void _stdcall addMessageEvent()
	{
		vEventMessage.push_back(CreateEvent(NULL, FALSE, FALSE, NULL));
	}

	__declspec(dllexport)
		void _stdcall setThreadEventMessage(int thread_num)
	{
		SetEvent(vEventMessage[thread_num]);
	}

	__declspec(dllexport)
		void _stdcall setAllThreadEventMessage()
	{
		for (int i = 0; i < int(vEventMessage.size()); i++)
		{
			setThreadEventMessage(i);
		}
	}

	__declspec(dllexport)
		HANDLE _stdcall getMessageEvent(int thread_num)
	{
		return vEventMessage[thread_num];
	}

	__declspec(dllexport)
		int _stdcall getCurrentEvent()
	{
		HANDLE hEvents[] = { ev.hEventStart, ev.hEventStop, ev.hEventCloseApp, ev.hEventMessage };
		int eventNumber = WaitForMultipleObjects(4, hEvents, FALSE, INFINITE) - WAIT_OBJECT_0;
		return eventNumber;
	}

	__declspec(dllexport)
		void _stdcall createMessage(char* msg, int thread_num)
	{
		Header h;
		h = {thread_num, int(strlen(msg) + 1)};
		HANDLE hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(h) + h.m_Size, "FileMap");
		LPVOID pBuff = (BYTE*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(h) + h.m_Size);
		memcpy((char*)pBuff, &h, sizeof(h));
		memcpy((char*)pBuff + sizeof(h), msg, h.m_Size);
		setEventMessage();
		waitEventConfirm();
		UnmapViewOfFile((char*)pBuff);
		CloseHandle(hFileMap);
		
	}
	__declspec(dllexport)
		char* __stdcall getMessage(int& thread_num)
	{	
		Header h;
		HANDLE hFileMapRead = CreateFileMapping(NULL, NULL, PAGE_READWRITE, 0, sizeof(Header), "FileMap");
		LPVOID pBuffRead = (BYTE*)MapViewOfFile(hFileMapRead, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Header));
		memcpy(&h, pBuffRead, sizeof(Header));
		UnmapViewOfFile(pBuffRead);
		CloseHandle(hFileMapRead);
		thread_num = h.m_To;
		HANDLE hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Header) + h.m_Size, "FileMap");
		LPVOID pBuff = (BYTE*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Header) + h.m_Size);
		char* msg = new char[h.m_Size];
		memcpy(msg, (char*)pBuff + sizeof(h), h.m_Size);
		UnmapViewOfFile(pBuff);
		CloseHandle(hFileMap);
		return msg;
	}

	__declspec(dllexport)
		void _stdcall closeAllHandles()
	{
		CloseHandle(ev.hEventConfirm);
		CloseHandle(ev.hEventStart);
		CloseHandle(ev.hEventStop);
		CloseHandle(ev.hEventCloseApp);
		CloseHandle(ev.hEventMessage);
	}
}

BEGIN_MESSAGE_MAP(CMFCLibraryApp, CWinApp)
END_MESSAGE_MAP()


// Создание CMFCLibraryApp

CMFCLibraryApp::CMFCLibraryApp()
{
	// TODO: добавьте код создания,
	// Размещает весь важный код инициализации в InitInstance
}

// Единственный объект CMFCLibraryApp

CMFCLibraryApp theApp;


// Инициализация CMFCLibraryApp

BOOL CMFCLibraryApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}