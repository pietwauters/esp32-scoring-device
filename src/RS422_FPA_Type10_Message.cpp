//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "RS422_FPA_Type10_Message.h"
#include <iostream>
#include <cstdio>
// using namespace std;

RS422_FPA_Type10_Message::RS422_FPA_Type10_Message()
{
    //ctor
    // create message with empty timer
    m_message[0]= SOH;
    m_message[1]= DC3;
    m_message[2]= 'C';  // CS is the identifyer for Cyrano Status
    m_message[3]= 'S';
    m_message[4]= STX;
    m_message[5]= '0';  // PisteNr
    m_message[6]= '0';
    m_message[7]= '0';
    m_message[8]= STX;
    m_message[9]= '0';  // Cyrano Online? '2' online '1' External WiFi online but Cyrano offline; '0' No Wifi
    m_message[10]= STX;
    m_message[11]= 'U';  // Machine Status: 'U' undefined; 'W' Waiting; 'H' Halted; 'F' Fencing; 'E' Ending; 'P' Paused; '5' Received NAK
    m_message[12]= STX;
    m_message[13]= '0';  // WiFi IP Address
    m_message[14]= '.';
    m_message[15]= '0';
    m_message[16]= '.';
    m_message[17]= '0';
    m_message[18]= '.';
    m_message[19]= '0';
    m_message[20]= EOT;
    iCurrentSize = 21;

}

RS422_FPA_Type10_Message::~RS422_FPA_Type10_Message()
{
    //dtor
}

void RS422_FPA_Type10_Message::SetPiste(int PisteNr)
{
  char temp[8];
  sprintf(temp,"%03d",PisteNr);
  m_message[5]= temp[0];
  m_message[6]= temp[1];
  m_message[7]= temp[2];
}

void RS422_FPA_Type10_Message::SetIPAddress(uint32_t IPAddress)
{
  union {
        uint8_t bytes[4];  // IPv4 address
        uint32_t dword;
    } _address;
  _address.dword = IPAddress;
  m_message[13]= _address.bytes[0];
  m_message[15]= _address.bytes[1];
  m_message[17]= _address.bytes[2];
  m_message[19]= _address.bytes[3];
}

RS422_FPA_Type10_Message::RS422_FPA_Type10_Message(const RS422_FPA_Type10_Message& other)
{
    //copy ctor
}


void RS422_FPA_Type10_Message::SetCyranoStatus(const uint8_t TheStatus)
{
      m_message[9]= TheStatus;
}

void RS422_FPA_Type10_Message::SetMachineStatus( const char statevalue)
{
  m_message[11]= statevalue;

}
