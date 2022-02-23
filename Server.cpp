#include "Server.h"



int main()
{
	g_hMutex = CreateMutex(NULL, false, NULL);
	if (NULL == g_hMutex) return -1; // ���ؽ� ���� ����
	if (GetLastError() == ERROR_ALREADY_EXISTS) // �̹� ������ ���ؽ��� �ֽ��ϴ�.
	{
		CloseHandle(g_hMutex);
		return -1;
	}

	// ������ �̿��Ͽ� �۾��� �ϰڴ�. ���� �ʱ�ȭ 
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return -1;
	//MessageBox(NULL, "������ ����� �غ� �Ǿ���.", "���� ��� �غ� �Ϸ�.", MB_OK);

	// socket() listen_socket ����.
	SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == listen_socket) err_quit("socket");	
	
	// ���� ���� ��ü ����.
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

	// listen() ���� ��⿭ ����.
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
		// accept() ���� ���.
		client_socket = accept(listen_socket, (SOCKADDR*)&clientaddr, &addrlen);
		if (INVALID_SOCKET == client_socket) continue;

		printf("\n[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		WaitForSingleObject(g_hMutex, INFINITE);
		// �ٸ� �����忡���� ����ϴ� ���� ���ҽ��� ����Ѵ�.
		g_vSocket.push_back(client_socket);
		ReleaseMutex(g_hMutex);

		// ������ ����.
		hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_socket, 0, &threadID);
		if (NULL == hThread) std::cout << "[����] ������ ���� ����!" << std::endl;
		else CloseHandle(hThread);
	}

	closesocket(listen_socket);
	CloseHandle(g_hMutex);

	// ���� ����� ������.
	WSACleanup();
}


DWORD WINAPI ProcessClient(LPVOID arg)
{
	SOCKET client_sock = (SOCKET)arg;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen); // Ŭ���̾�Ʈ ���� ���.
																
	// recv(), send()
	int len, retval;
	char buf[sizeof(TEST)];
	TEST send_packet;

	
	while (1)
	{
		// ������ �ޱ�.
		ZeroMemory(buf, sizeof(buf));
		retval = recv(client_sock, buf, sizeof(buf), 0);

		if (SOCKET_ERROR == retval) break;
		else if (0 == retval) break;

		TEST* recv_packet = (TEST*)buf;

		// ���� ������ ���.
		
		printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� �޾ҽ��ϴ�.\n", retval);
		printf("[TCP Ŭ���̾�Ʈ] ù��° �����ʹ� %c\n", recv_packet->cData1);
		printf("[TCP Ŭ���̾�Ʈ] �ι�° �����ʹ� %c\n", recv_packet->cData2);
		printf("[TCP Ŭ���̾�Ʈ] ����° �����ʹ� %d\n", recv_packet->iData);
		
		


		WaitForSingleObject(g_hMutex, INFINITE);
		// ������ ������.
		for (const SOCKET& sock : g_vSocket) // ������ ��� Ŭ���̾�Ʈ�� ������ ����.
		{
			//retval = send(sock, buf, sizeof(buf), 0);
			retval = send(sock, (char*)recv_packet, sizeof(TEST), 0);
			if (SOCKET_ERROR == retval) break;

			//getpeername(sock, (SOCKADDR*)&clientaddr, &addrlen); // Ŭ���̾�Ʈ ���� ���.
			//printf("[TCP ����] Ŭ���̾�Ʈ : IP �ּ�=%s, ��Ʈ ��ȣ=%d  %d����Ʈ�� ���½��ϴ�.\n",
			//	inet_ntoa(clientaddr.sin_addr),
			//	ntohs(clientaddr.sin_port),
			//	retval);
		}
		ReleaseMutex(g_hMutex);
	}

	WaitForSingleObject(g_hMutex, INFINITE);
	// vector���� ������ ������ socket ����.
	auto itr = std::find(g_vSocket.begin(), g_vSocket.end(), client_sock);
	if (g_vSocket.end() != itr) g_vSocket.erase(itr);
	ReleaseMutex(g_hMutex);

	// closesocket()
	closesocket(client_sock);

	printf("\n[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port));

	return 0;
}

void err_quit(const char* msg) // ũ��Ƽ�� �� ��� 
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	// msg : �޽��� �ڽ��� Ÿ��Ʋ(Caption)
	MessageBox(NULL, (LPCSTR)lpMsgBuf, msg, MB_ICONERROR);
	// �޸� ����, �ڵ� ��ȿȭ.
	LocalFree(lpMsgBuf);
	exit(-1);
}

void err_display(const char* msg) // ����� ���� �ΰ�� 
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


