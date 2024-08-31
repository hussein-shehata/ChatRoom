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

#include  "ClientClass.hpp"


using namespace std;

#define SERVER_NAME_LENGTH		6
static char ServerName [SERVER_NAME_LENGTH + 1]= "SERVER";

int MaxLength = 200;

vector<SOCKET> AcceptedClientsSockets;
vector<Client> AcceptedClients;




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
//      char Buffer[Length + SERVER_NAME_LENGTH];
//      strcpy(Buffer, ServerName);
//
//
//      Buffer[SERVER_NAME_LENGTH ] = ' ';
//      Buffer[SERVER_NAME_LENGTH + 1 ] = ':';
//      Buffer[SERVER_NAME_LENGTH + 2 ] = ' ';
//
//      //      cout<<"Enter ur message to the client"<<endl;
//      cin.getline( &(Buffer[SERVER_NAME_LENGTH + 3] ) ,MaxLength);


      int ByteCount = send(ClientSocket, ToBeSentMessage, MaxLength + 100, 0);

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
  //TODO Put Mutex here so the broadcast message wont interfere with the ongoing sending messages if there any
  for (Client& CurrentClient: AcceptedClients)
    {
      if(CurrentClient.GetValidClient() == false)
	{
	  // Dont Send any messages for a blocked client or clients whom didnt enter their name yet
	  continue;
	}

      SOCKET CurrentClientSocket = CurrentClient.GetClientSocket();
      SendToClient(CurrentClientSocket, ToBeSentMessage, Length, Broadcast, true);
    }
}

bool IsAMessageToServer(Client& CurrentClient,char* Buffer)
{
  if(Buffer[0] =='#' && Buffer[1] =='!' && Buffer[2] =='#' && Buffer[3] =='!')
    {
      //then the client give me his name
      CurrentClient.SetName(&Buffer[4]);
      char WelcomingMessage[100] = "SERVER: Please Welcome our new member : ";
      char ClientName[100];
      CurrentClient.GetName(ClientName);
      strcat(WelcomingMessage, ClientName);
      SendToAllClients(WelcomingMessage, MaxLength, true);

      return true;
    }
  return false;

}


// We have to make it constant or not using reference at all
void ReceiveFromClients() //TODO we can make a parameter to receive the incoming string if we want to
{
  DWORD timeout = 100;
  while(1)
    {
//      int idx = 0;
      for (Client& CurrentClient: AcceptedClients)
	{
	  char Buffer[MaxLength];
//	  cout<<"We have Client "<<idx<<endl;
//	  idx++;
//	  cout<<"we got the socket which is ";
	  SOCKET ClientSocket = CurrentClient.GetClientSocket();
//	  cout<<ClientSocket<<endl;


	  setsockopt( ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)); //setting Timeout
	  int ByteCount = recv(ClientSocket, Buffer, MaxLength, 0);

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
	      //Check if it Message to Server
	      //	      cout<<"Server receve something"<<endl;
	      if(IsAMessageToServer(CurrentClient, Buffer) == true)
		{
		  continue;
		}
	      //Check Validity of Client
	      if(CurrentClient.GetValidClient() == false && CurrentClient.GetNumberOfWarnings())
		{
		  if(CurrentClient.GetNumberOfWarnings() > 3)
		    {
		      //Block Client
		      continue;
		    }

		  else
		    {
		      CurrentClient.SetName(Buffer);
		      char WelcomingMessage[100] = "SERVER: Please Welcome our new member : ";
		      char ClientName[100];
		      CurrentClient.GetName(ClientName);
		      strcat(WelcomingMessage, ClientName);

		    }
		}
	      //		  continue;



	      bool DetectedOffensiveLanguage = false;

	      //TODO if there is  Offensive language in the received message make the boolen value = true and send a warning to the client
	      if(Buffer[15] == 'Q')
		{
		  DetectedOffensiveLanguage = true;
		}
	      if(ByteCount > 0 && DetectedOffensiveLanguage == false)
		{
		  //	  cout<<Buffer<<endl;
		  if(Buffer[0] == '\n' || Buffer[0] == '\0' || (Buffer[0] == ' '&& Buffer[1] == '\0') )
		    {
		      continue;
		    }
		  else
		    {
		      SendToAllClients(Buffer, MaxLength, false);
		    }
		}
	      else if(ByteCount > 0 && DetectedOffensiveLanguage == true)
		{
		  //TODO Send a warning message
		  if(CurrentClient.GetNumberOfWarnings() >= 3)
		    {
		      //Remove the User as he exceeded the number of available warnings
		      CurrentClient.SetValidClient(false);
		      char WarningMessage[MaxLength] = "Server ::::: You can not send any message now as you have been blocked for saying too many offensive language. \0";
		      SendToClient(ClientSocket, WarningMessage, MaxLength, false, false);

		      strcpy(WarningMessage, "SERVER::: Notifcation : ");
		      char ClientName [100] ;
		      int length = CurrentClient.GetName(ClientName);

		      strcat(WarningMessage, ClientName);

		      strcat(WarningMessage, " has been blocked for saying too many offensive words");

		      SendToAllClients(WarningMessage, MaxLength, true);
		    }
		  else if(CurrentClient.GetNumberOfWarnings() == 2)
		    {
		      // More than 2 warnings, then send to him different message
		      char WarningMessage[MaxLength] = "Server ::::: Last Warning !!! you have said an offensive, and Final Warning, one more time and you will be blocked \0";
		      SendToClient(ClientSocket, WarningMessage, MaxLength, false, false);
		    }
		  else
		    {
		      char WarningMessage[MaxLength] = "Server  :::::  Warning !!! you have said an offensive, your message wont be sent to the other members \0";
		      SendToClient(ClientSocket, WarningMessage, MaxLength, false, false);
		    }

		  CurrentClient.IncrementNumberOfWarnings();
		}
	      else
		{
		  //	  cout<<"Receiving Failed"<<endl;
		  WSACleanup();
		}
	  }
	}
//      cout<<"Finished Looping through all clients"<<endl;
    }
}


// TODO make it on an another thread but only wake ups every 1 min for now
void BroadcastMessage(void)
{
  while(1)
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

}

void SetColor(int textColor)
{
    cout << "\033[" << textColor << "m";
}


void signalHandler(int signal) {
    std::cout << "Segmentation fault (signal " << signal << ") caught!" << std::endl;
    exit(signal);  // Terminate the program after handling
}

int main() {
  signal(SIGSEGV, signalHandler);  // Register the signal handler
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

  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(hConsole, 12);


  acceptSocket = accept(serverSocket, NULL, NULL);

  if(acceptSocket == INVALID_SOCKET)
    {
      cout<<"accept Failed: "<<WSAGetLastError()<<endl;
      WSACleanup();
      return -1;
    }
  cout<<"Accepted connection"<<endl;

  Client Temp;
  Temp.SetClientSocket(&acceptSocket);
  AcceptedClients.push_back(Temp);

  thread Worker2(ReceiveFromClients);
  thread Worker1(BroadcastMessage);




//  thread Worker1(SendToClient, acceptSocket, 300);


  SOCKET TempSocket = serverSocket;
  while(1)
    {
//      cout<<"Before Finding anyy connection"<<endl;
      acceptSocket = accept(serverSocket, NULL, NULL);
//      cout<<"Found New Connection"<<endl;
      if(acceptSocket == INVALID_SOCKET)
        {
          cout<<"accept Failed: "<<WSAGetLastError()<<endl;
          acceptSocket = 0;
          serverSocket = TempSocket;
          continue;
//          WSACleanup();
          //          return -1;
        }
      else
	{
	  //      AcceptedClientsSockets.push_back(acceptSocket);

	  Temp.SetClientSocket(&acceptSocket);
	  AcceptedClients.push_back(Temp);
	}
      cout<<"Accepted connection"<<endl;
    }
//  SendToClient(acceptSocket, 200);
//  ReceiveFromClient(acceptSocket, 200);

//  Worker1.join();
  Worker2.join();

  cout<<"****************************Finished***************"<<endl;
  system("pause");
  WSACleanup();

  return 0;
}
