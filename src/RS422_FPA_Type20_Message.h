//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef RS422_FPA_TYPE20_MESSAGE_H
#define RS422_FPA_TYPE20_MESSAGE_H

#include "RS422_FPA_Message.h"


class RS422_FPA_Type20_Message : public RS422_FPA_Message
{
    public:
        RS422_FPA_Type20_Message();
        virtual ~RS422_FPA_Type20_Message();
        RS422_FPA_Type20_Message(const RS422_FPA_Type20_Message& other);
        void SetContact(bool value);


    protected:

    private:
};

#endif // RS422_FPA_TYPE20_MESSAGE_H
