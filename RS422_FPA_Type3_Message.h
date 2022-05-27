//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef RS422_FPA_TYPE3_MESSAGE_H
#define RS422_FPA_TYPE3_MESSAGE_H

#include "RS422_FPA_Message.h"

#define TYPE3MESSAGELENGTH 29
#define IndexScoreRight



class RS422_FPA_Type3_Message : public RS422_FPA_Message
{
    public:
        RS422_FPA_Type3_Message();
        virtual ~RS422_FPA_Type3_Message();
        RS422_FPA_Type3_Message(const RS422_FPA_Type3_Message& other);
        RS422_FPA_Type3_Message& operator=(const RS422_FPA_Type3_Message& other);
        void SetScoreLeft(int value);
        void SetScoreRight(int value);
        void SetYellowCardLeft(int value);
        void SetYellowCardRight(int value);
        void SetRedCardLeft(int value);
        void SetRedCardRight(int value);
        void SetBlackCardLeft(int value);
        void SetBlackCardRight(int value);
        void SetPrioLeft(){bRequiresTransmission = true;setSingleDigitNumberInAscii(2,22);};
        void SetPrioRight(){bRequiresTransmission = true;setSingleDigitNumberInAscii(1,22);};
        void SetNoPrio(){bRequiresTransmission = true;setSingleDigitNumberInAscii(0,22);};
        void SetRound(int value);
        void SetVideoCallsRight(int value);
        void SetVideoCallsLeft(int value);

    protected:

    private:
    void MakeRSingleDigit();
};

#endif // RS422_FPA_TYPE3_MESSAGE_H
