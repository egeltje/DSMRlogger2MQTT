#include <Arduino.h>
#include "reader.h"

void fnvRead(void) {
    static char sZChecksumValue[4];
    uint8_t iSerialByte;

    if (Serial.available() > 0) {
        iSerialByte = Serial.read();
        switch (iReaderState) {
            case READER_STATE_OFF:
                // ignore all the serial bytes
                break;
            case READER_STATE_WAITING:
                // wait for start of telegram

                if (iSerialByte == '/') {
                    iReaderState = READER_STATE_READING;
                    //buf = iSerialByte;
                }
                break;
            case READER_STATE_READING:
                if (Serial.available() > 0) {
                    iSerialByte = Serial.read();

                    if (iSerialByte == '!') {
                        iReaderState = READER_STATE_CRC;
                        //buf = iSerialByte;
                    }
                }
                break;
            case READER_STATE_CRC:
                // iSerialByte;
                if (Serial.available() > 0) {
                    iSerialByte = Serial.read();
                }
                break;
            case READER_STATE_PARSING:
                break;
        }
    }
}