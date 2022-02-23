#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_BUFFER_SIZE 256
#pragma comment(lib, "ws2_32.lib") // 윈속 라이브러리 사용을 알린다.

#include <WinSock2.h> // 윈속 사용을 위하여 헤더 파일 추가.
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

std::vector<SOCKET> g_vSocket; // 전역 변수.
HANDLE g_hMutex; // 전역 변수.

DWORD WINAPI ProcessClient(LPVOID arg);

void err_quit(const char* msg); // 크리티컬 한 경우 

void err_display(const char* msg); // 사소한 오류 인경우 