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

class Client{
  private:
    char Name[100]= {0};
//    char Buffer[100]="";
    int NumberOfWarnings = 0;
    SOCKET ClientSocket = 0;
    bool ValidClient = false;

  public:
    int GetName(char* RecevingBuffer);
    int SetName(char* RecevedName);
    void PrintName(void);

    void SetClientSocket( SOCKET* Socket);
    SOCKET& GetClientSocket();
    void IncrementNumberOfWarnings(void);
    int GetNumberOfWarnings(void);

    bool GetValidClient();
    void SetValidClient(bool Flag);

};

#endif /* CLIENTCLASS_HPP_ */
