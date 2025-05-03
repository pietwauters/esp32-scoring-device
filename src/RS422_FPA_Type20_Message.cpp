//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "RS422_FPA_Type20_Message.h"
#include <iostream>
#include <cstdio>
// using namespace std;

RS422_FPA_Type20_Message::RS422_FPA_Type20_Message()
{
    //ctor
    m_message[0]= SOH;
    m_message[1]= DC3;
    m_message[2]= 'B';  // BC is the identifyer for blade contact
    m_message[3]= 'C';
    m_message[4]= STX;
    m_message[5]= '0';  // 0/1
    m_message[6]= EOT;
    iCurrentSize = 7;

}

RS422_FPA_Type20_Message::~RS422_FPA_Type20_Message()
{
    //dtor
}



RS422_FPA_Type20_Message::RS422_FPA_Type20_Message(const RS422_FPA_Type20_Message& other)
{
    //copy ctor
}

void RS422_FPA_Type20_Message::SetContact(bool value){
  if(value)
    m_message[5]= '1';
  else
    m_message[5]= '0';
}
