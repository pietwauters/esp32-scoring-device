//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "RS422_FPA_Type2_Message.h"
// There is a difference in transmitted accuracy if the timer is running or not
// If the timer is not running time is displayed with 2 numbers behind point (hundredths of seconds)
// If the timer is running accuracy is 1 second if there are 10 or more seconds remaining
// else, 1/10th of a second accuracy


RS422_FPA_Type2_Message::RS422_FPA_Type2_Message()
{
    //ctor
    m_message[0]= SOH;
    m_message[1]= DC3;
    m_message[2]= 'N';
    m_message[3]= STX;
    m_message[4]= ' ';
    m_message[5]= '3';
    m_message[6]= ':';
    m_message[7]= '0';
    m_message[8]= '0';
    m_message[9]= '.';
    m_message[10]= '0';
    m_message[11]= '0';
    m_message[12]= EOT;
    m_minutes = 3;
    m_seconds = 0;
    m_hundredths = 0;
    iCurrentSize = 13;
}

RS422_FPA_Type2_Message::~RS422_FPA_Type2_Message()
{
    //dtor
}

RS422_FPA_Type2_Message::RS422_FPA_Type2_Message(const RS422_FPA_Type2_Message& other)
{
    //copy ctor
    for(int i = 0; i < TYPE2MESSAGELENGTH ;i++)
    {
        m_message[i]= other.m_message[i];
    }
}

void RS422_FPA_Type2_Message::SetTimerStatus(char Z)
{

    if((m_message[IndexStatus] != 'R') && (m_message[IndexStatus] != 'N') && (m_message[IndexStatus] != 'J') && (m_message[IndexStatus] != 'B'))
        return;
    bRequiresTransmission = true;
    m_message[IndexStatus] =Z;
    //SetTime(m_minutes,m_seconds,m_hundredths);
}
void RS422_FPA_Type2_Message::SetTimeNoDecimal()
{

    m_message[IndexMinutes] = '0' + m_minutes;
    m_message[IndexSeconds] = '0' + m_seconds/10;
    m_message[IndexSeconds + 1] = '0' + m_seconds - 10*(m_seconds/10);
    m_message[IndexHundredths] = ' ';
    m_message[IndexHundredths + 1] = ' ';
    m_message[IndexDecimalPoint] = ' ';
    bRequiresTransmission = true;
}
void RS422_FPA_Type2_Message::SetTimeSingleDecimal()
{

    m_message[IndexMinutes] = '0' + m_minutes;
    m_message[IndexSeconds] = '0' + m_seconds/10;
    m_message[IndexSeconds + 1] = '0' + m_seconds - 10*(m_seconds/10);
    m_message[IndexHundredths] = '0' + m_hundredths/10;
    m_message[IndexHundredths + 1] = ' ';
    m_message[IndexDecimalPoint] = '.';
    bRequiresTransmission = true;
}
void RS422_FPA_Type2_Message::SetTimeDoubleDecimal()
{

    m_message[IndexMinutes] = '0' + m_minutes;
    m_message[IndexSeconds] = '0' + m_seconds/10;
    m_message[IndexSeconds + 1] = '0' + m_seconds - 10*(m_seconds/10);
    m_message[IndexHundredths] = '0' + m_hundredths/10;
    m_message[IndexHundredths + 1] = '0' + m_seconds - 10*(m_hundredths/10);
    m_message[IndexDecimalPoint] = '.';
    bRequiresTransmission = true;
}
void RS422_FPA_Type2_Message::SetTime(uint8_t minutes, uint8_t seconds, uint8_t hundredths)
{

    bRequiresTransmission = true;
    m_minutes = minutes;
    m_seconds = seconds;
    m_hundredths = hundredths;
    if((m_message[IndexStatus] == 'J') || (m_message[IndexStatus] == 'B'))
    {
        SetTimeNoDecimal();
    }
    else
    {
        if(m_message[IndexStatus] == 'N')
            SetTimeDoubleDecimal();
        else // Status = 'R'
        {
            if(minutes * 60 + seconds > 9)
            {
                SetTimeNoDecimal();
            }
            else
            {
                SetTimeSingleDecimal();
            }
        }
    }
}
