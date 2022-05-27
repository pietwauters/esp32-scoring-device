//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef RS422_FPA_TYPE4_MESSAGE_H
#define RS422_FPA_TYPE4_MESSAGE_H

#include "RS422_FPA_Message.h"
#define TYPE4MESSAGELENGTH 12

class RS422_FPA_Type4_Message : public RS422_FPA_Message
{
    public:
        RS422_FPA_Type4_Message();
        virtual ~RS422_FPA_Type4_Message();
        RS422_FPA_Type4_Message(const RS422_FPA_Type4_Message& other);
        RS422_FPA_Type4_Message& operator=(const RS422_FPA_Type4_Message& other);
        void setWeapon(weapon_t W);

    protected:

    private:
};

#endif // RS422_FPA_TYPE4_MESSAGE_H
