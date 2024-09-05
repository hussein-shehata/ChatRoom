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
#include "ServerAPIs.hpp"


using namespace std;





/******************** Helper Function for debugging *****************/
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

void SetColor(int textColor)
{
    cout << "\033[" << textColor << "m";
}


void signalHandler(int signal) {
    std::cout << "Segmentation fault (signal " << signal << ") caught!" << std::endl;
    exit(signal);  // Terminate the program after handling
}

int main() {
  ServerRun();

  return 0;
}
