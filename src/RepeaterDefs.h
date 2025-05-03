//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef REPEATERDEFS_H
#define REPEATERDEFS_H
enum MessageType_t {EVENT, HEARTBEAT, PAIRING_REQUEST};
typedef struct struct_message {
  MessageType_t type;
  uint32_t event;
  int piste_ID;
  long messagenumber;
} struct_message;
#define FULL_STATUS_REPETITION_PERIOD 1021
#define MESSAGE_REPETITION_FACTOR 4

#endif // REPEATERDEFS_H
