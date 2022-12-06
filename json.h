#ifndef _json_H_
#define _json_H_

/**
 * JSON type identifier. Basic types are:
 *  o Object
 *  o Array
 *  o String
 *  o Other primitive: number, boolean (true/false) or null
 */
typedef enum { JSON_PRIMITIVE, JSON_OBJECT, JSON_ARRAY, JSON_STRING } JsonType;

typedef enum
{
    JSON_ERROR_NOMEM = -1, // Not enough tokens were provided
    JSON_ERROR_INVAL = -2, // Invalid character inside JSON string
    JSON_ERROR_PART = -3, // The string is not a full JSON packet, more bytes expected
    JSON_SUCCESS = 0 // Everthing is fine
} JsonError;

/**
 * JSON token description.
 * @param       type    type (object, array, string etc.)
 * @param       start   start position in JSON data string
 * @param       end     end position in JSON data string
 */
typedef struct
{
    JsonType type;
    int start;
    int end;
    int size;
    #ifdef json_PARENT_LINKS
    int parent;
    #endif
} JsonToken;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string
 */
typedef struct
{
    unsigned int pos; /* offset in the JSON string */
    unsigned int toknext; /* next token to allocate */
    int toksuper; /* superior token node, e.g parent object or array */
} JsonParser;

/**
 * Create JSON parser over an array of tokens
 */
void json_initJsonParser(JsonParser *parser);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each describing
 * a single JSON object.
 */
JsonError json_parseJson(JsonParser *parser, const char *js, JsonToken *tokens, unsigned int tokenNum);

#endif /* _json_H_ */
