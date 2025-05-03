//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "RS422_FPA_Type4_Message.h"

RS422_FPA_Type4_Message::RS422_FPA_Type4_Message()
{
    //ctor
    m_message[0]= SOH;
    m_message[1]= DC3;
    m_message[2]= 'I';
    m_message[3]= STX;
    m_message[4]= '0';  // Match Status
    m_message[5]= STX;
    m_message[6]= '0';  // Weapon
    m_message[7]= STX;
    m_message[8]= '0';  // Service Call
    m_message[9]= STX;
    m_message[10]= '0'; // Request Doctor, Technician, Video Assistance
    m_message[11]= EOT;
    iCurrentSize = 12;
}

RS422_FPA_Type4_Message::~RS422_FPA_Type4_Message()
{
    //dtor
}

RS422_FPA_Type4_Message::RS422_FPA_Type4_Message(const RS422_FPA_Type4_Message& other)
{
    //copy ctor
    for(int i = 0; i < TYPE4MESSAGELENGTH ;i++)
    {
        m_message[i]= other.m_message[i];
    }
}

RS422_FPA_Type4_Message& RS422_FPA_Type4_Message::operator=(const RS422_FPA_Type4_Message& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    return *this;
}

void RS422_FPA_Type4_Message::setWeapon(weapon_t W)
{

    switch(W)
    {
        case EPEE:
        m_message[6]= '1';
        break;

        case SABRE:
        m_message[6]= '2';
        break;

        case FOIL:
        m_message[6]= '3';
        break;

        default:
        m_message[6]= '0';

    }
    bRequiresTransmission = true;
}
