#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_BUFFER_SIZE 256
#pragma comment(lib, "ws2_32.lib") // ���� ���̺귯�� ����� �˸���.

#include <WinSock2.h> // ���� ����� ���Ͽ� ��� ���� �߰�.
#include <stdio.h>
#include <iostream>
#include <vector>
#include <algorithm>

#pragma pack(1)
typedef struct
{
	char cData1; // 1byte
	char cData2; // 1byte
	int iData; // 4byte
}TEST;
#pragma pack()



SOCKET client_socket;
HANDLE hThread;
DWORD threadID;

std::vector<SOCKET> g_vSocket; // ���� ����.
HANDLE g_hMutex; // ���� ����.

DWORD WINAPI ProcessClient(LPVOID arg);

void err_quit(const char* msg); // ũ��Ƽ�� �� ��� 

void err_display(const char* msg); // ����� ���� �ΰ�� 