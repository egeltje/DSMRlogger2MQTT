/*
 * parser.cpp
 * This file contains functions for parsing OBIS lines
 * and converting them to JSON.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

/*
 * fnpszConvertObisTimestamp
 * Translate the OBIS formatted timestamp to ISO8601
 * From YYMMDDhhmmssX to YYYY-MM-DDTHH:MM:SS+00:00"
 * Input: pointer to char array with Obis timestamp
 * Output: char array with ISO timestamp
 */
char* fnpszConvertObisTimestamp(char* _szObisTimestamp) {
    static char _szISOTimestamp[25] = "";
    char _szTimezone[7] = "";

    if (strlen(szObisTimestamp) == 13) {
        switch (szObisTimestamp[12]) {
            case 'W':
                strcpy(_szTimezone, "+01:00"); // CET
                break;
            case 'S':
                strcpy(_szTimezone, "+02:00"); // CEST
                break;
            default:
                strcpy(_szTimezone, "+00:00"); // Zulu
                break;
        }

        // This is super evil assuming that the passed Obis timezone is formatted correctly.
        sprintf(_szISOTimestamp, "20%c%c-%c%c-%c%cT%c%c:%c%c:%c%c%s",   
                            szObisTimestamp[0], szObisTimestamp[1], szObisTimestamp[2], szObisTimestamp[3], szObisTimestamp[4], szObisTimestamp[5],
                            szObisTimestamp[6], szObisTimestamp[7], szObisTimestamp[8], szObisTimestamp[9], szObisTimestamp[10], szObisTimestamp[11],
                            _szTimezone);
    } else {
        // Error
    }

    return _szISOTimestamp;
}

/*
 * Three types of valid OBIS messages
 * 1) single:    <id>(<value>)
 * 2) timestamp: <id>(<timestamp>)(<value>)
 * 3) log:       <id>(n)(id)(<timestamp(0)>)(<value0>)..(<timestamp(n)>)(<value(n)>)
 * The log type is basically one or more timestamp type messages and will be parsed as such
 */
void fnvParseObisLine(ObisLine* pstObisLine) {
    uint16_t    _iPos = 0;
    pstObisLine->bValid = false;
    // extract OBIS ID
    // if multi-line -> multiline
    // if mbus-line -> mbusline
    // all else -> singleline

    // parse ID
    while ((_iPos <= _iLineLength) && (_szLine[_iPos] != '(')) {
        _iPos++;
    }
    if (_iPos <= _iLineLength) {
        _iIDLength = (_iPos - 1) - _iIDStart;
        strncpy(_szID, _pszLine[_iIDStart], _iIDLength);
    } else {
        // Error
        _iIDLength = 0;
    }

}

char* fnpszParseSingleValue2JSON(char* _pszLine, uint16_t _iLineLength, char _cValueType) {
    uint16_t    _iPos = 0;
    char        _szValue[256] = "";
    uint16_t     _iValueStart = 0;
    uint16_t     _iValueLength = 0;
    char        _szUnit[4] = "";
    uint16_t     _iUnitStart = 0;
    uint16_t     _iUnitLength = 0;
    static char _szReturnMessage[256 + 4 + 22 + 1] = ""; // Value + Unit + JSON characters + \0

    if (_szLine[_iPos] == '(') {
        // parse value / unit, _iPos is at value bracket (
        _iPos++; // ignore the bracket
        _iValueStart = _iPos;

        while ((_iPos <= _iLineLength) && (_pszLine[_iPos] != '*') && (_pszLine[_iPos] != ')')) {
            _iPos++;
        }
        if (_iPos <= _iLineLength) {
            _iValueLength = _iPos - _iValueStart;
            strncpy(_szValue, _pszLine[_iValueStart], _iValueLength);

            if (_pszLine[_iPos] == '*') { // unit found
                // parse unit, _iPos is at unit asterisk *
                _iPos++; // ignore the asterisk
                _iUnitStart = _iPos;

                while ((_iPos <= _iLineLength) && (_pszLine[_iPos] != ')')) {
                    _iPos++;
                }
                if (_iPos <= _iLineLength) {
                    _iUnitLength = _iPos - _iUnitStart;
                    strncpy(_szUnit, _pszLine[_iUnitStart], _iUnitLength);
                } else {
                    // Error
                    _iUnitLength = 0;
                }
            }
        } else {
            // Error
            _iValueLength = 0;
        }
        
        if (_iValueLength > 0) {
            // TODO: Format according to ValueType
            sprintf(_szReturnMessage, "{\"value\":\"%s\",\"unit\":\"%s\"}", _szValue, _szUnit);
        } else {
            // Error
            sprintf(_szReturnMessage, "{\"ERROR\":\"Could not parse value or unit\"}");
        }
    } else {
        // Error, line did not start with a bracket (
    }
    return _szReturnMessage;
}

char* fnpszParseTimestampedValue2JSON(char* _pszLine, uint16_t _iLineLength, char _cValueType) {
    uint16_t    _iPos = 0;
    char        _sztimestamp[25] = "";
    uint16_t     _iTimestampStart = 0;
    uint16_t     _iTimestampLength = 0;
    char        _szValue[256] = "";
    uint16_t     _iValueStart = 0;
    uint16_t     _iValueLength = 0;
    char        _szUnit[4] = "";
    uint16_t     _iUnitStart = 0;
    uint16_t     _iUnitLength = 0;
    static char _szReturnMessage[25 + 256 + 4 + 36 + 1] = ""; // ISOtimestampp + Value + Unit + JSON characters + \0

    if (_szLine[_iPos] == '(') {
        // timestamp, _iPos is at timestamp bracket (
        _iPos++; // ignore the bracket
        _iTimestampStart = _iPos;

        while ((_iPos <= _iLineLength) && (_pszLine[_iPos] != ')')) {
            _iPos++;
        }
        if (_iPos <= _iLineLength) {
            _iTimestampLength = _iPos - _iTimestampStart;
            strncpy(_szTimestamp, _pszLine[_iTimestampStart], _iTimestampLength);
            _sztimestamp = fnpszConvertObisTimestamp(_sztimestamp);
        } else {
            // Error
        }

        // Continue to the value bracket (
        while ((_iPos <= _iLineLength) && (_pszLine[_iPos] != '(')) {
            _iPos++;
        }
        // parse value / unit, _iPos is at value bracket )
        _iPos++; // ignore the bracket
        _iValueStart = _iPos;

        while ((_iPos <= _iLineLength) && (_pszLine[_iPos] != '*') && (_pszLine[_iPos] != ')')) {
            _iPos++;
        }
            if (_pszLine[_iPos] == '*') { // unit found
                // parse unit, _iPos is at unit asterisk *
                _iPos++; // ignore the asterisk
                _iUnitStart = _iPos;

                while ((_iPos <= _iLineLength) && (_pszLine[_iPos] != ')')) {
                    _iPos++;
                }
                if (_iPos <= _iLineLength) {
                    _iUnitLength = _iPos - _iUnitStart;
                    strncpy(_szUnit, _pszLine[_iUnitStart], _iUnitLength);
                } else {
                    // Error
                    _iUnitLength = 0;
                }
            }
        } else {
            // Error
            _iValueLength = 0;
        }
        
        if ((_szTimestamp > 0) && (_iValueLength > 0)) {
            // No formatting depending on type. Timestamped values are never strings.
            sprintf(_szReturnMessage, "{\"timestamp\":\"%s\",\"value\":%s,\"unit\":\"%s\"}", _szTimestamp, _szValue, _szUnit);
        } else {
            // Error
        }
    } else {
        // Error, line did not start with a bracket (
    }

    return _szReturnMessage;
}




char* fnpszParseObisTextValue2JSON(ObisLine* pstObisLine, char* pszDescription){
    static char _szMessage[1024];
    char _szCharacter[2];
    uint8_t i = 0;

    while (i < pstObisLine->iValueLength) {
        itoa((atoi(pstObisLine->szLine+i) * 10) + atoi(pstObisLine->szLine+i+1), _szCharacter, 10);
        strcat(_szMessage, _szCharacter);
        i = i + 2;
    }

    return _szMessage;
}





char* fnParseTelegram(char* psTelegram) {
    static char _szMessage[2048];
    char _szObis[16];
    char _szDescription[32];
    char _szType[8];
    ObisLine _stObisLine;

    strcat (_szMessage, "{");
// for (uint8_t line = 0; line < lines; line++) {
// if line = 0 then different line

//        strcpy(_stObisLine.szLine, _szTelegramLine);
        fnvParseObisLine(&_stObisLine);
        strncpy(_szObis, _stObisLine.szLine, _stObisLine.iIDLength);
        //sprintf(_szMessage, "{\"error\":\"unable to parse: %s\"}", _szObis);

        if (strcmp(_szObis, "1-3:0.2.8")) {
            strcpy(_szDescription, "p1_version");
            strcpy(_szType, "S2");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "0-0:1.0.0")) {
            strcpy(_szDescription, "timestamp");
            strcpy(_szType, "TST");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "0-0:96.1.1")) {
            strcpy(_szDescription, "equipment_id");
            strcpy(_szType, "S96");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:1.8.1")) {
            strcpy(_szDescription, "energy_delivered_tariff1");
            strcpy(_szType, "F9(3,3)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:1.8.2")) {
            strcpy(_szDescription, "energy_delivered_tariff2");
            strcpy(_szType, "F9(3,3)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:2.8.1")) {
            strcpy(_szDescription, "energy_returned_tariff1");
            strcpy(_szType, "F9(3,3)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:2.8.2")) {
            strcpy(_szDescription, "energy_returned_tariff2");
            strcpy(_szType, "F9(3,3)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "0-0:96.14.0")) {
            strcpy(_szDescription, "electricity_tariff");
            strcpy(_szType, "S4");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:1.7.0")) {
            strcpy(_szDescription, "power_delivered");
            strcpy(_szType, "F5(3,3)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:2.7.0")) {
            strcpy(_szDescription, "power_returned");
            strcpy(_szType, "F5(3,3)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "0-0:96.7.21")) {
            strcpy(_szDescription, "electricity_failures");
            strcpy(_szType, "F5(0,0)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "0-0:96.7.9")) {
            strcpy(_szDescription, "electricity_long_failures");
            strcpy(_szType, "F5(0,0)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:99.97.0")) {
            strcpy(_szDescription, "electricity_failure_log");
            strcat(_szMessage, fnpszParseObisMultiValue2JSON(&_stObisLine, _szDescription));
        }
        if (strcmp(_szObis, "1-0:32.32.0")) {
            strcpy(_szDescription, "electricity_sags_l1");
            strcpy(_szType, "F5(0,0)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:52.32.0")) {
            strcpy(_szDescription, "electricity_sags_l2");
            strcpy(_szType, "F5(0,0)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:72.32.0")) {
            strcpy(_szDescription, "electricity_sags_l3");
            strcpy(_szType, "F5(0,0)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:32.36.0")) {
            strcpy(_szDescription, "electricity_swells_l1");
            strcpy(_szType, "F5(0,0)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:52.36.0")) {
            strcpy(_szDescription, "electricity_swells_l2");
            strcpy(_szType, "F5(0,0)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:72.36.0")) {
            strcpy(_szDescription, "electricity_swells_l3");
            strcpy(_szType, "F5(0,0)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "0-0:96.13.0")) {
            strcpy(_szDescription, "message_long");
            strcpy(_szType, "S1024");
            strcat(_szMessage, fnpszParseObisTextValue2JSON(&_stObisLine, _szDescription));
        }
        if (strcmp(_szObis, "1-0:32.7.0")) {
            strcpy(_szDescription, "voltage_l1");
            strcpy(_szType, "F4(1,1)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:52.7.0")) {
            strcpy(_szDescription, "voltage_l2");
            strcpy(_szType, "F4(1,1)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:72.7.0")) {
            strcpy(_szDescription, "voltage_l3");
            strcpy(_szType, "F4(1,1)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:31.7.0")) {
            strcpy(_szDescription, "current_l1");
            strcpy(_szType, "F3(0,0)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:51.7.0")) {
            strcpy(_szDescription, "current_l2");
            strcpy(_szType, "F3(0,0)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:71.7.0")) {
            strcpy(_szDescription, "current_l3");
            strcpy(_szType, "F3(0,0)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:21.7.0")) {
            strcpy(_szDescription, "power_delivered_l1");
            strcpy(_szType, "F5(3,3)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:41.7.0")) {
            strcpy(_szDescription, "power_delivered_l2");
            strcpy(_szType, "F5(3,3)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:61.7.0")) {
            strcpy(_szDescription, "power_delivered_l3");
            strcpy(_szType, "F5(3,3)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:22.7.0")) {
            strcpy(_szDescription, "power_returned_l1");
            strcpy(_szType, "F5(3,3)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:42.7.0")) {
            strcpy(_szDescription, "power_returned_l2");
            strcpy(_szType, "F5(3,3)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "1-0:62.7.0")) {
            strcpy(_szDescription, "power_returned_l3");
            strcpy(_szType, "F5(3,3)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "0-1:24.1.0")) {
            strcpy(_szDescription, "mbus1_device");
            strcpy(_szType, "F3(0,0)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "0-1:96.1.0")) {
            strcpy(_szDescription, "mbus1_equipment_id");
            strcpy(_szType, "S96");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "0-1:24.2.1")) {
            strcpy(_szDescription, "mbus1_value");
            strcat(_szMessage, fnpszParseObisMBUSValue2JSON(&_stObisLine, _szDescription));
        }
        if (strcmp(_szObis, "0-2:24.1.0")) {
            strcpy(_szDescription, "mbus2_device");
            strcpy(_szType, "F3(0,0)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "0-2:96.1.0")) {
            strcpy(_szDescription, "mbus2_equipment_id");
            strcpy(_szType, "S96");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "0-2:24.2.1")) {
            strcpy(_szDescription, "mbus2_value");
            strcat(_szMessage, fnpszParseObisMBUSValue2JSON(&_stObisLine, _szDescription));
        }
        if (strcmp(_szObis, "0-3:24.1.0")) {
            strcpy(_szDescription, "mbus3_device");
            strcpy(_szType, "F3(0,0)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "0-3:96.1.0")) {
            strcpy(_szDescription, "mbus3_equipment_id");
            strcpy(_szType, "S96");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "0-3:24.2.1")) {
            strcpy(_szDescription, "mbus3_value");
            strcat(_szMessage, fnpszParseObisMBUSValue2JSON(&_stObisLine, _szDescription));
        }
        if (strcmp(_szObis, "0-4:24.1.0")) {
            strcpy(_szDescription, "mbus4_device");
            strcpy(_szType, "F3(0,0)");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "0-4:96.1.0")) {
            strcpy(_szDescription, "mbus4_equipment_id");
            strcpy(_szType, "S96");
            strcat(_szMessage, fnpszParseObisSingleValue2JSON(&_stObisLine, _szDescription, _szType));
        }
        if (strcmp(_szObis, "0-4:24.2.1")) {
            strcpy(_szDescription, "mbus4_value");
            strcat(_szMessage, fnpszParseObisMBUSValue2JSON(&_stObisLine, _szDescription));
        }

//  if (line < lines) { szMessage += ","; }
//  }

    strcat(_szMessage, "}");

    return _szMessage;
}
