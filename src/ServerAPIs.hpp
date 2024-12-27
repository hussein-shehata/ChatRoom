/*
 * ServerAPIs.hpp
 *
 *  Created on: Sep 4, 2024
 *      Author: Hussein Shehata
 */

#ifndef SERVERAPIS_HPP_
#define SERVERAPIS_HPP_

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include <vector>
#include <cstring>

#include  "ClientClass.hpp"

int inet_ptonnn(int af, const char *src, void *dst);

void SendToClient(const SOCKET &ClientSocket,char* ToBeSentMessage, int Length, bool Broadcast, bool AllClients);
void SendToAllClients(char* ToBeSentMessage, int Length, bool Broadcast);

bool IsAMessageToServer(Client& CurrentClient);
void ReceiveFromClients();
void BroadcastMessage(void);

void SendMembersStatus(Client& CurrentClient);
void EstablishConnectionBetweenTwoClients(string FirstClientName, string SecondClientName);

void DeleteClient(Client& CurrentClient);

int  ServerRun();


#endif /* SERVERAPIS_HPP_ */
