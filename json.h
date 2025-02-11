#ifndef JSON_H
#define JSON_H
#define ARRAY_SIZE 10;

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* {
    hello: {},

} */

typedef enum {
    JSON_INT,
    JSON_STRING,
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_BOOL,
    JSON_NULL,
} JSON_TYPE;

typedef enum {
    SQUARE_OPEN_BRACKET,
    SQUARE_CLOSE_BRACKET,
    CURLY_OPEN_BRACKET,
    CURLY_CLOSE_BRACKET,
    ALNUM,
    NUMBER,
    COMMA,
    DOUBLE_POINT,
    SPACE,
    SYMBOL,
    STR,
    ESCAPE,
} TOKEN_TYPE;



/*--------------------------------------------------
 * Vorwärtsdeklarationen
 *--------------------------------------------------*/
/* Diese Deklarationen sind nötig, da z. B. JsonValue
   auf JsonArray und JsonPair verweist, die später definiert werden. */
typedef struct JsonArray JsonArray;
typedef struct JsonPair JsonPair;

/*--------------------------------------------------
 * Strukturdefinitionen
 *--------------------------------------------------*/

/* Ein JSON-String, der einen Ausschnitt des Quell-Strings beschreibt */
typedef struct {
    size_t start;
    size_t end;
    size_t size;
} JsonString;

/* Dynamisches Array – falls du es z. B. für den Parser oder zur internen Verwaltung nutzen möchtest */
typedef struct {
    void *array;
    size_t used;
    size_t size;
} Array;

/* Parser-Struktur, die den aktuellen Zustand des Parsens enthält */
typedef struct {
    char* source;
    char currentChar; 
    size_t currentIndex; 
} json_parser;

/* JsonValue – der zentrale Typ, der einen JSON-Wert (String, Zahl, Objekt, Array etc.) repräsentiert */
typedef struct {
    JSON_TYPE type;
    union {
        JsonString* str;   // Für JSON_STRING
        JsonArray* arr;    // Für JSON_ARRAY
        JsonPair* obj;     // Für JSON_OBJECT (als verkettete Liste von JsonPair)
        int number;        // Für JSON_INT (und evtl. auch JSON_BOOL, z. B. 0 für false, != 0 für true)
    };
} JsonValue;

/* JsonArray – enthält eine Liste von JsonValue-Pointern */
struct JsonArray {
    JsonValue **items;
    size_t count;
    size_t used;
};

/* JsonPair – repräsentiert ein Schlüssel-Wert-Paar in einem JSON-Objekt */
struct JsonPair {
    JsonString* key;
    JsonValue* value;
    JsonPair* parent;  // Optional: Verweis auf das Elternelement (falls benötigt)
    JsonPair* child;   // Zeiger auf das nächste Paar in diesem Objekt
};

/* Top-Level JSON-Objekt (kann als Container für alle Paare dienen) */
typedef struct {
    JsonPair* pairs;
} json;


json* parseJsonFromString(char* jsonString);
void pretty_print_json(const json* j, const char* source);


#endif