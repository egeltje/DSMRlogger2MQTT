#ifndef _PARSER_H_
#define	_PARSER_H_

typedef struct ObisValue_t {
    char        szValue[1024];
    char        szUnit[4];
} ObisValue;

typedef struct ObisLine_t {
    char        szLine[2048];
    uint16_t    iLineStart;
    uint16_t    iLineLength;
    uint16_t    iIDStart;
    uint16_t    iIDLength;
    uint16_t    iValueStart;
    uint16_t    iValueLength;
    uint16_t    iUnitStart;
    uint16_t    iUnitLength;
    bool        bValid;
} ObisLine;

void  fnvParseObisLine(ObisLine* pstObisLine);
char* fnpszParseObisSingleValue2JSON(ObisLine* pstObisLine, char* pszDescription, char* psType);
char* fnpszParseObisTextValue2JSON(ObisLine* pstObisLine, char* pszDescription);
char* fnpszParseObisMultiValue2JSON(ObisLine* pstObisLine, char* pszDescription);
char* fnpszParseObisMBUSValue2JSON(ObisLine* pstObisLine, char* pszDescription);

#endif /* _PARSER_H_ */