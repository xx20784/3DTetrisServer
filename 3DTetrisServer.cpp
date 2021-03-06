#include "stdafx.h"

#pragma comment (lib , "ws2_32.lib")

UINT HighestScore = 0;

bool LoadHightestScoreFromFile() {
	std::ifstream file("score.txt");

	if (file.fail()) {
		return false;
	}

	file >> HighestScore;
	return true;
}

void SaveHightestScoreToFile(char* _ip, USHORT _port) {
	std::ofstream file("score.txt");
	file << HighestScore << " " << _ip << " " << _port;
}

UINT WINAPI WorkThread(void *_data) {
	SOCKET sock = (SOCKET)_data;

	struct sockaddr_in client_addr;
	int addr_len = sizeof(struct sockaddr_in);
	getpeername(sock, (struct sockaddr*)&client_addr, &addr_len);

	printf_s("New Challenger Play Game : %s(%d)\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	printf_s("Allocation Thread : %d\n", GetCurrentThreadId());

	int getScore = 0;
	recv(sock, (char*)&getScore, sizeof(UINT), 0);

	if (getScore >= HighestScore) {
		HighestScore = getScore;
		printf_s("New Best Player!!! : %s(%d)\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		printf_s("HightestScore : %d\n", HighestScore);
		SaveHightestScoreToFile(inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	}

	closesocket(sock);
	printf_s("Delete Thread : %d\n\n", GetCurrentThreadId());
	return 0;
}

int main(int _argc, char** _argv) {
	if (LoadHightestScoreFromFile() == false) {
		printf_s("Load HightestScore from File");
		return 1;
	}

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf_s("WSAStartup Fail");
		return 1;
	}

	SOCKET listenSocket;
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET) {
		printf_s("Create Listen Socket Fail");
		return 1;
	}

	struct sockaddr_in server_addr;
	ZeroMemory(&server_addr, sizeof(struct sockaddr_in));
	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(listenSocket, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
		printf_s("Bind Listen Socket Fail");
		closesocket(listenSocket);
		return 1;
	}

	if (listen(listenSocket, 5) == SOCKET_ERROR) {
		printf_s("Wait Client Fail");
		closesocket(listenSocket);
		return 1;
	}

	printf_s("Run Server......\n");

	SOCKET clientSocket;
	struct sockaddr_in client_addr;
	ZeroMemory(&client_addr, sizeof(struct sockaddr_in));

	HANDLE hThread = nullptr;
	while (true) {
		// Client의 Accpet요청을 받고 Client와의 연결 Socket을 생성해 clientSocket에 저장한다.
		int addr_len = sizeof(struct sockaddr_in);
		clientSocket = accept(listenSocket, (struct sockaddr*) &client_addr, &addr_len);
		// Client Socket을 토대로 Client와 통신할 함수(WorkThread)를 Multi-Thread로 실행한다.
		hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, &WorkThread, reinterpret_cast<void*>(clientSocket), 0, nullptr));
		// 굳이 Main Thread에서 제어할거 아니면 ThreadHandle을 곧장 해제해준다.
		CloseHandle(hThread);
	}

	closesocket(listenSocket);
	WSACleanup();
	return 0;
}