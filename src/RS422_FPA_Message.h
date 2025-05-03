//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef RS422_FPA_MESSAGE_H
#define RS422_FPA_MESSAGE_H
#include <iostream>
#include "weaponenum.h"
//#include <Arduino.h>
// using namespace std;

#define MAX_MESSAGE_LENGTH_RS422	39 + 4
#define SOH  0x01
#define DC3  0x13
#define DC4  0x14
#define EOT  0x04
#define STX  0x02

//enum weapon_t {undefined = 0, Epee, Sabre, Foil};

class RS422_FPA_Message
{
    public:
        RS422_FPA_Message();
        virtual ~RS422_FPA_Message();
        RS422_FPA_Message(const RS422_FPA_Message& other);

        /*char Getmessage[39]() { return m_message[39]; }
        void Setmessage[39](char val) { m_message[39] = val; }*/
        void Print();
        void SerialOut();
        bool NeedsTransmission() {return bRequiresTransmission;};
        void SetMessageTransmitted() {bRequiresTransmission = false;};
        size_t GetCurrentSize(){return iCurrentSize;};
        //const uint8_t * GetBuffer(){return m_message;};
        uint8_t * GetBuffer(){return m_message;};
        void setSingleDigitNumberInAscii(int value, int bytenumber);
        void setDoubleDigitNumberInAscii(int value, int bytenumber);
        int GetEOTIndex();

    protected:
        size_t CalculateSize();
        uint8_t m_message[MAX_MESSAGE_LENGTH_RS422];
        bool bRequiresTransmission = true;
        size_t iCurrentSize = MAX_MESSAGE_LENGTH_RS422;

    private:
		bool SerialAttached = false;


};

#endif // RS422_FPA_MESSAGE_H
