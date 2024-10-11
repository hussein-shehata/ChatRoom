/*
 * ClientMessageClass.cpp
 *
 *  Created on: Sep 1, 2024
 *      Author: Hussein Shehata
 */


#include "ClientMessageClass.hpp"
using namespace std;
ClientMessage::ClientMessage():Name(""), Message(""), RecevingEndName("")
{
	MessageFlags.NewAvatarFlag = 0;
	MessageFlags.NewNameFlag = 0;
	MessageFlags.PrivateMessageFlag = 0;
	MessageFlags.RequestingMembersUpdate = 0;
	MessageFlags.NotifyingNewMemberFlag = 0;
	MessageFlags.ExitFlag = 0;
}

string ClientMessage::GetName(void) const
{
	return Name;
}
string ClientMessage::GetClientMessage(void) const
{
	return Message;
}

string ClientMessage::GetRecevingEndName(void) const
{
  return RecevingEndName;
}


void ClientMessage::SetName(string NewName)
{
	Name = NewName;
}
void ClientMessage::SetMessage(string NewMessage)
{
	Message = NewMessage;
}


void ClientMessage::SetRecevingEndName(string Name)
{
  RecevingEndName = Name;
}

unsigned char ClientMessage::GetNewNameFlag(void)
{
	return MessageFlags.NewNameFlag;
}

void ClientMessage::SetNewNameFlag(bool NewFlag)
{
	MessageFlags.NewNameFlag = NewFlag & 0x01;
}

unsigned char ClientMessage::GetPrivateMessageFlag(void)
{
  return MessageFlags.PrivateMessageFlag;
}

void ClientMessage::SetPrivateMessageFlag(bool NewFlag)
{
	MessageFlags.PrivateMessageFlag = NewFlag & 0x01;
}

unsigned char ClientMessage::GetRequestingMembersUpdate(void)
{
  return MessageFlags.RequestingMembersUpdate;
}

void ClientMessage::SetRequestingMembersUpdate(bool NewFlag)
{
  MessageFlags.RequestingMembersUpdate = NewFlag & 0x01;
}

unsigned char ClientMessage::GetNotifyingNewMemberFlag(void)
{
  return MessageFlags.NotifyingNewMemberFlag;
}


void ClientMessage::SetNotifyingNewMemberFlag(bool NewFlag)
{
  MessageFlags.NotifyingNewMemberFlag = NewFlag & 0x01;
}

unsigned char ClientMessage::GetExitFlag(void)
{
  return MessageFlags.ExitFlag;
}
void ClientMessage::SetExitFlag(bool NewFlag)
{
  MessageFlags.ExitFlag = NewFlag & 0x01;
}

/**************************Format of Buffer Message
 * First Byte : Flags;
 * Second Byte: Length of Name
 * Third-> N Byte : Name
 * N+1 -> N+3 : Length of Message ( Max 56K Bytes)
 * N+4 -> N+4+Length: Message
 * M->M+1 Length of receiving client
 * M+2 ->.... Name of Receiving Client
 *
 */

int ClientMessage::Serialize(char* Buffer)
{
	int TotalLength = 0;
	memcpy(&(Buffer[0]), &MessageFlags, sizeof(MessageFlags));
	TotalLength = TotalLength + 1;

	string Name = GetName();
	unsigned char NameLength = Name.size();
	TotalLength = TotalLength + NameLength + 1;

	memcpy(&(Buffer[1]), &NameLength, sizeof(NameLength));

	memcpy(&(Buffer[2]), Name.data(), sizeof(unsigned char) * NameLength); // Excluding the null terminator

	unsigned int StartOfMessageLengthByte = 2 + NameLength;
	string Message = GetClientMessage();
	unsigned short MessageLength = Message.size();
	TotalLength = TotalLength + MessageLength + 2;

	memcpy(&(Buffer[StartOfMessageLengthByte]), &MessageLength, sizeof(unsigned short));

	memcpy(&(Buffer[StartOfMessageLengthByte + 2]), Message.data(), sizeof(unsigned char) * MessageLength); // Excluding the null terminator

	if(MessageFlags.PrivateMessageFlag == 1)
	  {
	    //Then we have name of client in the Receiving end
	    unsigned int StartOfRecevingEndNamePart = StartOfMessageLengthByte + 2 + MessageLength;
	    NameLength = RecevingEndName.size();
	    TotalLength = TotalLength + NameLength + 1;

	    memcpy(&(Buffer[StartOfRecevingEndNamePart]), &NameLength, sizeof(NameLength));

	    memcpy(&(Buffer[StartOfRecevingEndNamePart + 1]), RecevingEndName.data(), sizeof(unsigned char) * NameLength); // Excluding the null terminator
	  }

	return TotalLength;
}

void ClientMessage::Deserialize(const char* Buffer)
{
	// We wont receive the Flags from a server we only send them, so we wont Deserlialize them
	string ReceivedName = "";
	unsigned char NameLength = 0;
	string ReceivedMessage="";
	unsigned short ReceivedMessageLength = 0;
	Flags ReveivedMessageFlags;

	memcpy(&ReveivedMessageFlags, &(Buffer[0]), sizeof(Flags));

	memcpy(&NameLength, &(Buffer[1]), sizeof(NameLength));
	ReceivedName.resize(NameLength);

	memcpy(&ReceivedName[0], &(Buffer[2]), sizeof(unsigned char) * NameLength);

	unsigned int StartOfMessageLengthByte = 2 + NameLength;

	memcpy(&ReceivedMessageLength, &(Buffer[StartOfMessageLengthByte]), sizeof(unsigned short));

	ReceivedMessage.resize(ReceivedMessageLength);
	memcpy(&ReceivedMessage[0], &(Buffer[StartOfMessageLengthByte + 2]), sizeof(unsigned char) * ReceivedMessageLength);


	if(ReveivedMessageFlags.PrivateMessageFlag == 1)
	  {
	    //Then we have name of client in the Receiving end
	    unsigned int StartOfRecevingEndNamePart = StartOfMessageLengthByte + 2 + ReceivedMessageLength;
	    memcpy(&NameLength, &(Buffer[StartOfRecevingEndNamePart]), sizeof(NameLength));
	    RecevingEndName.resize(NameLength);

	    memcpy(&RecevingEndName[0], &(Buffer[StartOfRecevingEndNamePart + 1]), sizeof(unsigned char) * NameLength);

	  }

	SetFlags(ReveivedMessageFlags);
	SetName(ReceivedName);
	SetMessage(ReceivedMessage);


}

void ClientMessage::SetFlags(Flags ReveivedMessageFlags)
{
	MessageFlags.NewNameFlag = ReveivedMessageFlags.NewNameFlag;
	MessageFlags.NewAvatarFlag = ReveivedMessageFlags.NewAvatarFlag;
	MessageFlags.PrivateMessageFlag = ReveivedMessageFlags.PrivateMessageFlag;
	MessageFlags.RequestingMembersUpdate = ReveivedMessageFlags.RequestingMembersUpdate;
	MessageFlags.NotifyingNewMemberFlag = ReveivedMessageFlags.NotifyingNewMemberFlag;
	MessageFlags.ExitFlag = ReveivedMessageFlags.ExitFlag;

}

int ClientMessage::GetLengthOfMessage()
{
	int TotalLength = 0;
	TotalLength = TotalLength + 1;
	unsigned char NameLength = Name.size();
	TotalLength = TotalLength + NameLength + 1;
	unsigned short MessageLength = Message.size();
	TotalLength = TotalLength + MessageLength + 2;

	return TotalLength;

}
