//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "RS422_FPA_Type1_Message.h"



RS422_FPA_Type1_Message::RS422_FPA_Type1_Message()
{
    //ctor

    m_message[0]= SOH;
    m_message[1]= DC4;
    m_message[2]= 'R';
    m_message[3]= '0';
    m_message[4]= 'G';
    m_message[5]= '0';
    m_message[6]= 'W';
    m_message[7]= '0';
    m_message[8]= 'w';
    m_message[9]= '0';
    m_message[10]= EOT;
    iCurrentSize = 11;


}

RS422_FPA_Type1_Message::~RS422_FPA_Type1_Message()
{
    //dtor
}

RS422_FPA_Type1_Message::RS422_FPA_Type1_Message(const RS422_FPA_Type1_Message& other)
{
    //copy ctor
    for(int i = 0; i < TYPE1MESSAGELENGTH ;i++)
    {
        m_message[i]= other.m_message[i];
    }
}
