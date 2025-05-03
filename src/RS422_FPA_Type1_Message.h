//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef RS422_FPA_TYPE1_MESSAGE_H
#define RS422_FPA_TYPE1_MESSAGE_H

#include "RS422_FPA_Message.h"
#define TYPE1MESSAGELENGTH 11
#define IndexRed 3
#define IndexGreen 5
#define IndexWhiteLeft 9
#define IndexWhiteRight 7


class RS422_FPA_Type1_Message : public RS422_FPA_Message
{
    public:
        RS422_FPA_Type1_Message();
        virtual ~RS422_FPA_Type1_Message();
        RS422_FPA_Type1_Message(const RS422_FPA_Type1_Message& other);
        void SetRed(bool value){bRequiresTransmission = true;if(value){m_message[IndexRed]= '1';}else{m_message[IndexRed]= '0';}};
        void SetGreen(bool value){bRequiresTransmission = true;if(value){m_message[IndexGreen]= '1';}else{m_message[IndexGreen]= '0';}};
        void SetWhiteLeft(bool value){bRequiresTransmission = true;if(value){m_message[IndexWhiteLeft]= '1';}else{m_message[IndexWhiteLeft]= '0';}};
        void SetWhiteRight(bool value){bRequiresTransmission = true;if(value){m_message[IndexWhiteRight]= '1';}else{m_message[IndexWhiteRight]= '0';}};

    protected:

    private:
};

#endif // RS422_FPA_TYPE1_MESSAGE_H
