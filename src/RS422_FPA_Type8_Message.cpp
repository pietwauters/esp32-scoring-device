//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "RS422_FPA_Type8_Message.h"


RS422_FPA_Type8_Message::RS422_FPA_Type8_Message()
{
    //ctor
    // create message with empty timer
    m_message[0]= SOH;
    m_message[1]= DC3;
    m_message[2]= 'U';
    m_message[3]= 'F';
    m_message[4]= STX;
    m_message[5]= STX;
    m_message[6]= '0';
    m_message[7]= STX;
    m_message[8]= '0';
    m_message[9]= EOT;
    iCurrentSize = 10;

    /*m_message[0]= SOH;
    m_message[1]= DC3;
    m_message[2]= 'U';
    m_message[3]= 'F';
    m_message[4]= STX;
    m_message[5]= '0';
    m_message[6]= ':';
    m_message[7]= '0';
    m_message[8]= '0';
    m_message[9]= STX;
    m_message[10]= '0';
    m_message[11]= STX;
    m_message[12]= '0';
    m_message[13]= EOT;
    iCurrentSize = 14;*/
}

RS422_FPA_Type8_Message::~RS422_FPA_Type8_Message()
{
    //dtor
}

RS422_FPA_Type8_Message::RS422_FPA_Type8_Message(const RS422_FPA_Type8_Message& other)
{
    //copy ctor
}

void RS422_FPA_Type8_Message::SetTime(uint8_t minutes, uint8_t seconds)
{

    if(m_message[6] != ':') // Unwillingness to fight timer WAS NOT  used before
    {
        m_message[9]= STX;
        m_message[10]= m_message[6];
        m_message[11]= STX;
        m_message[12]= m_message[8];
        m_message[13]= EOT;
        m_message[6]= ':';
        CalculateSize();
    }

    m_message[5]= '0' + minutes;
    m_message[7]= '0' + seconds / 10;
    m_message[8]= '0' + seconds % 10;
}
void RS422_FPA_Type8_Message::SetPCardRight(uint8_t Status)
{

    if(m_message[6] == ':') // Unwillingness to fight timer is used
    {
        m_message[10]= '0' + Status;
    }
    else
    {
        m_message[6]= '0' + Status;
    }
}
void RS422_FPA_Type8_Message::SetPCardLeft(uint8_t Status)
{

    if(m_message[6] == ':') // Unwillingness to fight timer is used
    {
        m_message[12]= '0' + Status;
    }
    else
    {
        m_message[8]= '0' + Status;
    }
}
