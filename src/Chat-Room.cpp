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
#include <vector>
#include <cstring>


using namespace std;

#define SERVER_NAME_LENGTH		6
static char ServerName [SERVER_NAME_LENGTH + 1]= "SERVER";

int MaxLength = 200;

vector<SOCKET> AcceptedClientsSockets;




/******************** Helper Functio for debugging *****************/
void PrintMessage(char* Message)
{
  int i = 0;
  while( Message[i] != 0 && Message[i] !='\n')
    {
      cout<<Message[i];
      i++;
    }
  cout<<endl;
}










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




void SendToClient(const SOCKET &ClientSocket,char* ToBeSentMessage, int Length, bool Broadcast, bool AllClients)
{
  if(Broadcast == true)
    {
      /// Broadcasting a server message like giving a maintenance warning
      char Buffer[Length + SERVER_NAME_LENGTH];
      strcpy(Buffer, ServerName);


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
  else if(AllClients == true && Broadcast == false)
    {
      /// Just Trasmitting the client message to all the clients

      int ByteCount = send(ClientSocket, ToBeSentMessage, Length, 0);
    }
  else if(AllClients == false && Broadcast == false)
    {
      /// Giving a certain Client a message like warning for example

      int ByteCount = send(ClientSocket, ToBeSentMessage, Length, 0);
    }
  else
    {

    }
}


void SendToAllClients(char* ToBeSentMessage, int Length, bool Broadcast)
{
  // Iterating at all the Client Sockets
  for (SOCKET& CurrentClientSocket: AcceptedClientsSockets)
    {
      SendToClient(CurrentClientSocket, ToBeSentMessage, Length, Broadcast, true);
    }
}

// We have to make it constant or not using reference at all
void ReceiveFromClients() //TODO we can make a parameter to receive the incoming string if we want to
{
  DWORD timeout = 100;
  while(1)
    {
      for (SOCKET& CurrentClientSocket: AcceptedClientsSockets)
	{
	  char Buffer[MaxLength];

	  setsockopt(CurrentClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)); //setting Timeout
	  int ByteCount = recv(CurrentClientSocket, Buffer, MaxLength, 0);

	  if(ByteCount == SOCKET_ERROR)
	    {
	      if (WSAGetLastError() == WSAETIMEDOUT)
		{
		  // Timeout Happens so do nothing
//		  cout<<"Server didnot receve anything"<<endl;
		  continue;
		}
	      else
		{
		  cout<<"error in Receving the message"<<endl;
		  WSACleanup();
		  return ;
		}
	    }
	  else{

	      bool DetectedOffensiveLanguage = false;

	      //TODO if there is  Offensive language in the received message make the boolen value = true and send a warning to the client
	      if(Buffer[15] == 'Q')
		{
		  DetectedOffensiveLanguage = true;
		}
	      if(ByteCount > 0 && DetectedOffensiveLanguage == false)
		{
		  //	  cout<<Buffer<<endl;
		  SendToAllClients(Buffer, MaxLength, false);
		}
	      else if(ByteCount > 0 && DetectedOffensiveLanguage == true)
		{
		  //TODO Send a warning message
		  char WarningMessage[MaxLength] = "Server  :::::  Warning !!! you have said an offensive, your message wont be sent to the other members \0";
		  SendToClient(CurrentClientSocket, WarningMessage, MaxLength, false, false);
		}
	      else
		{
		  //	  cout<<"Receiving Failed"<<endl;
		  WSACleanup();
		}
	  }
	}
    }
}


// TODO make it on an another thread but only wake ups every 1 min for now
void BroadcastMessage(void)
{
  char Buffer[MaxLength + SERVER_NAME_LENGTH];
  strcpy(Buffer, ServerName);


  Buffer[SERVER_NAME_LENGTH ] = ' ';
  Buffer[SERVER_NAME_LENGTH + 1 ] = ':';
  Buffer[SERVER_NAME_LENGTH + 2 ] = ' ';
  cout<<"Enter a BroadcastMessage"<<endl;

  //      cout<<"Enter ur message to the client"<<endl;
  cin.getline( &(Buffer[SERVER_NAME_LENGTH + 3] ) ,MaxLength);

  SendToAllClients(Buffer, MaxLength + SERVER_NAME_LENGTH, true);

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

  AcceptedClientsSockets.push_back(acceptSocket);

  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(hConsole, 12);


//  thread Worker1(SendToClient, acceptSocket, 300);
  thread Worker2(ReceiveFromClients);

  while(1)
    {
      acceptSocket = accept(serverSocket, NULL, NULL);
      if(acceptSocket == INVALID_SOCKET)
        {
          cout<<"accept Failed: "<<WSAGetLastError()<<endl;
          WSACleanup();
          return -1;
        }
      AcceptedClientsSockets.push_back(acceptSocket);
      cout<<"Accepted connection"<<endl;
    }
//  SendToClient(acceptSocket, 200);
//  ReceiveFromClient(acceptSocket, 200);

//  Worker1.join();
  Worker2.join();


  system("pause");
  WSACleanup();

  return 0;
}
