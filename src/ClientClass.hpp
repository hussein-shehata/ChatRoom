/*
 * ClientClass.hpp
 *
 *  Created on: Aug 13, 2024
 *      Author: Hussein Shehata
 */

#ifndef CLIENTCLASS_HPP_
#define CLIENTCLASS_HPP_

#include <winsock2.h>
#include <iostream>
#include "ClientMessageClass.hpp"
#include <vector>

class Client{
  private:

//    string ClientName[100]="";
    int NumberOfWarnings = 0;
    SOCKET ClientSocket = 0;
    bool ValidClient = false;

    vector<string> FriendList;


  public:
    ClientMessage ReceivedClientMessage;//TODO make it private with getter
    Flags MessageFlags; //TODO make it private with getter
    string GetName();
    int SetName(char* RecevedName);
    void PrintName(void);

    void SetClientSocket( SOCKET* Socket);
    SOCKET& GetClientSocket();
    void IncrementNumberOfWarnings(void);
    int GetNumberOfWarnings(void);

    bool GetValidClient();
    void SetValidClient(bool Flag);

    bool CheckIfFriends(string Name);
    void AddFriend(string Name);

};

#endif /* CLIENTCLASS_HPP_ */
