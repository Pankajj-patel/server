#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <tchar.h>
#include <thread>
#include <vector>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
using namespace std;
#pragma comment(lib,"ws2_32.lib")

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT 12345

/*

basic step to setup socket programming in c++

//Initalize winsock lib
//create socket
//get IP and Port
//bind IP and port with Socket
//Listen on socket
//accept
//cleanup the winsock

*/

//-----------------------------
// Creating Initialize function
bool Initialize()
{
	WSADATA data;
	return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

void handleClient(SOCKET ClientSocket , vector<SOCKET>&Clients,int ClientNO)
{
	std::cout << "Client No : " << ClientNO << " Client Connected \n";
	char buffer[DEFAULT_BUFLEN];
	while (TRUE)
	{
		//Receive
		int bytesReceived = recv(ClientSocket, buffer, sizeof(buffer), 0);
		if (bytesReceived <= 0)
		{
			std::cout << "Client No :" << ClientNO << "Client DIsconnected \n";
			break;
		}
		string message(buffer, bytesReceived);
		std::cout <<"Client No :" << ClientNO << " Message : " << message << endl;
		for (auto Client : Clients)
		{
			if (Client != ClientSocket)
			{
				send(Client, message.c_str(), message.length(), 0);
			}
		}
	}
	auto it = find(Clients.begin(), Clients.end(), ClientSocket);
	if (it != Clients.end())
	{
		Clients.erase(it);
	}
	closesocket(ClientSocket);
}


int main()
{

	//Declare and Intialize variables

	int IResult;
	SOCKET Client = INVALID_SOCKET;

	//calling Initialize function
	IResult = Initialize();

	if (!IResult)
	{
		std::cout << "Winsock Initialization Failed" << "\n";
	}

	SOCKET Server = socket(AF_INET, SOCK_STREAM, 0);
	if (Server == INVALID_SOCKET)
	{
		std::cout << "Socket Creation Failed" << "\n";
		return 1;
	}

	//create server adress structure;
	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(DEFAULT_PORT);
	//serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//convert ip (0.0.0.0) put it inside sin_family in binary format
	IResult = InetPton(AF_INET, _T("0.0.0.0"), &serveraddr.sin_addr);

	//create an Connected Client Adress Structure


	if(!IResult) {
		cout << "setting address structure failed" << "\n";
		closesocket(Server);
		WSACleanup();
		return 1;
	}

	//bind
	IResult = bind(Server, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr));
	if(IResult == SOCKET_ERROR)
	{
		cout << " bind failed" << "\n";
		closesocket(Server);
		WSACleanup();
		return 1;
	}

	//Listining 
	IResult = listen(Server, SOMAXCONN);
	if(IResult == SOCKET_ERROR)
	{
		std::cout << " Listen failed" << "\n";
		closesocket(Server);
		WSACleanup();
		return 1;
	}
	std::cout << " Server has Started Listening on port :" << DEFAULT_PORT << endl;
	
	vector<SOCKET> Clients;
	while (TRUE)
	{
		//accept
		Client = accept(Server, nullptr, nullptr);// we can pass address structure ans its length to know client 
		if (Client == INVALID_SOCKET)
		{
			std::cout << " Invalid client socket" << "\n";
		}
		int ClientNO = 1;
		Clients.push_back(Client);
		thread t1(handleClient, Client , std::ref(Clients),ClientNO);
		t1.detach();
		ClientNO++;
	}	
	closesocket(Client);
	closesocket(Server);
	WSACleanup();
}