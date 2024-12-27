/*
 * ServerAPIs.cpp
 *
 *  Created on: Sep 4, 2024
 *      Author: Hussein Shehata
 */
#include "ServerAPIs.hpp"

using namespace std;

#define SERVER_NAME_LENGTH 6
static char ServerName[SERVER_NAME_LENGTH + 1] = "SERVER";

int MaxLength = 52000;

vector<Client> AcceptedClients;

int inet_ptonnn(int af, const char *src, void *dst)
{
  struct sockaddr_storage ss;
  int size = sizeof(ss);
  char src_copy[INET6_ADDRSTRLEN + 1];

  ZeroMemory(&ss, sizeof(ss));
  /* stupid non-const API */
  strncpy(src_copy, src, INET6_ADDRSTRLEN + 1);
  src_copy[INET6_ADDRSTRLEN] = 0;

  if (WSAStringToAddress(src_copy, af, NULL, (struct sockaddr *)&ss, &size) == 0)
  {
    switch (af)
    {
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

static Client &GetClientByName(string Name)
{
  for (Client &CurrentClient : AcceptedClients)
  {
    if (CurrentClient.GetName() == Name)
    {
      return CurrentClient;
    }
  }
}
void DeleteClient(Client &CurrentClient)
{
  cout << "Will Delete Client with name " << CurrentClient.GetName() << endl;
  int ClientIndex = 0;
  // NOTE Just added may fix the problem which happens sometimes when disconnecting clients
  ClientMessage ToBeSentMessage;
  char Buffer[MaxLength];
  ToBeSentMessage.SetName("Server");
  ToBeSentMessage.SetMessage("#Deleted");
  ToBeSentMessage.SetExitFlag(true);
  int Length = ToBeSentMessage.Serialize(Buffer);
  // Handshake before deleting the client from the connected clients to make sure that the server delete him first.
  SendToClient(CurrentClient.GetClientSocket(), Buffer, Length, false, false);
  closesocket(CurrentClient.GetClientSocket());
  for (Client &Client : AcceptedClients)
  {
    if (CurrentClient.GetName() == Client.GetName())
    {
      // Delete Client as it will the desctructor which will close the connection
      AcceptedClients.erase(AcceptedClients.begin() + ClientIndex);
      break;
    }
    ClientIndex++;
  }
  cout << "The remaining Clients are " << endl;
  for (Client &Client : AcceptedClients)
  {
    cout << Client.GetName() << endl;
  }
}

void SendToClient(const SOCKET &ClientSocket, char *ToBeSentMessage, int Length, bool Broadcast, bool AllClients)
{
  // NOTE the next 3 blocks are the same in calling the same api with the same parameters, either keep one or remeber why you made three blocks
  if (Broadcast == true)
  {
    /// Broadcasting a server message like giving a maintenance warning

    int ByteCount = send(ClientSocket, ToBeSentMessage, Length, 0);
    if (ByteCount > 0)
    {
      cout << "Sent Successfully" << endl;
    }
    else
    {
      cout << "Send Failed with error code " << WSAGetLastError() << endl;
      WSACleanup();
    }
  }
  // TODO enhance this code
  else if (AllClients == true && Broadcast == false)
  {
    /// Just Trasmitting the client message to all the clients

    //		cout<<"Sending to All Clients"<<endl;
    int ByteCount = send(ClientSocket, ToBeSentMessage, Length, 0);
    if (ByteCount > 0)
    {
      cout << "Sent Successfully" << endl;
    }
    else
    {
      cout << "Send Failed with error code " << WSAGetLastError() << endl;
      WSACleanup();
    }
  }
  else if (AllClients == false && Broadcast == false)
  {
    /// Giving a certain Client a message like warning for example
    //		cout<<"Sending to the Client"<<endl;
    int ByteCount = send(ClientSocket, ToBeSentMessage, Length, 0);
    if (ByteCount > 0)
    {
      cout << "Sent Successfully delete msg" << endl;
    }
    else
    {
      cout << "Send Failed with error code " << WSAGetLastError() << endl;
      WSACleanup();
    }
  }
  else
  {
  }
}

void SendToAllClients(char *ToBeSentMessage, int Length, bool Broadcast)
{
  // Iterating at all the Client Sockets
  // TODO Put Mutex here so the broadcast message wont interfere with the ongoing sending messages if there any
  for (Client &CurrentClient : AcceptedClients)
  {
    if (CurrentClient.GetValidClient() == false)
    {
      // Dont Send any messages for a blocked client or clients whom didnt enter their name yet
      continue;
    }

    SOCKET CurrentClientSocket = CurrentClient.GetClientSocket();
    SendToClient(CurrentClientSocket, ToBeSentMessage, Length, Broadcast, true);
  }
}

bool IsAMessageToServer(Client &CurrentClient)
{

  unsigned char Flag = CurrentClient.ReceivedClientMessage.GetNewNameFlag();
  unsigned char RequestMemberUpdateFlag = CurrentClient.ReceivedClientMessage.GetRequestingMembersUpdate();
  unsigned char NotifyingExitingClient = CurrentClient.ReceivedClientMessage.GetExitFlag();

  char Buffer[MaxLength];
  if ((Flag & 0x01) == 1)
  {
    // then the client is sending his name only and there is no message
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
  else if ((RequestMemberUpdateFlag & 0x01) == 1)
  {
    SendMembersStatus(CurrentClient);
    return true;
  }

  else if ((NotifyingExitingClient & 0x01) == 1)
  {
    string ClientName = CurrentClient.GetName();
    DeleteClient(CurrentClient);
    // Send to all the Clients that the Client has left
    ClientMessage ToBeSentMessage;
    ToBeSentMessage.SetName("Server");
    ToBeSentMessage.SetMessage("The User : ' " + ClientName + " ' has left.");
    ToBeSentMessage.SetNotifyingNewMemberFlag(true);

    int Length = ToBeSentMessage.Serialize(Buffer);

    SendToAllClients(Buffer, Length, true);
    ToBeSentMessage.SetNotifyingNewMemberFlag(false);

    return true;
  }
  return false;
}

// We have to make it constant or not using reference at all
void ReceiveFromClients() // TODO we can make a parameter to receive the incoming string if we want to
{
  DWORD timeout = 100;
  char Buffer[MaxLength];
  int dummy;
  while (1)
  {
    for (Client &CurrentClient : AcceptedClients)
    {
      
      SOCKET ClientSocket = CurrentClient.GetClientSocket();
      Buffer[0] = 'H';

      int ByteCount = recv(ClientSocket, Buffer, MaxLength, 0);

      if (ByteCount <= 0)
      {
        if (WSAGetLastError() == WSAETIMEDOUT)
        {
          // Timeout Happens so do nothing
          continue;
        }
        else
        {
          cout<<"First char is "<<Buffer[0]<<endl;
          cout << "error in Receving the message from " << CurrentClient.GetName() << " with Error code " << WSAGetLastError() << endl;
          cout<< " with socket number : "<<ClientSocket<<endl;
          cin>>dummy;
          // WSACleanup();
          continue;
        }
      }

      // Deserialize Incoming Message
      CurrentClient.ReceivedClientMessage.Deserialize(Buffer);

      // Check if it Message to Server
      if (IsAMessageToServer(CurrentClient) == true)
      {
        continue;
      }
      // Check Validity of Client
      if (CurrentClient.GetValidClient() == false && CurrentClient.GetNumberOfWarnings())
      {
        if (CurrentClient.GetNumberOfWarnings() > 3)
        {
          // Block Client
          continue;
        }

        else
        {

        }
      }
      //		  continue;

      bool DetectedOffensiveLanguage = false;
      string ReceivedMessage = CurrentClient.ReceivedClientMessage.GetClientMessage();
      int Length = CurrentClient.ReceivedClientMessage.GetLengthOfMessage();
      // TODO if there is  Offensive language in the received message make the boolen value = true and send a warning to the client
      if (ReceivedMessage[1] == 'Q')
      {
        DetectedOffensiveLanguage = true;
      }
      if (ByteCount > 0 && DetectedOffensiveLanguage == false)
      {
        if (ReceivedMessage[0] == '\n' || ReceivedMessage[0] == '\0' || (ReceivedMessage[0] == ' ' && ReceivedMessage[1] == '\0'))
        {
          continue;
        }
        else
        {
          unsigned char Flag = CurrentClient.ReceivedClientMessage.GetPrivateMessageFlag();
          if ((Flag & 0x01) == 1)
          {
            // This is Private Message
            SOCKET ReceivingSocket = GetClientByName(CurrentClient.ReceivedClientMessage.GetRecevingEndName()).GetClientSocket();
            SendToClient(ReceivingSocket, Buffer, Length, false, false);
          }
          else
          {
            // This message is for all clients
            SendToAllClients(Buffer, Length, false);
          }
        }
      }
      else if (ByteCount > 0 && DetectedOffensiveLanguage == true)
      {
        ClientMessage NotificationMessage;
        NotificationMessage.SetName("SERVER");
        char WarningMessage[300];
        // TODO Send a warning message
        if (CurrentClient.GetNumberOfWarnings() >= 3)
        {
          // Remove the User as he exceeded the number of available warnings
          CurrentClient.SetValidClient(false);

          NotificationMessage.SetMessage("You can not send any message now as you have been blocked for saying too many offensive language.");

          int Length = NotificationMessage.Serialize(WarningMessage);
          SendToClient(ClientSocket, WarningMessage, Length, false, false);

          string NotifyingMessageToOtherClients = "Notification : " + CurrentClient.ReceivedClientMessage.GetName() + " has been blocked for saying too many offensive words.";
          NotificationMessage.SetMessage(NotifyingMessageToOtherClients);
          Length = NotificationMessage.Serialize(WarningMessage);

          SendToAllClients(WarningMessage, Length, true);
        }
        else if (CurrentClient.GetNumberOfWarnings() == 2)
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
        // NOTE I commented the below line to see if it is the issue of our program
        // WSACleanup();
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
  while (1)
  {
    char Buffer[MaxLength];
    string ServerMessage;
    cout << "Enter a BroadcastMessage" << endl;
    cin >> ServerMessage;
    ToBeSentMessage.SetMessage(ServerMessage);
    Length = ToBeSentMessage.Serialize(Buffer);

    //      cout<<"Will Send the broadcast now"<<endl;
    SendToAllClients(Buffer, Length, true);
  }
}

void EstablishConnectionBetweenTwoClients(string FirstClientName, string SecondClientName)
{
  Client &RequestingClient = GetClientByName(FirstClientName);
  if (RequestingClient.CheckIfFriends(SecondClientName) == true)
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
void SendMembersStatus(Client &RequesterClient)
{
  ClientMessage ToBeSentMessage;
  ToBeSentMessage.SetRequestingMembersUpdate(true);
  int Length = 0;
  char Buffer[MaxLength] = " ";

  for (Client &CurrentClient : AcceptedClients)
  {
    ToBeSentMessage.SetName(CurrentClient.GetName());
    // Giving the status of each client connected to the server
    unsigned char MemberStatus = CurrentClient.GetValidClient();
    if (MemberStatus == false)
    {
      ToBeSentMessage.SetMessage("Blocked");
    }
    else if (MemberStatus == true)
    {
      ToBeSentMessage.SetMessage("Accepted");
    }

    Length = ToBeSentMessage.Serialize(Buffer);
    SendToClient(RequesterClient.GetClientSocket(), Buffer, Length, false, false);
  }
  // Sending to client that we finished transmitting the names of client
  ToBeSentMessage.SetName("Server");
  ToBeSentMessage.SetMessage("EndOfClients");
  Length = ToBeSentMessage.Serialize(Buffer);
  SendToClient(RequesterClient.GetClientSocket(), Buffer, Length, false, false);
  ToBeSentMessage.SetRequestingMembersUpdate(false);
}

int InitComm(SOCKET& serverSocket)
{
int port = 55555;
  WSADATA wsaData;
  int wsaerr;
  WORD wVersionRequested = MAKEWORD(2, 2);
  wsaerr = WSAStartup(wVersionRequested, &wsaData);
  if (wsaerr != 0)
  {
    cout << "The Winsock dll not found" << endl;
    return -1;
  }
  else
  {
    cout << "The Winsock dll found" << endl;
    cout << "The status " << wsaData.szSystemStatus << endl;
  }

  serverSocket = INVALID_SOCKET;
  serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (serverSocket == INVALID_SOCKET)
  {
    cout << "Error at socket() : " << WSAGetLastError() << endl;
    WSACleanup();
    return -1;
  }
  else
  {
    cout << "Socket is OK !" << endl;
  }

  sockaddr_in service;
  service.sin_family = AF_INET;
  //  inet_pton()
  service.sin_port = htons(port);
  inet_ptonnn(AF_INET, ("127.0.0.1"), &service.sin_addr); // TODO most likely this will cause issue

  if (bind(serverSocket, (SOCKADDR *)&service, sizeof(service)) == SOCKET_ERROR)
  {
    cout << "Bind() failed : " << WSAGetLastError() << endl;
    closesocket(serverSocket);
    WSACleanup();
    return -1;
  }
  else
  {
    cout << "Bind() is OK!" << endl;
  }

  if (listen(serverSocket, 1) == SOCKET_ERROR)
  {
    cout << "Listen(): error " << WSAGetLastError() << endl;
  }
  else
  {
    cout << "Listen() is OK, I am waiting for connections..." << endl;
  }

  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(hConsole, 12);
  return 0;
}

int ServerRun()
{
  SOCKET serverSocket, acceptSocket;
  if(InitComm(serverSocket) != 0)
  {
    return -1;
  }

  acceptSocket = accept(serverSocket, NULL, NULL);

  if (acceptSocket == INVALID_SOCKET)
  {
    cout << "accept Failed: " << WSAGetLastError() << endl;
    WSACleanup();
    return -1;
  }
  cout << "Accepted connection" << endl;
  DWORD timeout = 100;
  setsockopt(acceptSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)); // setting Timeout

  Client Temp;
  Temp.SetClientSocket(&acceptSocket);
  AcceptedClients.push_back(Temp);

  thread Worker2(ReceiveFromClients);
  thread Worker1(BroadcastMessage);

  while (1)
  {
    acceptSocket = accept(serverSocket, NULL, NULL);
    if (acceptSocket == INVALID_SOCKET)
    {
      Sleep(1000);
      cout << "accept Failed: " << WSAGetLastError() << endl;
      continue;
      //          WSACleanup();
      //          return -1;
    }
    
    //      AcceptedClientsSockets.push_back(acceptSocket);
    setsockopt(acceptSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)); // setting Timeout
    Temp.SetClientSocket(&acceptSocket);
    AcceptedClients.push_back(Temp);
    
    cout << "Accepted connection" << endl;
  }
  //  SendToClient(acceptSocket, 200);
  //  ReceiveFromClient(acceptSocket, 200);

  //  Worker1.join();
  Worker2.join();

  cout << "****************************Finished***************" << endl;
  system("pause");
  WSACleanup();

  return 0;
}
