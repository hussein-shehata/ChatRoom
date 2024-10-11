/*
 * ServerAPIs.cpp
 *
 *  Created on: Sep 4, 2024
 *      Author: Hussein Shehata
 */
#include "ServerAPIs.hpp"


using namespace std;

#define SERVER_NAME_LENGTH		6
static char ServerName [SERVER_NAME_LENGTH + 1]= "SERVER";

int MaxLength = 52000;

vector<Client> AcceptedClients;


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


static Client& GetClientByName(string Name)
{
  for (Client& CurrentClient : AcceptedClients)
    {
      if(CurrentClient.GetName() == Name)
	{
	  return CurrentClient;
	}
    }

}
static void  DeleteClient(Client& CurrentClient)
{
  cout<<"Will Delete Client with name "<<CurrentClient.GetName()<<endl;
  int ClientIndex = 0;
  for (Client& Client : AcceptedClients)
    {
      if(CurrentClient.GetName() == Client.GetName())
      	{
      	  // Delete Client as it will the desctructor which will close the connection
	  AcceptedClients.erase(AcceptedClients.begin() + ClientIndex);
      	}
      ClientIndex++;
    }
}


void SendToClient(const SOCKET &ClientSocket,char* ToBeSentMessage, int Length, bool Broadcast, bool AllClients)
{
  // NOTE the next 3 blocks are the same in calling the same api with the same parameters, either keep one or remeber why you made three blocks
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


		int ByteCount = send(ClientSocket, ToBeSentMessage, Length, 0);
		if(ByteCount > 0)
		{
//		  cout<<"Sent Successfully"<<endl;
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

//		cout<<"Sending to All Clients"<<endl;
		int ByteCount = send(ClientSocket, ToBeSentMessage, Length, 0);
	}
	else if(AllClients == false && Broadcast == false)
	{
		/// Giving a certain Client a message like warning for example
//		cout<<"Sending to the Client"<<endl;
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

bool IsAMessageToServer(Client& CurrentClient)
{

  unsigned char Flag = CurrentClient.ReceivedClientMessage.GetNewNameFlag();
  unsigned char RequestMemberUpdateFlag = CurrentClient.ReceivedClientMessage.GetRequestingMembersUpdate();
  unsigned char NotifyingExitingClient = CurrentClient.ReceivedClientMessage.GetExitFlag();

  char Buffer[MaxLength];
  if( (Flag & 0x01) == 1)
    {
      //then the client is sending his name only and there is no message
      ClientMessage ToBeSentMessage;
      ToBeSentMessage.SetName("SERVER");
      ToBeSentMessage.SetMessage("Please Welcome our new member : " + CurrentClient.ReceivedClientMessage.GetName());
      ToBeSentMessage.SetNotifyingNewMemberFlag(true);

      int Length = ToBeSentMessage.Serialize(Buffer);

      CurrentClient.SetValidClient(true);
      SendToAllClients(Buffer, Length, true);
      ToBeSentMessage.SetNotifyingNewMemberFlag(false);


      return true;

    }
  else if ( (RequestMemberUpdateFlag & 0x01) == 1)
    {
      SendMembersStatus(CurrentClient);
      return true;
    }

  else if ( (NotifyingExitingClient & 0x01) == 1)
    {
      DeleteClient(CurrentClient);
      // Send to all the Clients that the Client has left
      ClientMessage ToBeSentMessage;
      ToBeSentMessage.SetName("Server");
      ToBeSentMessage.SetMessage("The User : " + CurrentClient.ReceivedClientMessage.GetName() + " has left.");
      ToBeSentMessage.SetNotifyingNewMemberFlag(true);

      int Length = ToBeSentMessage.Serialize(Buffer);

      SendToAllClients(Buffer, Length, true);
      ToBeSentMessage.SetNotifyingNewMemberFlag(false);

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
      for (Client& CurrentClient: AcceptedClients)
	{
	  char Buffer[MaxLength];
	  SOCKET ClientSocket = CurrentClient.GetClientSocket();

	  setsockopt( ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)); //setting Timeout
	  int ByteCount = recv(ClientSocket, Buffer, MaxLength, 0);

	  if(ByteCount == SOCKET_ERROR)
	    {
	      if (WSAGetLastError() == WSAETIMEDOUT)
		{
		  // Timeout Happens so do nothing
		  continue;
		}
	      else
		{
		  cout<<"error in Receving the message from "<<CurrentClient.GetName()<<"with Error code "<<WSAGetLastError()<<endl;
		  WSACleanup();
		  return ;
		}
	    }
	  else
	    {
	      // Deserialize Incoming Message
	      CurrentClient.ReceivedClientMessage.Deserialize(Buffer);

	      //Check if it Message to Server
	      if(IsAMessageToServer(CurrentClient) == true)
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
		      // TODO : Till we handle the Validity of the client
		      //						CurrentClient.SetName(Buffer);
		      //						char WelcomingMessage[100] = "SERVER: Please Welcome our new member : ";
		      //						char ClientName[100];
		      //						CurrentClient.GetName(ClientName);
		      //						strcat(WelcomingMessage, ClientName);

		    }
		}
	      //		  continue;



	      bool DetectedOffensiveLanguage = false;
	      string ReceivedMessage = CurrentClient.ReceivedClientMessage.GetClientMessage();
	      int Length = CurrentClient.ReceivedClientMessage.GetLengthOfMessage();
	      //TODO if there is  Offensive language in the received message make the boolen value = true and send a warning to the client
	      if(ReceivedMessage[1] == 'Q')
		{
		  DetectedOffensiveLanguage = true;
		}
	      if(ByteCount > 0 && DetectedOffensiveLanguage == false)
		{
		  //	  cout<<Buffer<<endl;
		  if(ReceivedMessage[0] == '\n' || ReceivedMessage[0] == '\0' || (ReceivedMessage[0] == ' '&& ReceivedMessage[1] == '\0') )
		    {
		      continue;
		    }
		  else
		    {
		      unsigned char Flag = CurrentClient.ReceivedClientMessage.GetPrivateMessageFlag();
		      if( (Flag & 0x01) == 1)
			{
			  // This is Private Message
			  SOCKET ReceivingSocket = GetClientByName(CurrentClient.ReceivedClientMessage.GetRecevingEndName()) .GetClientSocket() ;
			  SendToClient(ReceivingSocket, Buffer, Length, false, false);
			}
		      else
			{
			  // This message is for all clients
			  SendToAllClients(Buffer, Length, false);
			}
		    }
		}
	      else if(ByteCount > 0 && DetectedOffensiveLanguage == true)
		{
		  ClientMessage NotificationMessage;
		  NotificationMessage.SetName("SERVER");
		  char WarningMessage[300];
		  //TODO Send a warning message
		  if(CurrentClient.GetNumberOfWarnings() >= 3)
		    {
		      //Remove the User as he exceeded the number of available warnings
		      CurrentClient.SetValidClient(false);

		      NotificationMessage.SetMessage("You can not send any message now as you have been blocked for saying too many offensive language.");

		      int Length = NotificationMessage.Serialize(WarningMessage);
		      SendToClient(ClientSocket, WarningMessage, Length, false, false);

		      string NotifyingMessageToOtherClients = "Notification : " + CurrentClient.ReceivedClientMessage.GetName() + " has been blocked for saying too many offensive words.";
		      NotificationMessage.SetMessage(NotifyingMessageToOtherClients);
		      Length = NotificationMessage.Serialize(WarningMessage);

		      SendToAllClients(WarningMessage, Length, true);
		    }
		  else if(CurrentClient.GetNumberOfWarnings() == 2)
		    {
		      // More than 2 warnings, then send to him different message
		      NotificationMessage.SetMessage("Last Warning !!! you have said an offensive, and Final Warning, one more time and you will be blocked");
		      int Length = NotificationMessage.Serialize(WarningMessage);
		      SendToClient(ClientSocket, WarningMessage, Length, false, false);
		    }
		  else
		    {
		      NotificationMessage.SetMessage(" Warning !!! you have said an offensive, your message wont be sent to the other members");
		      int Length = NotificationMessage.Serialize(WarningMessage);

		      SendToClient(ClientSocket, WarningMessage, Length, false, false);
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
	ClientMessage ToBeSentMessage;
	ToBeSentMessage.SetName("SERVER");
	int Length = 0;
  while(1)
    {
      char Buffer[MaxLength];
      string ServerMessage;
      cout<<"Enter a BroadcastMessage"<<endl;
      cin>>ServerMessage;
	  ToBeSentMessage.SetMessage(ServerMessage);
	  Length = ToBeSentMessage.Serialize(Buffer);

//      cout<<"Will Send the broadcast now"<<endl;
      SendToAllClients(Buffer, Length, true);
    }

}



void EstablishConnectionBetweenTwoClients(string FirstClientName, string SecondClientName)
{
  Client& RequestingClient = GetClientByName(FirstClientName);
  if( RequestingClient.CheckIfFriends(SecondClientName) == true)
    {
      // Already Friends so there is no need to establish connection
    }
  else
    {

    }

}
// TODO make the status enums and may be handle the function more properly
// TODO make the server send the number of clients first so the client can handle them properly, can do it with flag
// TODO make that we dont send the name of the disconnected members
void SendMembersStatus(Client& RequesterClient)
{
  ClientMessage ToBeSentMessage;
  ToBeSentMessage.SetRequestingMembersUpdate(true);
  int Length = 0;
  char Buffer[MaxLength] = " ";

  for (Client& CurrentClient: AcceptedClients)
    {
      ToBeSentMessage.SetName(CurrentClient.GetName());
      //Giving the status of each client connected to the server
      unsigned char MemberStatus = CurrentClient.GetValidClient();
      if(MemberStatus == false)
	{
	  ToBeSentMessage.SetMessage("Blocked");
	}
      else if(MemberStatus == true)
      	{
      	  ToBeSentMessage.SetMessage("Accepted");
      	}

      Length = ToBeSentMessage.Serialize(Buffer);
      SendToClient(RequesterClient.GetClientSocket(), Buffer, Length, false, false);
    }
  //Sending to client that we finished transmitting the names of client
  ToBeSentMessage.SetName("Server");
  ToBeSentMessage.SetMessage("EndOfClients");
  Length = ToBeSentMessage.Serialize(Buffer);
  SendToClient(RequesterClient.GetClientSocket(), Buffer, Length, false, false);
  ToBeSentMessage.SetRequestingMembersUpdate(false);

}


int  ServerRun()
{
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
