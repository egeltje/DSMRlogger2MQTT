#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

// -----------------------------------------------------------------
// | 00| 01| 02| 03| 04| 05| 06| 07| 08| 09| 10| 11| 12| 13| 14| 15|
// -----------------------------------------------------------------
// | 1 | : | 2 | ( | 4 | 5 | 6 | . | 7 | 8 | 9 | * | k | W | h | ) |
// -----------------------------------------------------------------

char* fnpszConvertObisTimestamp(char* szObisTimestamp) {
// From Obis YYMMDDhhmmssX to ISO YYYY-MM-DDTHH:MM:SS+01:00"
    static char _szISOTimestamp[32];
    char _szTimezone[7] = "";

    if (strlen(szObisTimestamp) != 13) {

        switch (szObisTimestamp[12]) {
            case 'W':
                strcpy(_szTimezone, "+01:00");
                break;
            case 'S':
                strcpy(_szTimezone, "+02:00");
                break;
            default:
                strcpy(_szTimezone, "+00:00");
                break;
        }

        // This is super evil assuming that the passed Obis timezone is formatted correctly.
        sprintf(_szISOTimestamp, "20%c%c-%c%c-%c%cT%c%c:%c%c:%c%c%s",   
                            szObisTimestamp[0], szObisTimestamp[1], szObisTimestamp[2], szObisTimestamp[3], szObisTimestamp[4], szObisTimestamp[5],
                            szObisTimestamp[6], szObisTimestamp[7], szObisTimestamp[7], szObisTimestamp[9], szObisTimestamp[10], szObisTimestamp[11],
                            _szTimezone);
    }

    return _szISOTimestamp;
}

void fnvParseObisLine(ObisLine* pstObisLine) {
    uint16_t    _iPos = 0;
    pstObisLine->bValid = false;

    while (_iPos <= pstObisLine->iLineLength && (pstObisLine->szLine[_iPos] != '(') ) {
        _iPos++;
    }
    if (_iPos > pstObisLine->iLineLength) {
        // error, value did not start with a (
    } else {
        pstObisLine->iIDStart = 0;
        pstObisLine->iIDLength = _iPos - pstObisLine->iIDStart;     // [3 - 0 = 3]
        // insert OBIS ID format checks here
        pstObisLine->iValueStart = _iPos + 1;   // [3 + 1 = 4]

        while ((_iPos <= pstObisLine->iLineLength) && (pstObisLine->szLine[_iPos] != ')')) {
            _iPos++;
        }
        if (_iPos > pstObisLine->iLineLength) {
            // error, value did not end with a )
        } else {
            pstObisLine->iValueLength = _iPos - pstObisLine->iValueStart; // [15 - 4 = 11]

            _iPos = pstObisLine->iValueStart;
            while ((_iPos <= pstObisLine->iValueStart + pstObisLine->iValueLength) && (pstObisLine->szLine[_iPos] != '*')) {
                _iPos++;
            }
            if (_iPos > pstObisLine->iValueStart + pstObisLine->iValueLength) {
                // No unit in the value
            } else {
                pstObisLine->iUnitStart = _iPos + 1;    // [12]
                pstObisLine->iUnitLength = pstObisLine->iValueStart + pstObisLine->iValueLength - _iPos - 1; // [4 + 11 - 11 - 1 = 3]
                pstObisLine->iValueLength = pstObisLine->iValueLength - pstObisLine->iUnitLength - 1; // 11 - 3 - 1 = 7
            }
            pstObisLine->bValid = true;
        }
    }
}

char* fnpszParseObisSingleValue2JSON(ObisLine* pstObisLine, char* pszDescription, char* pszType) {
    static char _szMessage[128];
    char _szValue[pstObisLine->iValueLength + 1];

    strncpy(_szValue, pstObisLine->szLine + pstObisLine->iValueStart, pstObisLine->iValueLength);
    strcat(_szValue, "\0");

    switch (pszType[0]) {
        case 'F':
            sprintf(_szMessage, "{\"%s\":%f}", pszDescription, strtof(_szValue, NULL));
            break;
        case 'S':
            sprintf(_szMessage, "{\"%s\":\"%s\"}", pszDescription, _szValue);
            break;
        case 'T':
            sprintf(_szMessage, "{\"%s\":\"%s\"}", pszDescription, fnpszConvertObisTimestamp(_szValue));
            break;
        default:
            sprintf(_szMessage, "{\"error\":\"unable to parse value %s with type %s\"}", _szValue, pszType);
            break;
    }

    return _szMessage;
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

char* fnpszParseObisMultiValue2JSON(ObisLine* pstObisLine, char* pszDescription) {
    static char _sMessage[1024];
// (2)(0-0:96.7.19)(101208152415W)(0000000240*s)(101208151004W)(0000000301*s)
// Get first number 
// itterate over every number
//      extract timestamp
//      extract value
// construct JSON string
// {"pszDescription":{"timestamp":[timestamp, timestamp],"value":[value, value]}}

    return _sMessage;
}

char* fnpszParseObisMBUSValue2JSON(ObisLine* pstObisLine, char* pszDescription) {
    static char _szMessage[128];
    char        _szTimestamp[13] = "";
    char        _szValue[16] = "";
    uint16_t    _iPos = pstObisLine->iValueStart + pstObisLine->iValueLength + 1;
    uint16_t    _iValueStart = 0;
    uint16_t    _iValueLength = 0;
    uint16_t    _iUnitStart = 0;
    uint16_t    _iUnitLength = 0;
    
    strncpy(_szTimestamp, pstObisLine->szLine + pstObisLine->iValueStart, pstObisLine->iValueLength);
    strcat(_szTimestamp, "\0");

    while (_iPos <= pstObisLine->iLineLength && (pstObisLine->szLine[_iPos] != '(') ) {
        _iPos++;
    }
    if (_iPos > pstObisLine->iLineLength) {
        // error, value did not start with a (
    } else {
        _iValueStart = _iPos + 1;

        while ((_iPos <= pstObisLine->iLineLength) && (pstObisLine->szLine[_iPos] != ')')) {
            _iPos++;
        }
        if (_iPos > pstObisLine->iLineLength) {
            // error, value did not end with a )
        } else {
            _iValueLength = _iPos - _iValueStart;

            _iPos = _iValueStart;
            while ((_iPos <= _iValueStart + _iValueLength) && (pstObisLine->szLine[_iPos] != '*')) {
                _iPos++;
            }
            if (_iPos > _iValueStart + _iValueLength) {
                // No unit in the value
            } else {
                _iUnitStart = _iPos + 1;
                _iUnitLength = _iValueStart + _iValueLength - _iPos - 1;
                _iValueLength = _iValueLength - _iUnitLength - 1;
            }
            strncpy(_szValue, pstObisLine->szLine + _iValueStart, _iValueLength);
            strcat(_szValue, "\0");
        }
    }

    sprintf(_szMessage, "{\"%s_timestamp\":\"%s\"},{\"%s\":%f}", pszDescription, _szTimestamp, pszDescription, strtof(_szValue, NULL));

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
