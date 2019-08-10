#include <Arduino.h>
#include "parser.h"
#include "reader.h"

char szTelegram[1024] = "";
ObisLine stObisLine;
char szMessage[1024] = "";

#include <Arduino.h>

void setup() {
  // setup wifi
  // setup mqtt
  // setup serial
  // setup dsmr (?)
}

void loop() {
  // if telegram received
  //   led on
  //   check crc
  //   if crc correct
  //     jsonify using raw OBIS IDs
  //     publish
  //   else
  //     publish error
  //   led off
  // sleep

  fnvParseObisLine(&stObisLine);
}
