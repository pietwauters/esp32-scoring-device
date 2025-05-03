//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "RS422_FPA_Type5_Message.h"

RS422_FPA_Type5_6_Message::RS422_FPA_Type5_6_Message()
{
    //ctor
    m_message[0]= SOH;
    m_message[1]= DC3;
    m_message[2]= 'N';
    m_message[3]= 'L';
    m_message[4]= STX;
    m_message[5]= STX;
    m_message[6]= STX;
    m_message[7]= EOT;
    iCurrentSize = 8;
}

RS422_FPA_Type5_6_Message::~RS422_FPA_Type5_6_Message()
{
    //dtor
}

// Below is not correct because if the target message is shorter than the Source
// there will be illegal memory access
// Not an issue, because there is always storage for a max size packet
RS422_FPA_Type5_6_Message::RS422_FPA_Type5_6_Message(const RS422_FPA_Type5_6_Message& other)
{
    //copy ctor
    for(int i = 0; i < TYPE5MESSAGELENGTH ;i++)
    {
        m_message[i]= other.m_message[i];
    }
}

RS422_FPA_Type5_6_Message& RS422_FPA_Type5_6_Message::operator=(const RS422_FPA_Type5_6_Message& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    return *this;
}

void RS422_FPA_Type5_6_Message::SetName(const char* name, size_t len)
{

    uint8_t reallen = 20;
    bRequiresTransmission = true;

    char temp[TYPE5MESSAGELENGTH];
    if(len < 20)
        reallen = len;
    // find startposision

    int i = 5;
    while((m_message[i] != STX) && (i < 12))
        i++;

    // i now points to the STX indicating the start of the name
    int StartOfName = i++;
    while((m_message[i] != STX) && (i < TYPE5MESSAGELENGTH-1))
        i++;

    // i now points to the STX indicating the end of the name & start of NOC
    int EndOfName = i;
    // Now we have to save the part that comes after the current name
    temp[0] = m_message[i++];
    temp[1] = m_message[i++];
    temp[2] = m_message[i++];
    temp[3] = m_message[i++];
    temp[4] = m_message[i++];


    StartOfName++;
    for(int j = StartOfName; j< StartOfName+reallen ; j++)
    {
        m_message[j] = *name++;
    }
    m_message[StartOfName++ + reallen] = temp[0];
    m_message[StartOfName++ + reallen] = temp[1];
    m_message[StartOfName++ + reallen] = temp[2];
    m_message[StartOfName++ + reallen] = temp[3];
    m_message[StartOfName++ + reallen] = temp[4];
    CalculateSize();
}

void RS422_FPA_Type5_6_Message::SetNOC(const char* NOC)
{

    int i = 5;
    bRequiresTransmission = true;
    while((m_message[i] != STX) && (i < 12))
        i++;
    i++;
    while((m_message[i] != STX) && (i < TYPE5MESSAGELENGTH-1))
        i++;
    // i now points to the STX indicating the end of the name & start of NOC
    int EndOfName = i;
    if(NOC[0] == 0)
    {
        m_message[i+1] = EOT;
        CalculateSize();
        return;
    }
    if(m_message[i+1] == EOT)
    {
        m_message[i+4] = EOT;
    }
    m_message[++i] = NOC[0];
    m_message[++i] = NOC[1];
    m_message[++i] = NOC[2];
    CalculateSize();
}

void RS422_FPA_Type5_6_Message::SetUID(const char* UID, size_t len)
{

    uint8_t reallen = 8;
    bRequiresTransmission = true;
    char temp[TYPE5MESSAGELENGTH];
    if(len < 8)
        reallen = len;
    // find startposision

    int i = 5;
    while((m_message[i] != STX) && (i < 5+8))
        i++;

    int EndOfUID = i;
    int k = 0;
    while(m_message[i] != EOT)
    {
        temp[k++] = m_message[i++];
    }
    int remaining = k;

    for(int j = 5; j< 5+reallen ; j++)
    {
        m_message[j] = *UID++;
    }
    k=0;
    int j;
    for(j = 0; j < remaining; j++)
        m_message[5+reallen + j] = temp[j];
    m_message[5+reallen + j] = EOT;
    CalculateSize();
}
