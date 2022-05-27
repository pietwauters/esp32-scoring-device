//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef RS422_FPA_TYPE2_MESSAGE_H
#define RS422_FPA_TYPE2_MESSAGE_H

#include "RS422_FPA_Message.h"
#define TYPE2MESSAGELENGTH 13
#define IndexStatus 2
#define IndexMinutes 5
#define IndexSeconds 7
#define IndexDecimalPoint 9
#define IndexHundredths 10

enum TimeStatus_t {Running, Stopped, Injury, Break};

//const char DefaultMessage2[] = {SOH,DC3,'N',STX,' ','3',':','0','0','.','0','0',EOT};

class RS422_FPA_Type2_Message : public RS422_FPA_Message
{
    public:
        RS422_FPA_Type2_Message();
        virtual ~RS422_FPA_Type2_Message();
        RS422_FPA_Type2_Message(const RS422_FPA_Type2_Message& other);
        void SetTime(uint8_t minutes, uint8_t seconds, uint8_t hundredths);
        void SetTimerStatus(char Z);

    protected:

    private:
    void SetTimeNoDecimal();
    void SetTimeSingleDecimal();
    void SetTimeDoubleDecimal();
    uint8_t m_minutes;
    uint8_t m_seconds;
    uint8_t m_hundredths;

};

#endif // RS422_FPA_TYPE2_MESSAGE_H
