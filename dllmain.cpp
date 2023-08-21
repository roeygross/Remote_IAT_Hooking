// Final Assignment Of Operating Systems
// Roey Gross - 328494091 - Campus Lev

// The Hooking DLL - dllmain.cpp
// The DLL activates the Hooking function
// The activation is automaticly right after a proccess loaded the DLL
// The Hooking function hooks createFile and sends message to the server

#define WIN32_LEAN_AND_MEAN
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include <windows.h>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

#define DLL_EXPORT
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "8000" 



DWORD saved_hooked_func_addr;
void activateClient();

// Hook function hooks the createfile function and activates activateClient instead
int hook(PCSTR func_to_hook, PCSTR DLL_to_hook, DWORD new_func_addres)
{
	//Initializing parameters
	PIMAGE_DOS_HEADER dosHeader;
	PIMAGE_NT_HEADERS NTHeader;
	PIMAGE_OPTIONAL_HEADER32 optionalHeader;
	IMAGE_DATA_DIRECTORY importDirectory;
	DWORD descriptorStartRVA;
	PIMAGE_IMPORT_DESCRIPTOR importDescriptor;
	int index;

	// We need to get inside the IAT and hook createFile over there

	// Get base address of currently running .exe
	DWORD baseAddress = (DWORD)GetModuleHandle(NULL);

	// Get the import directory address
	dosHeader = (PIMAGE_DOS_HEADER)(baseAddress);

	if (((*dosHeader).e_magic) != IMAGE_DOS_SIGNATURE) {
		return 0;
	}

	// Locate NT header
	NTHeader = (PIMAGE_NT_HEADERS)(baseAddress + (*dosHeader).e_lfanew);
	if (((*NTHeader).Signature) != IMAGE_NT_SIGNATURE) {
		return 0;
	}

	// Locate optional header
	optionalHeader = &(*NTHeader).OptionalHeader;
	if (((*optionalHeader).Magic) != 0x10B) {
		return 0;
	}

	importDirectory = (*optionalHeader).DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	descriptorStartRVA = importDirectory.VirtualAddress;

	importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(descriptorStartRVA + (*optionalHeader).ImageBase);

	index = 0;
	char* DLL_name;
	// Look for the DLL which includes the function for hooking
	while (importDescriptor->Characteristics != 0) {
		DLL_name = (char*)(baseAddress + importDescriptor->Name);
		printf("DLL name: %s\n", DLL_name);
		if (!strcmp(DLL_to_hook, DLL_name))
			break;
		index++;
	}

	// exit if the DLL is not found in import directory
	if (importDescriptor[index].Characteristics == 0) {
		printf("DLL was not found");
		return 0;
	}

	// Search for requested function in the DLL
	PIMAGE_THUNK_DATA thunkILT; // Import Lookup Table - names
	PIMAGE_THUNK_DATA thunkIAT; // Import Address Table - addresses
	PIMAGE_IMPORT_BY_NAME nameData;

	thunkILT = (PIMAGE_THUNK_DATA)(optionalHeader->ImageBase + importDescriptor[index].OriginalFirstThunk);
	thunkIAT = (PIMAGE_THUNK_DATA)(importDescriptor[index].FirstThunk + optionalHeader->ImageBase);
	if ((thunkIAT == NULL) or (thunkILT == NULL)) {
		return 0;
	}

	while (((*thunkILT).u1.AddressOfData != 0) & (!((*thunkILT).u1.Ordinal & IMAGE_ORDINAL_FLAG))) {
		nameData = (PIMAGE_IMPORT_BY_NAME)(baseAddress + (*thunkILT).u1.AddressOfData);
		if (!strcmp(func_to_hook, (char*)(*nameData).Name))
			break;
		thunkIAT++;
		thunkILT++;
	}

	// Hook IAT: Write over function pointer
	DWORD dwOld = NULL;
	saved_hooked_func_addr = (*thunkIAT).u1.Function;
	VirtualProtect((LPVOID) & ((*thunkIAT).u1.Function), sizeof(DWORD), PAGE_READWRITE, &dwOld);
	DWORD new_func_add = (DWORD)&activateClient; //put activateClient instead
	(*thunkIAT).u1.Function = new_func_add;
	VirtualProtect((LPVOID) & ((*thunkIAT).u1.Function), sizeof(DWORD), dwOld, NULL);
	return 1;
}

// Create a client socket and send to the server that createFile were activated
void activateClient() {
	// Client creation
	// In order to create and use the client we need to:
	// 1. Initialize winsock
	// 2. Create a socket
	// 3. Connect
	// 4. Send data
	// 5. Shutdown Winsock
	// 6. Close the socket and cleanup

	// 1. Initialize winsock
	WSADATA wsaData;
	SOCKET sock = INVALID_SOCKET;
	struct addrinfo* pInfo = NULL,* ptr = NULL, info;
	const char* sendbuf = "Hey server, CreateFile were called right now!";
	char recvbuf[DEFAULT_BUFLEN];
	int ipInfo;
	int recvbuflen = DEFAULT_BUFLEN;


	ipInfo = WSAStartup(MAKEWORD(2, 2), &wsaData);

	// 2. Create a socket + 3. Connect
	ZeroMemory(&info, sizeof(info));
	info.ai_family = AF_UNSPEC;
	info.ai_socktype = SOCK_STREAM;
	info.ai_protocol = IPPROTO_TCP;

	// Get server address and port
	ipInfo = getaddrinfo("127.0.0.1", DEFAULT_PORT, &info, &pInfo);

	// Connect to an address until succeeds
	for (ptr = pInfo; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		ipInfo = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (ipInfo == SOCKET_ERROR) {
			closesocket(sock);
			sock = INVALID_SOCKET;
			continue;
		}
		break;
	}
	freeaddrinfo(pInfo);

	// 4. Send data
	ipInfo = send(sock, sendbuf, (int)strlen(sendbuf) + 1, 0);

	// 5. Shutdown Winsock
	ipInfo = shutdown(sock, SD_SEND);

	// 6. Close the socket and cleanup
	closesocket(sock);
	WSACleanup();

	//assembly code lets the program run  the createfile right after the hook,
	//when we enter createfile the stack will act like we have not done hooking and will run createfile succcesfully
	_asm
	{
		pop     edi
		pop     esi
		xor ecx, ebp
		pop     ebx
		mov esp, ebp
		pop ebp
		jmp saved_hooked_func_addr //go back to createfile
	}
}

// dll main activates the hook func
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) // Reserved
{
	PCSTR func_to_hook = "CreateFileW";
	PCSTR DLL_to_hook = "KERNEL32.dll";
	DWORD new_func_address = (DWORD)&activateClient;

	
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hook(func_to_hook, DLL_to_hook, new_func_address); //when dll is loaded hook will be called
		break;
	case DLL_THREAD_ATTACH:
		// A process is creating a new thread.
		break;
	case DLL_THREAD_DETACH:
		// A thread exits normally.
		break;
	case DLL_PROCESS_DETACH:
		// A process unloads the DLL.
		break;
	}
	return TRUE;
}

