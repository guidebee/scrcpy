#include "remote_control_msg.h"
#include "control_msg.h"
#include <assert.h>
#include <string.h>

#include "config.h"
#include "util/buffer_util.h"
#include "util/log.h"
#include "util/str_util.h"
#include "util/json.h"


size_t
remote_control_msg_deserialize(const unsigned char *buf, size_t len,
                               struct control_msg *msg) {
    if (len < 3) {
        // at least type + empty string length
        return 0; // not available
    }

    json_value *json_obj = json_parse(buf, len);

    if (json_obj != NULL) {
        process_value(json_obj, 0);
    } else {
        LOGI("NULL json obj");
    }

    json_value_free(json_obj);

    msg->type = buf[0];
    switch (msg->type) {

        default:
            LOGW("Unknown remote control message type: %d", (int) msg->type);
            return 0; // error, we cannot recover
    }
}

