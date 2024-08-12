//============================================================================
// Name        : Chat-Room.cpp
// Author      : Hussein
// Version     :
// Copyright   : 
// Description : Hello World in C++, Ansi-style
//============================================================================

//#include "stdafx.h"
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <fstream>

#include <cstring>

#define SERVER_NAME_LENGTH		6
static char ServerName [SERVER_NAME_LENGTH + 1]= "SERVER";


using namespace std;

void ReadIPAddressAndPortNumber(string &IPAddress, int &PortNumber)
{
  ifstream infile("config.txt");

  if (infile.is_open()) {
      // Read IP address from the first line
      std::getline(infile, IPAddress);

      // Read port number from the second line
      std::string portStr;
      std::getline(infile, portStr);
      PortNumber = std::stoi(portStr); // Convert string to integer

      infile.close();

      // Output the IP address and port number
      cout << "IP Address: " << IPAddress << endl;
      cout << "Port Number: " << PortNumber << endl;
  }
  else
    {
      std::cerr << "Unable to open the file." << std::endl;
    }
}

int inet_ptonnn(int af, const char *src, void *dst)
{
  struct sockaddr_storage ss;
  int size = sizeof(ss);
  char src_copy[INET6_ADDRSTRLEN+1];

  ZeroMemory(&ss, sizeof(ss));
  /* stupid non-const API */
  strncpy (src_copy, src, INET6_ADDRSTRLEN+1);
  src_copy[INET6_ADDRSTRLEN] = 0;

  if (WSAStringToAddress(src_copy, af, NULL, (struct sockaddr *)&ss, &size) == 0) {
    switch(af) {
      case AF_INET:
    *(struct in_addr *)dst = ((struct sockaddr_in *)&ss)->sin_addr;
    return 1;
      case AF_INET6:
    *(struct in6_addr *)dst = ((struct sockaddr_in6 *)&ss)->sin6_addr;
    return 1;
    }
  }
  return 0;
}



void SendToClient(const SOCKET &ClientSocket, int MaxLength)
{
      char Buffer[MaxLength + SERVER_NAME_LENGTH];
      strcpy(Buffer, ServerName);
  while(1)
    {

      Buffer[SERVER_NAME_LENGTH ] = ' ';
      Buffer[SERVER_NAME_LENGTH + 1 ] = ':';
      Buffer[SERVER_NAME_LENGTH + 2 ] = ' ';

//      cout<<"Enter ur message to the client"<<endl;
      cin.getline( &(Buffer[SERVER_NAME_LENGTH + 3] ) ,MaxLength);


      int ByteCount = send(ClientSocket, Buffer, MaxLength + 100, 0);

      if(ByteCount > 0)
	{
//	  cout<<"Sent Successfully"<<endl;
	}
      else
	{
	  cout<<"Send Failed"<<endl;
	  WSACleanup();
	}
    }
}


// We have to make it constant or not using reference at all
void ReceiveFromClient(const SOCKET &ClientSocket, int MaxLength) //TODO we can make a parameter to receive the incoming string if we want to
{
  while(1)
    {

      char Buffer[MaxLength + 100];
      int ByteCount = recv(ClientSocket, Buffer, MaxLength + 100, 0);

      if(ByteCount > 0)
	{
	  cout<<Buffer<<endl;
	}
      else
	{
//	  cout<<"Receiving Failed"<<endl;
	  WSACleanup();
	}
    }
}



void SetColor(int textColor)
{
    cout << "\033[" << textColor << "m";
}


int main() {
  int TestPort;
  string TestStr;

  ReadIPAddressAndPortNumber(TestStr, TestPort);
  SOCKET 	serverSocket, acceptSocket;
  int port = 55555;
  WSADATA wsaData;
  int wsaerr;
  WORD wVersionRequested = MAKEWORD(2,2);
  wsaerr = WSAStartup(wVersionRequested, &wsaData);
  if(wsaerr != 0)
    {
      cout<<"The Winsock dll not found"<<endl;
      return 0;
    }
  else
    {
      cout<<"The Winsock dll found"<<endl;
      cout<<"The status " << wsaData.szSystemStatus<<endl;
    }



  serverSocket = INVALID_SOCKET;
  serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(serverSocket == INVALID_SOCKET)
    {
      cout<<"Error at socket() : "<<WSAGetLastError()<<endl;
      WSACleanup();
      return 0;
    }
  else
    {
      cout<< "Socket is OK !"<<endl;
    }


  sockaddr_in service;
  service.sin_family = AF_INET;
//  inet_pton()
  service.sin_port = htons(port);
  inet_ptonnn(AF_INET, ("127.0.0.1"), &service.sin_addr); //TODO most likely this will cause issue

  if( bind(serverSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
    {
      cout<<"Bind() failed : " <<WSAGetLastError()<<endl;
      closesocket(serverSocket);
      WSACleanup();
      return 0;
    }
  else
    {
      cout<<"Bind() is OK!"<<endl;
    }


  if(listen(serverSocket, 1) == SOCKET_ERROR)
    {
      cout<<"Listen(): error "<<WSAGetLastError()<<endl;
    }
  else
    {
      cout<<"Listen() is OK, I am waiting for connections..."<<endl;

    }


  acceptSocket = accept(serverSocket, NULL, NULL);
  if(acceptSocket == INVALID_SOCKET)
    {
      cout<<"accept Failed: "<<WSAGetLastError()<<endl;
      WSACleanup();
      return -1;
    }
  cout<<"Accepted connection"<<endl;


  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(hConsole, 12);
  thread Worker1(SendToClient, acceptSocket, 300);
  thread Worker2(ReceiveFromClient,acceptSocket, 300);

  while(1)
    {
      acceptSocket = accept(serverSocket, NULL, NULL);
      if(acceptSocket == INVALID_SOCKET)
        {
          cout<<"accept Failed: "<<WSAGetLastError()<<endl;
          WSACleanup();
          return -1;
        }
      cout<<"Accepted connection"<<endl;
    }
//  SendToClient(acceptSocket, 200);
//  ReceiveFromClient(acceptSocket, 200);

  Worker1.join();
  Worker2.join();


  system("pause");
  WSACleanup();

  return 0;
}
