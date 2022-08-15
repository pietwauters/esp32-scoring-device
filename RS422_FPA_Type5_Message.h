//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef RS422_FPA_TYPE5_MESSAGE_H
#define RS422_FPA_TYPE5_MESSAGE_H

#include "RS422_FPA_Message.h"
#define TYPE5MESSAGELENGTH 39

class RS422_FPA_Type5_6_Message : public RS422_FPA_Message
{
    public:
        RS422_FPA_Type5_6_Message();
        virtual ~RS422_FPA_Type5_6_Message();
        RS422_FPA_Type5_6_Message(const RS422_FPA_Type5_6_Message& other);
        RS422_FPA_Type5_6_Message& operator=(const RS422_FPA_Type5_6_Message& other);
        void SetTypeToLeft(){m_message[3]= 'L';};
        void SetTypeToRight(){m_message[3]= 'R';};
        void SetUID(const char* name, size_t len = 8);
        void SetName(const char* name, size_t len = 20);
        void SetNOC(const char* NOC);

    protected:

    private:
};

#endif // RS422_FPA_TYPE5_MESSAGE_H
