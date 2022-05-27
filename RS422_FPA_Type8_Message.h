//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef RS422_FPA_TYPE8_MESSAGE_H
#define RS422_FPA_TYPE8_MESSAGE_H

#include "RS422_FPA_Message.h"


class RS422_FPA_Type8_Message : public RS422_FPA_Message
{
    public:
        RS422_FPA_Type8_Message();
        virtual ~RS422_FPA_Type8_Message();
        RS422_FPA_Type8_Message(const RS422_FPA_Type8_Message& other);
        void SetTime(uint8_t minutes, uint8_t seconds);
        void SetPCardRight(uint8_t Status);
        void SetPCardLeft(uint8_t Status);

    protected:

    private:
};

#endif // RS422_FPA_TYPE8_MESSAGE_H
