

#include "ir_lg.h"

void ir_lg::send(unsigned long command)
{
  irsend.sendNEC(pre_data + command, 32);
  delay(100);
  Serial.print("sending: ");
  Serial.println(pre_data + command,HEX);
}

//
// END OF FILE
//
