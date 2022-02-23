#include "Server.h"



int main()
{
	g_hMutex = CreateMutex(NULL, false, NULL);
	if (NULL == g_hMutex) return -1; // 뮤텍스 생성 실패
	if (GetLastError() == ERROR_ALREADY_EXISTS) // 이미 생성된 뮤텍스가 있습니다.
	{
		CloseHandle(g_hMutex);
		return -1;
	}

	// 원속을 이용하여 작업을 하겠다. 원속 초기화 
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return -1;
	//MessageBox(NULL, "윈속을 사용할 준비가 되었다.", "윈속 사용 준비 완료.", MB_OK);

	// socket() listen_socket 생성.
	SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == listen_socket) err_quit("socket");	
	
	// 서버 정보 객체 설정.
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	
	if (bind(listen_socket, (SOCKADDR*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
	{
		closesocket(listen_socket);
		WSACleanup();
		err_quit("bind");
	}

	// listen() 수신 대기열 생성.
	if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR)
	{
		closesocket(listen_socket);
		WSACleanup();
		err_quit("listen");
	}

	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(SOCKADDR_IN);
	ZeroMemory(&clientaddr, addrlen);
	SOCKET client_socket;
	HANDLE hThread;
	DWORD threadID;

	while (1) {
		// accept() 연결 대기.
		client_socket = accept(listen_socket, (SOCKADDR*)&clientaddr, &addrlen);
		if (INVALID_SOCKET == client_socket) continue;

		printf("\n[TCP 서버] 클라이언트 접속 : IP 주소=%s, 포트 번호=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		WaitForSingleObject(g_hMutex, INFINITE);
		// 다른 스레드에서도 사용하는 공용 리소스를 사용한다.
		g_vSocket.push_back(client_socket);
		ReleaseMutex(g_hMutex);

		// 스레드 생성.
		hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_socket, 0, &threadID);
		if (NULL == hThread) std::cout << "[오류] 스레드 생성 실패!" << std::endl;
		else CloseHandle(hThread);
	}

	closesocket(listen_socket);
	CloseHandle(g_hMutex);

	// 원속 사용이 끝났다.
	WSACleanup();
}


DWORD WINAPI ProcessClient(LPVOID arg)
{
	SOCKET client_sock = (SOCKET)arg;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen); // 클라이언트 정보 얻기.
																
	// recv(), send()
	int len, retval;
	char buf[sizeof(TEST)];
	TEST send_packet;

	
	while (1)
	{
		// 데이터 받기.
		ZeroMemory(buf, sizeof(buf));
		retval = recv(client_sock, buf, sizeof(buf), 0);

		if (SOCKET_ERROR == retval) break;
		else if (0 == retval) break;

		TEST* recv_packet = (TEST*)buf;

		// 받은 데이터 출력.
		
		printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", retval);
		printf("[TCP 클라이언트] 첫번째 데이터는 %c\n", recv_packet->cData1);
		printf("[TCP 클라이언트] 두번째 데이터는 %c\n", recv_packet->cData2);
		printf("[TCP 클라이언트] 세번째 데이터는 %d\n", recv_packet->iData);
		
		


		WaitForSingleObject(g_hMutex, INFINITE);
		// 데이터 보내기.
		for (const SOCKET& sock : g_vSocket) // 접속한 모든 클라이언트에 데이터 전송.
		{
			//retval = send(sock, buf, sizeof(buf), 0);
			retval = send(sock, (char*)recv_packet, sizeof(TEST), 0);
			if (SOCKET_ERROR == retval) break;

			//getpeername(sock, (SOCKADDR*)&clientaddr, &addrlen); // 클라이언트 정보 얻기.
			//printf("[TCP 서버] 클라이언트 : IP 주소=%s, 포트 번호=%d  %d바이트를 보냈습니다.\n",
			//	inet_ntoa(clientaddr.sin_addr),
			//	ntohs(clientaddr.sin_port),
			//	retval);
		}
		ReleaseMutex(g_hMutex);
	}

	WaitForSingleObject(g_hMutex, INFINITE);
	// vector에서 접속을 종료한 socket 제거.
	auto itr = std::find(g_vSocket.begin(), g_vSocket.end(), client_sock);
	if (g_vSocket.end() != itr) g_vSocket.erase(itr);
	ReleaseMutex(g_hMutex);

	// closesocket()
	closesocket(client_sock);

	printf("\n[TCP 서버] 클라이언트 종료 : IP 주소=%s, 포트 번호=%d\n",
		inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port));

	return 0;
}

void err_quit(const char* msg) // 크리티컬 한 경우 
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	// msg : 메시지 박스의 타이틀(Caption)
	MessageBox(NULL, (LPCSTR)lpMsgBuf, msg, MB_ICONERROR);
	// 메모리 해제, 핸들 무효화.
	LocalFree(lpMsgBuf);
	exit(-1);
}

void err_display(const char* msg) // 사소한 오류 인경우 
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s\n", msg, (LPCSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}


