/*
 * ClientCode.hpp
 *
 *  Created on: Sep 1, 2024
 *      Author: Hussein Shehata
 */

#ifndef CLIENTCODE_HPP_
#define CLIENTCODE_HPP_

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <iostream>
#include <cstring>
#include "string"

#include <thread>
using namespace std;

void enableANSI();
void ClearAboveLine(void);
void SendNameToServer(SOCKET& ServerSocket);
void SendToServer(SOCKET ServerSocket, int MaxLength);
void ReceiveFromServer(SOCKET ServerSocket, int MaxLength);
int inet_ptonnn(int af, const char *src, void *dst);
void SetColor(int textColor);
int Run();

void SendToServerMessage(SOCKET ServerSocket, int MaxLength, string Message);




#endif /* CLIENTCODE_HPP_ */
