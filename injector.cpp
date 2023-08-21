// Final Assignment Of Operating Systems
// Roey Gross - 328494091 - Campus Lev

// The DLL INJECTION - injector.cpp
// The program injects the hooking DLL in Notepad 
// For finding the PID of the proccess the program uses PIDFind function

#include "pch.h"
#include <TlHelp32.h>
#include <tlhelp32.h>


#define DLL_NAME "HOOKDLL.dll"
#define name L"notepad.exe" // proccess's name
#define WIN32_LEAN_AND_MEAN
#define BUFSIZE 4096

DWORD PIDFind(const std::wstring& processName);

int main()
{
	LPCSTR dllname = "HOOKDLL.dll"; //the dll is in the injector project file
	LPSTR* pPath = NULL;
	CHAR  PATH[BUFSIZE];
	DWORD pathlen = GetFullPathNameA(dllname, BUFSIZE, PATH, pPath); //get dll's path
	if (pathlen == 0)
	{
		cout << "error in GetFullPathNameA: " << GetLastError();
		return 0;
	}

	DWORD ID = PIDFind(name); // Find notepad's PID
	DWORD err;
	PVOID addrLoadLibrary = (LPVOID)GetProcAddress(GetModuleHandleA("KERNEL32.dll"), "LoadLibraryA");
	HANDLE 	proc = OpenProcess(PROCESS_ALL_ACCESS, false, ID);
	if (NULL == proc) {
		err = GetLastError();
		cout << "error in OpenProcess: "<< err << " probably the proccess is not open";
		return 0;
	}
	// Get a pointer to memory location in remote process,
	// big enough to store DLL path
	PVOID memAddr = (LPVOID)VirtualAllocEx(proc, NULL, strlen((char*)PATH) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (NULL == memAddr) {
		err = GetLastError();
		cout << "error in VirtualAllocEx: " << err;
		return 0;
	}
	// Write DLL name to remote process memory
	bool check = WriteProcessMemory(proc, (LPVOID)memAddr, PATH, strlen((char*)PATH) + 1, NULL);
	if (0 == check) {
		err = GetLastError();
		cout << "error in WriteProcessMemory: " << err;
		return 0;
	}
	// Open remote thread, while executing LoadLibrary
	// with parameter DLL name, will trigger DLLMain
	HANDLE hRemote = CreateRemoteThread(proc, NULL, NULL, (LPTHREAD_START_ROUTINE)addrLoadLibrary, (LPVOID)memAddr, NULL, NULL);
	if (NULL == hRemote) {
		int err = GetLastError();
		cout << "error in CreateRemoteThread: " << err;
		return 0;
	}
	WaitForSingleObject(hRemote, INFINITE);
	check = CloseHandle(hRemote);
	return 0;
}

DWORD PIDFind(const std::wstring& processName) // Finds the PID by his name
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processesSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	Process32First(processesSnapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile))
	{
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processesSnapshot, &processInfo))
	{
		if (!processName.compare(processInfo.szExeFile))
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}
	CloseHandle(processesSnapshot);
	return 0;
}