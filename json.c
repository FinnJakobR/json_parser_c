#include "json.h"

//--debug print

// Forward-Deklarationen
void pretty_print_value(JsonValue* value, const char* source, int indent);
void pretty_print_object(JsonPair* pair, const char* source, int indent);
void pretty_print_array(JsonArray* arr, const char* source, int indent);
JsonValue* parseJsonValue(json_parser* p);
// Gibt einen JsonValue formatiert aus.
void pretty_print_value(JsonValue* value, const char* source, int indent) {
    if (value == NULL) {
        printf("null");
        return;
    }
    switch (value->type) {
        case JSON_INT:
            printf("%d", value->number);
            break;
        case JSON_STRING: {
            // Hier wird davon ausgegangen, dass value->str->start der Index in source ist
            size_t start = value->str->start;
            size_t size  = value->str->size;
            printf("\"%.*s\"", (int)size, source + start);
            break;
        }
        case JSON_BOOL:
            // Wir gehen davon aus, dass value->number 0 für false und !=0 für true enthält.
            printf(value->number ? "true" : "false");
            break;
        case JSON_NULL:
            printf("null");
            break;
        case JSON_ARRAY:
            pretty_print_array(value->arr, source, indent);
            break;
        case JSON_OBJECT:
            pretty_print_object(value->obj, source, indent);
            break;
        default:
            printf("/* unknown type */");
            break;
    }
}

// Gibt ein JSON-Array formatiert aus.
void pretty_print_array(JsonArray* arr, const char* source, int indent) {
    if (arr == NULL) {
        printf("null");
        return;
    }
    printf("[\n");
    for (size_t i = 0; i < arr->count; i++) {
        pretty_print_value(arr->items[i], source, indent + 2);
        if (i < arr->count - 1) {
            printf(",");
        }
        printf("\n");
    }
    printf("]");
}

// Gibt ein JSON-Objekt (als verkettete Liste von JsonPair) formatiert aus.
void pretty_print_object(JsonPair* pair, const char* source, int indent) {
    printf("{\n");
    JsonPair* current = pair;
    int first = 1;
    while (current != NULL) {
        if (!first) {
            printf(",\n");
        }
        first = 0;
        // Den Schlüssel (als JsonString) ausgeben:
        size_t key_start = current->key->start;
        size_t key_size  = current->key->size;
        printf("\"%.*s\": ", (int)key_size, source + key_start);
        // Den zugehörigen Wert ausgeben:
        pretty_print_value(current->value, source, indent + 2);
        current = current->child;
    }
    printf("\n");
    printf("}");
}

// Top-Level-Funktion: Gibt das gesamte JSON (als Objekt) aus.
void pretty_print_json(const json* j, const char* source) {
    if (j == NULL) {
        printf("null\n");
        return;
    }
    pretty_print_object(j->pairs, source, 0);
}

//---

void initJsonArray(JsonArray *a, size_t initialSize) {
  a->items = malloc(initialSize * sizeof(JsonValue*));
  a->used = 0;
  a->count = initialSize;
}

void insertJsonArray(JsonArray *a, JsonValue* element) {
  if (a->used == a->count) {
    a->count *= 2;
    a->items = realloc(a->items, a->count * sizeof(JsonValue*));
  }
    a->items[a->used++] = element;
}


void nextChar(json_parser* p) {

    p->currentIndex++;
    p->currentChar = p->source[p->currentIndex];

    while (isspace(p->currentChar))
    {
      p->currentIndex++;
        p->currentChar = p->source[p->currentIndex];

    }

    return;
}



TOKEN_TYPE toType(json_parser* p) {


    while (isspace(p->currentChar))
    {
        nextChar(p);
    }
    
    

    switch (p->currentChar)
    {
    case '{':
        return CURLY_OPEN_BRACKET;

    case '}':
        return CURLY_CLOSE_BRACKET;

    case '[':
        return SQUARE_OPEN_BRACKET;
    
    case ']':
        return SQUARE_CLOSE_BRACKET;

    case '\'':
        return STR;

    case ',':
        return COMMA;

    case ':':
        return DOUBLE_POINT;

    case '\\':
        return ESCAPE;

    default:
        if(isdigit(p->currentChar)){
            return NUMBER;
        }

    }    

    return SYMBOL;

}

int expect(TOKEN_TYPE type ,json_parser* p) {
    if(type == toType(p)){
        nextChar(p);
        return 1;
    }

    printf("Expected %d: got %c\n", type, p->currentChar);

    return 0;

}


int isEnd(json_parser* p) {

    return p->currentIndex >= strlen(p->source);

}

int accept(TOKEN_TYPE type, json_parser* p) {

    if(type == toType(p)) {

        nextChar(p);
        return 1;
    }

    return 0;

}


json_parser* init_parser(char* source) {

    json_parser* p = malloc(sizeof(json_parser));
    p->currentChar = source[0];
    p->currentIndex = 0;
    p->source = source;
    return p;
}

int parseJsonNumber(json_parser* p) {
    size_t start = p->currentIndex;
    while (isdigit(p->currentChar)) {
         nextChar(p);
    }

    char *startPtr = p->source + start;
    char *endPtr;

    long num = strtol(startPtr, &endPtr, 10);
    p->currentIndex = endPtr - p->source;

    return (int) num;
}


JsonString* parseJsonString(json_parser* p) {
        
        expect(STR, p);

        size_t start = p->currentIndex;
        size_t end = p->currentIndex;
        
        while ((toType(p) != STR && !isEnd(p)))
        {
            end++;
            
            nextChar(p);

            while(toType(p) == ESCAPE) {
                p->currentIndex+=2;
                p->currentChar = p->source[p->currentIndex];
                end+=2;
            }
        }

        expect(STR, p);

        JsonString* str = malloc(sizeof(JsonString));
        str->end = end;
        str->size = end - start;
        str->start = start;

        return str;
}

JsonArray* parseJsonArray(json_parser* p) {
    expect(SQUARE_OPEN_BRACKET, p);

    JsonArray* arr = malloc(sizeof(JsonArray));
    initJsonArray(arr, 10);

    
    insertJsonArray(arr, parseJsonValue(p));

    while(accept(COMMA, p)){

        insertJsonArray(arr, parseJsonValue(p));
    }

        if (arr->used < arr->count) {
            arr->items = realloc(arr->items, arr->used * sizeof(JsonValue*));
            arr->count = arr->used;
    }
    


    return arr;
}



JsonPair* parseJsonPair(json_parser* p) {
    
    JsonPair* head = malloc(sizeof(JsonPair));
    JsonPair* currentElement = head;
    

    head->key = parseJsonString(p);
    expect(DOUBLE_POINT, p);
    head->value = parseJsonValue(p);


    while (accept(COMMA, p))
    {
        JsonPair* newPair = malloc(sizeof(JsonPair));
        newPair->key = parseJsonString(p);
        expect(DOUBLE_POINT, p);
        
        newPair->value = parseJsonValue(p);
        newPair->child = NULL;
        newPair->parent = currentElement;
        currentElement->child = newPair;

        currentElement = newPair;
    }


    return head;
    
}

JsonValue* parseJsonValue(json_parser* p) {

    JsonValue* value  = malloc(sizeof(JsonValue));

    if(p->currentChar == '{'){
        expect(CURLY_OPEN_BRACKET, p);
        value->type = JSON_OBJECT;
        value->obj = parseJsonPair(p);
        expect(CURLY_CLOSE_BRACKET, p);

    } 

    else if(p->currentChar == '[') {
        value->type = JSON_ARRAY;
        value->arr = parseJsonArray(p);
        expect(SQUARE_CLOSE_BRACKET, p);
    }
    
    else if(p->currentChar == '\'') {
        value->type = JSON_STRING;
        value->str = parseJsonString(p);
    } 
    
    else if(isdigit(p->currentChar)) {
        value->type = JSON_INT;
        value->number = parseJsonNumber(p);
    }

    else if(memcmp(p->source + p->currentIndex, "null", strlen("null")) == 0) {
        value->type = JSON_NULL;
        value->obj = NULL;

        p->currentIndex+= strlen("null");
        p->currentChar = p->source[p->currentIndex];
    }

    else if(memcmp(p->source + p->currentIndex, "true", strlen("true")) == 0){
        value->type = JSON_BOOL;
        value->number = 1;
        p->currentIndex+= strlen("true");
        p->currentChar = p->source[p->currentIndex];
    }

    else if(memcmp(p->source + p->currentIndex , "false", strlen("false")) == 0){
        value->type = JSON_BOOL;
        value->number = 0;
        p->currentIndex+= strlen("false");
        p->currentChar = p->source[p->currentIndex];
    }
    
    else {
        printf("unknown Symbol %c\n", p->currentChar);
    }   

    return value;
}


json* parseJsonFromString(char* JsonString) {
    json_parser* p = init_parser(JsonString);
    json* newJson = malloc(sizeof(json));

    expect(CURLY_OPEN_BRACKET, p);
    newJson->pairs = parseJsonPair(p);
    expect(CURLY_CLOSE_BRACKET, p);

    free(p);

    return newJson;

}