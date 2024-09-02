/*
 * ClientMessageClass.cpp
 *
 *  Created on: Sep 1, 2024
 *      Author: Hussein Shehata
 */


#include "ClientMessageClass.hpp"
using namespace std;
ClientMessage::ClientMessage():Name(""), Message("")
{
	MessageFlags.NewAvatarFlag = 0;
	MessageFlags.NewNameFlag = 0;
}

string ClientMessage::GetName(void) const
{
	return Name;
}
string ClientMessage::GetClientMessage(void) const
{
	return Message;
}

void ClientMessage::SetName(string NewName)
{
	Name = NewName;
}
void ClientMessage::SetMessage(string NewMessage)
{
	Message = NewMessage;
}

unsigned char ClientMessage::GetNewNameFlag(void)
{
	return MessageFlags.NewNameFlag;
}

void ClientMessage::SetNewNameFlag(bool NewFlag)
{
	MessageFlags.NewNameFlag = NewFlag & 0x01;
}


/**************************Format of Buffer Message
 * First Byte : Flags;
 * Second Byte: Length of Name
 * Third-> N Byte : Name
 * N+1 -> N+3 : Length of Message ( Max 56K Bytes)
 * N+4 -> ....: Message
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

	SetFlags(ReveivedMessageFlags);
	SetName(ReceivedName);
	SetMessage(ReceivedMessage);


}

void ClientMessage::SetFlags(Flags ReveivedMessageFlags)
{
	MessageFlags.NewNameFlag = ReveivedMessageFlags.NewNameFlag;
	MessageFlags.NewAvatarFlag = ReveivedMessageFlags.NewAvatarFlag;

}
