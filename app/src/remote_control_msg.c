#include "remote_control_msg.h"
#include "control_msg.h"
#include <assert.h>
#include <string.h>

#include "config.h"
#include "util/buffer_util.h"
#include "util/log.h"
#include "util/str_util.h"
#include "util/json.h"



char *my_str = "{ \
'glossary': { \
'title': 'example glossary', \
'GlossDiv': { \
'title': 'S', \
'GlossList': { \
'GlossEntry': { \
'ID': 'SGML', \
'SortAs': 'SGML', \
'GlossTerm': 'Standard Generalized Markup Language', \
'Acronym': 'SGML', \
'Abbrev': 'ISO 8879:1986', \
'GlossDef': { \
'para': 'A meta-markup language, used to create markup languages such as DocBook.', \
'GlossSeeAlso': ['GML', 'XML'] \
}, \
'GlossSee': 'markup' \
} \
} \
} \
} \
}";

// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

static void print_depth_shift(int depth)
{
    int j;
    for (j=0; j < depth; j++) {
        printf(" ");
    }
}

static void process_value(json_value* value, int depth);

static void process_object(json_value* value, int depth)
{
    int length, x;
    if (value == NULL) {
        return;
    }
    length = value->u.object.length;
    for (x = 0; x < length; x++) {
        print_depth_shift(depth);
        printf("object[%d].name = %s\n", x, value->u.object.values[x].name);
        process_value(value->u.object.values[x].value, depth+1);
    }
}

static void process_array(json_value* value, int depth)
{
    int length, x;
    if (value == NULL) {
        return;
    }
    length = value->u.array.length;
    printf("array\n");
    for (x = 0; x < length; x++) {
        process_value(value->u.array.values[x], depth);
    }
}

static void process_value(json_value* value, int depth)
{
    int j;
    if (value == NULL) {
        return;
    }
    if (value->type != json_object) {
        print_depth_shift(depth);
    }
    switch (value->type) {
        case json_none:
            printf("none\n");
            break;
        case json_object:
            process_object(value, depth+1);
            break;
        case json_array:
            process_array(value, depth+1);
            break;
        case json_integer:
            printf("int: %10" PRId64 "\n", value->u.integer);
            break;
        case json_double:
            printf("double: %f\n", value->u.dbl);
            break;
        case json_string:
            printf("string: %s\n", value->u.string.ptr);
            break;
        case json_boolean:
            printf("bool: %d\n", value->u.boolean);
            break;
    }
}

size_t
remote_control_msg_deserialize(const unsigned char *buf, size_t len,
                               struct control_msg *msg) {
    if (len < 3) {
        // at least type + empty string length
        return 0; // not available
    }

    char *rep="'";
    char *with="\"";

    char *json_str = str_replace(my_str,rep,with);



    json_value *json_obj=json_parse(json_str,strlen(json_str));


    if (json_obj!=NULL){
        process_value(json_obj, 0);
    }else{
        LOGI("NULL json obj");
    }


    json_value_free (json_obj);
    free(json_str);

    msg->type = buf[0];
    switch (msg->type) {

        default:
            LOGW("Unknown remote control message type: %d", (int) msg->type);
            return 0; // error, we cannot recover
    }
}

