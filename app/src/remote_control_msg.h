#ifndef REMOTECONTROLMSG_H
#define REMOTECONTROLMSG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "config.h"
#include "android/input.h"
#include "android/keycodes.h"
#include "common.h"

#define REMOTE_CONTROL_MSG_TEXT_MAX_LENGTH 300
#define REMOTE_CONTROL_MSG_CLIPBOARD_TEXT_MAX_LENGTH 4093
#define REMOTE_CONTROL_MSG_SERIALIZED_MAX_SIZE \
    (3 + REMOTE_CONTROL_MSG_CLIPBOARD_TEXT_MAX_LENGTH)

#define POINTER_ID_MOUSE UINT64_C(-1);

enum remote_control_msg_type {
    REMOTE_CONTROL_MSG_TYPE_INJECT_KEYCODE,
    REMOTE_CONTROL_MSG_TYPE_INJECT_TEXT,
    REMOTE_CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT,
    REMOTE_CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT,
    REMOTE_CONTROL_MSG_TYPE_BACK_OR_SCREEN_ON,
    REMOTE_CONTROL_MSG_TYPE_EXPAND_NOTIFICATION_PANEL,
    REMOTE_CONTROL_MSG_TYPE_COLLAPSE_NOTIFICATION_PANEL,
    REMOTE_CONTROL_MSG_TYPE_GET_CLIPBOARD,
    REMOTE_CONTROL_MSG_TYPE_SET_CLIPBOARD,
    REMOTE_CONTROL_MSG_TYPE_SET_SCREEN_POWER_MODE,
    REMOTE_CONTROL_MSG_TYPE_ROTATE_DEVICE,
};

enum screen_power_mode {
    // see <https://android.googlesource.com/platform/frameworks/base.git/+/pie-release-2/core/java/android/view/SurfaceControl.java#305>
            SCREEN_POWER_MODE_OFF = 0,
    SCREEN_POWER_MODE_NORMAL = 2,
};

struct remote_control_msg {
    enum remote_control_msg_type type;
    union {
        struct {
            enum android_keyevent_action action;
            enum android_keycode keycode;
            enum android_metastate metastate;
        } inject_keycode;
        struct {
            char *text; // owned, to be freed by SDL_free()
        } inject_text;
        struct {
            enum android_motionevent_action action;
            enum android_motionevent_buttons buttons;
            uint64_t pointer_id;
            struct position position;
            float pressure;
        } inject_touch_event;
        struct {
            struct position position;
            int32_t hscroll;
            int32_t vscroll;
        } inject_scroll_event;
        struct {
            char *text; // owned, to be freed by SDL_free()
        } set_clipboard;
        struct {
            enum screen_power_mode mode;
        } set_screen_power_mode;
    };
};

// buf size must be at least REMOTE_CONTROL_MSG_SERIALIZED_MAX_SIZE
// return the number of bytes written
size_t
remote_control_msg_serialize(const struct remote_control_msg *msg, unsigned char *buf);

size_t
remote_control_msg_deserialize(const unsigned char *buf, size_t len,
                        struct remote_control_msg *msg);
void
remote_control_msg_destroy(struct remote_control_msg *msg);

#endif
