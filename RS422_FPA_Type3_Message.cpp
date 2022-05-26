//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "RS422_FPA_Type3_Message.h"

RS422_FPA_Type3_Message::RS422_FPA_Type3_Message()
{
    //ctor
    m_message[0]= SOH;
    m_message[1]= DC3;
    m_message[2]= 'D';
    m_message[3]= STX;
    m_message[4]= ' ';
    m_message[5]= '0';
    m_message[6]= ':';
    m_message[7]= ' ';
    m_message[8]= '0';
    m_message[9]= STX;
    m_message[10]= ' ';
    m_message[11]= '0';
    m_message[12]= ' ';
    m_message[13]= '0';
    m_message[14]= '0';
    m_message[15]= STX;
    m_message[16]= ' ';
    m_message[17]= '0';
    m_message[18]= ' ';
    m_message[19]= '0';
    m_message[20]= '0';
    m_message[21]= STX;
    m_message[22]= '0';
    m_message[23]= STX;
    m_message[24]= '1';
    m_message[25]= STX;
    m_message[26]= '2';
    m_message[27]= '2';
    m_message[28]= EOT;
    iCurrentSize = 29;

}

RS422_FPA_Type3_Message::~RS422_FPA_Type3_Message()
{
    //dtor
}

RS422_FPA_Type3_Message::RS422_FPA_Type3_Message(const RS422_FPA_Type3_Message& other)
{
    //copy ctor
    for(int i = 0; i < TYPE3MESSAGELENGTH ;i++)
    {
        m_message[i]= other.m_message[i];
    }
}

RS422_FPA_Type3_Message& RS422_FPA_Type3_Message::operator=(const RS422_FPA_Type3_Message& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    return *this;
}

void RS422_FPA_Type3_Message::SetScoreLeft(int value)
{
    bRequiresTransmission = true;
    setDoubleDigitNumberInAscii(value,7);

}
void RS422_FPA_Type3_Message::SetScoreRight(int value)
{
    bRequiresTransmission = true;
    setDoubleDigitNumberInAscii(value,4);
}

void RS422_FPA_Type3_Message::SetYellowCardLeft(int value){bRequiresTransmission = true;setDoubleDigitNumberInAscii(value,16);};
void RS422_FPA_Type3_Message::SetYellowCardRight(int value){bRequiresTransmission = true;setDoubleDigitNumberInAscii(value,10);};
void RS422_FPA_Type3_Message::SetRedCardLeft(int value){bRequiresTransmission = true;setDoubleDigitNumberInAscii(value,18);};
void RS422_FPA_Type3_Message::SetRedCardRight(int value){bRequiresTransmission = true;setDoubleDigitNumberInAscii(value,12);};
void RS422_FPA_Type3_Message::SetBlackCardLeft(int value){bRequiresTransmission = true;setSingleDigitNumberInAscii(value,20);};
void RS422_FPA_Type3_Message::SetBlackCardRight(int value){bRequiresTransmission = true;setSingleDigitNumberInAscii(value,14);};
void RS422_FPA_Type3_Message::MakeRSingleDigit()
{
    if(STX != m_message[25])
    {
        m_message[25] = STX;
        m_message[26] = m_message[27];
        m_message[27] = m_message[28];
        m_message[28] = EOT;
    }
}
void RS422_FPA_Type3_Message::SetRound(int value)
{
    if(value == -1) // eXtra minute
    {
        m_message[24] = 'X';
        MakeRSingleDigit(); // check if we need to trim
    }
    else
    {
        if(value < 10)
        {
            setSingleDigitNumberInAscii(value,24);
            MakeRSingleDigit(); // check if we need to trim
        }
        else
        {
            // make room if needed
            if(m_message[25] == STX)
            {
                iCurrentSize = 30;
                m_message[29] = EOT;
                m_message[28] = m_message[27];
                m_message[27] = m_message[26];
                m_message[26] = STX;
            }
            setDoubleDigitNumberInAscii(value,24);
        }
    }
}

void RS422_FPA_Type3_Message::SetVideoCallsRight(int value)
{
    bRequiresTransmission = true;
    int index = GetEOTIndex() - 2;
    setSingleDigitNumberInAscii(value,index);
}
void RS422_FPA_Type3_Message::SetVideoCallsLeft(int value)
{
    bRequiresTransmission = true;
    int index = GetEOTIndex() - 1;
    setSingleDigitNumberInAscii(value,index);
}
