#include "remote_control_msg.h"
#include "control_msg.h"
#include <assert.h>
#include <string.h>
#include <src/util/json.h>

#include "config.h"
#include "util/buffer_util.h"
#include "util/log.h"
#include "util/str_util.h"
#include "util/json.h"

json_value *get_key_object(json_value *value, char *key) {
    if (value != NULL) {
        int length = value->u.object.length;
        int x;
        for (x = 0; x < length; x++) {

            if (strncmp(value->u.object.values[x].name, key, strlen(key)) == 0) {
                return value->u.object.values[x].value;

            }
        }
    }
    return NULL;
}

char *get_key_value(json_value *value, char *key) {
    if (value != NULL) {
        int length = value->u.object.length;
        int x;
        for (x = 0; x < length; x++) {

            if (strncmp(value->u.object.values[x].name, key, strlen(key)) == 0) {
                return value->u.object.values[x].value->u.string.ptr;

            }
        }
    }
    return "";
}

char *get_message_type(json_value *value) {
    return get_key_value(value, "msg_type");
}

size_t
remote_control_msg_deserialize(const unsigned char *buf, size_t len,
                               struct control_msg *msg) {
    if (len < 3) {
        // at least type + empty string length
        return 0; // not available
    }
    json_value *value = json_parse(buf, len);
    if (value != NULL) {
        char *msg_type = get_message_type(value);
        if (strcmp(msg_type, "CONTROL_MSG_TYPE_INJECT_KEYCODE") == 0) {
            msg->type = CONTROL_MSG_TYPE_INJECT_KEYCODE;
        } else if (strcmp(msg_type, "CONTROL_MSG_TYPE_INJECT_TEXT") == 0) {
            msg->type = CONTROL_MSG_TYPE_INJECT_TEXT;
        } else if (strcmp(msg_type, "CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT") == 0) {
            msg->type = CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT;
        } else if (strcmp(msg_type, "CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT") == 0) {
            msg->type = CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT;
        } else if (strcmp(msg_type, "CONTROL_MSG_TYPE_BACK_OR_SCREEN_ON") == 0) {
            msg->type = CONTROL_MSG_TYPE_BACK_OR_SCREEN_ON;
        } else if (strcmp(msg_type, "CONTROL_MSG_TYPE_EXPAND_NOTIFICATION_PANEL") == 0) {
            msg->type = CONTROL_MSG_TYPE_EXPAND_NOTIFICATION_PANEL;
        } else if (strcmp(msg_type, "CONTROL_MSG_TYPE_COLLAPSE_NOTIFICATION_PANEL") == 0) {
            msg->type = CONTROL_MSG_TYPE_COLLAPSE_NOTIFICATION_PANEL;
        } else if (strcmp(msg_type, "CONTROL_MSG_TYPE_GET_CLIPBOARD") == 0) {
            msg->type = CONTROL_MSG_TYPE_GET_CLIPBOARD;
        } else if (strcmp(msg_type, "CONTROL_MSG_TYPE_SET_CLIPBOARD") == 0) {
            msg->type = CONTROL_MSG_TYPE_SET_CLIPBOARD;
        } else if (strcmp(msg_type, "CONTROL_MSG_TYPE_SET_SCREEN_POWER_MODE") == 0) {
            msg->type = CONTROL_MSG_TYPE_SET_SCREEN_POWER_MODE;
        } else if (strcmp(msg_type, "CONTROL_MSG_TYPE_ROTATE_DEVICE") == 0) {
            msg->type = CONTROL_MSG_TYPE_ROTATE_DEVICE;
        } else /* default: */
        {
            msg->type = 9999;
        }
        size_t ret = len;

        switch (msg->type) {

            case CONTROL_MSG_TYPE_INJECT_KEYCODE:
                LOGD("CONTROL_MSG_TYPE_INJECT_KEYCODE: %d", (int) msg->type);
                break;
            case CONTROL_MSG_TYPE_INJECT_TEXT:
                LOGD("CONTROL_MSG_TYPE_INJECT_TEXT: %d", (int) msg->type);
                json_value *inject_text=get_key_object(value,"inject_text");
                if (inject_text!=NULL) {
                    char *message = get_key_value(inject_text,"text");
                    int clipboard_len = strlen(message);
                    char *text = SDL_malloc(clipboard_len + 1);
                    memcpy(text, message, clipboard_len);
                    text[clipboard_len] = '\0';
                    msg->inject_text.text = text;
                }
                break;
            case CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT:
                LOGW("CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT: %d", (int) msg->type);

                break;
            case CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT:
                LOGW("CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT: %d", (int) msg->type);
                break;
            case CONTROL_MSG_TYPE_BACK_OR_SCREEN_ON:
                LOGW("CONTROL_MSG_TYPE_BACK_OR_SCREEN_ON: %d", (int) msg->type);
                break;
            case CONTROL_MSG_TYPE_EXPAND_NOTIFICATION_PANEL:
                LOGW("CONTROL_MSG_TYPE_EXPAND_NOTIFICATION_PANEL: %d", (int) msg->type);
                break;
            case CONTROL_MSG_TYPE_COLLAPSE_NOTIFICATION_PANEL:
                LOGW("CONTROL_MSG_TYPE_COLLAPSE_NOTIFICATION_PANEL: %d", (int) msg->type);
                break;
            case CONTROL_MSG_TYPE_GET_CLIPBOARD:
                LOGW("CONTROL_MSG_TYPE_GET_CLIPBOARD: %d", (int) msg->type);
                break;

            case CONTROL_MSG_TYPE_SET_CLIPBOARD:
                LOGW("CONTROL_MSG_TYPE_SET_CLIPBOARD: %d", (int) msg->type);
                break;
            case CONTROL_MSG_TYPE_SET_SCREEN_POWER_MODE:
                LOGW("CONTROL_MSG_TYPE_SET_SCREEN_POWER_MODE: %d", (int) msg->type);
                break;
            case CONTROL_MSG_TYPE_ROTATE_DEVICE:
                LOGW("CONTROL_MSG_TYPE_ROTATE_DEVICE: %d", (int) msg->type);
                break;
            default:
                LOGW("Unknown remote control message type: %d", (int) msg->type);
                ret = 0; // error, we cannot recover
        }

        json_value_free(value);
        return ret;
    } else {
        LOGI("NULL json obj");
        return 0;
    }
}

