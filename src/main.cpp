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
  /*
  {
    "<obisid>":{
      "value":"<value>",
      "unit":"<unit>"
    },
    "1-0:99.97.0":{
      "0":{
        "obisid":{
          "value":"<value>",
          "unit":"<unit>"
        }
      },
      "1":{
        "obisid":{
          "value":"<value>",
          "unit":"<unit>"
        }
      }
    },
    "0-n:24.2.1":{
      "timestamp": "<timestamp>",
      "value": "<value>",
      "unit": "<unit>"
    }
  }

  if telegram received
    led on
    check crc
    if crc correct
      ignore first line (vendor proprietary)
      extract obis id
      if single-line
        jsonify using raw OBIS IDs
      if multi-line
        extract number of lines
        for each line
          jsonify using raw OBIS IDs (like single line)
      if mbus-line
        jsonify using raw OBIS IDs
      publish
    else
      publish error
    led off
  sleep
*/
  fnvParseObisLine(&stObisLine);
}
