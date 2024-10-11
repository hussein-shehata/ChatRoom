/*
 * ClientClass.cpp
 *
 *  Created on: Aug 13, 2024
 *      Author: Hussein Shehata
 */

#include "ClientClass.hpp"
using namespace std;

Client::~Client()
{
  // closesocket(ClientSocket);
}
string Client::GetName()
{
  return ReceivedClientMessage.GetName();
}
void Client::PrintName(void)
{
//  if(Name[0] == 0)
//      {
//        //Do not have a name yet
//        ValidClient = false;
//        cout<<"This Client Does not have a name yet"<<endl;
//      }
//  else
//    {
//      cout<<Name;
//    }

}

int Client::SetName(char* RecevedName)
{
//  int idx = 0;
//  while(RecevedName[idx] != '\0' && RecevedName[idx] != '\n' && RecevedName[idx] != ' ')
//    {
//      idx++;
//    }
//  memcpy(Name, RecevedName, ((idx) * sizeof(char) )  );
//  Name[idx] = '\0';
//  ValidClient = true;
//
//  return (idx+1);
}

void Client::SetClientSocket( SOCKET* Socket)
{
//  cout<<"*************The Setter the socket is ";
  ClientSocket = *Socket;
//  cout<<ClientSocket<<endl;
}


void Client::IncrementNumberOfWarnings(void)
{
  NumberOfWarnings++;
}


int Client::GetNumberOfWarnings(void)
{
  return NumberOfWarnings;
}


bool Client::GetValidClient()
{
  return ValidClient;// TODO till we handle the validity and link it to input email not a name
//	return true;
}

void Client::SetValidClient(bool Flag)
{
  ValidClient = Flag;
}

SOCKET& Client::GetClientSocket()
{
//  cout<<"From inside the Getter";
//  cout<<"Socket is "<<ClientSocket<<endl;
  return ClientSocket;
}

bool Client::CheckIfFriends(string Name)
{
  for (const string& CurrentFriend : FriendList )
    {
      if(CurrentFriend == Name)
	{
	  return true;
	}
    }
  return false;
}

void Client::AddFriend(string Name)
{
  FriendList.push_back(Name);
}
