//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "RS422_FPA_Message.h"


RS422_FPA_Message::RS422_FPA_Message()
{
    //ctor
}

RS422_FPA_Message::~RS422_FPA_Message()
{
    //dtor
}

RS422_FPA_Message::RS422_FPA_Message(const RS422_FPA_Message& other)
{
    //copy ctor
}

void RS422_FPA_Message::Print()
{

    int i = 1;
    std::cout << "SOH ";
    while(m_message[i] != EOT)
    {
        switch (m_message[i])
        {
            case DC3:
            std::cout << "DC3 ";
            break;

            case DC4:
            std::cout << "DC4 ";
            break;

            case STX:
            std::cout << "STX ";
            break;

            default:
            std::cout << m_message[i];
            break;

        }
        i++;

    }
    std::cout << " EOT" << std::endl;

}

size_t RS422_FPA_Message::CalculateSize()
{

    size_t i = 0;
    while(m_message[i]!= EOT)
        i++;
    iCurrentSize = i+1;
    return iCurrentSize;
}

void RS422_FPA_Message::setSingleDigitNumberInAscii(int value, int bytenumber)
{
    m_message[bytenumber] = '0' + value;
}

void RS422_FPA_Message::setDoubleDigitNumberInAscii(int value, int bytenumber)
{
    int tens = value / 10;
    if(tens > 0)
        m_message[bytenumber]= '0' + tens;
    else
        m_message[bytenumber]= ' ';
    m_message[bytenumber+1]= '0' + value - 10 * tens;
}
int RS422_FPA_Message::GetEOTIndex()
{
    int index = 0;
    while(m_message[index] != EOT)
    {
        if(index < MAX_MESSAGE_LENGTH_RS422)
            index++;
        else
            return -1;
    }
    return index;
}
