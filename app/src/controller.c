#include "controller.h"

#include <assert.h>

#include "config.h"
#include "util/lock.h"
#include "util/log.h"
#include "control_msg.h"

bool
controller_init(struct controller *controller, socket_t control_socket, socket_t remote_control_socket,
                socket_t remote_client_socket) {
    cbuf_init(&controller->queue);

    if (!receiver_init(&controller->receiver, control_socket)) {
        return false;
    }

    if (!remote_init(&controller->remote, remote_control_socket, remote_client_socket, controller)) {
        return false;
    }

    if (!(controller->mutex = SDL_CreateMutex())) {
        receiver_destroy(&controller->receiver);
        remote_destroy(&controller->remote);
        return false;
    }

    if (!(controller->msg_cond = SDL_CreateCond())) {
        receiver_destroy(&controller->receiver);
        remote_destroy(&controller->remote);
        SDL_DestroyMutex(controller->mutex);
        return false;
    }

    controller->control_socket = control_socket;
    controller->stopped = false;
    controller->fp_events = NULL;

    return true;
}

void
controller_destroy(struct controller *controller) {
    SDL_DestroyCond(controller->msg_cond);
    SDL_DestroyMutex(controller->mutex);

    struct control_msg msg;
    while (cbuf_take(&controller->queue, &msg)) {
        control_msg_destroy(&msg);
    }

    receiver_destroy(&controller->receiver);
    remote_destroy(&controller->remote);

}

char *to_json(const struct control_msg *msg) {
    char *buffer =SDL_malloc(CONTROL_MSG_SERIALIZED_MAX_SIZE);
    if(buffer!=NULL) {
        char temp[256];
        strcpy(buffer, "{\n");
        switch (msg->type) {
            case CONTROL_MSG_TYPE_INJECT_KEYCODE: {
                sprintf(temp, "    \"msg_type\": \"%s\",\n", "CONTROL_MSG_TYPE_INJECT_KEYCODE");
                strcat(buffer, temp);
                sprintf(temp, "    \"key_code\":{\n");
                strcat(buffer, temp);
                sprintf(temp, "        \"action\":%d,\n", msg->inject_keycode.action);
                strcat(buffer, temp);
                sprintf(temp, "        \"key_code\":%d,\n", msg->inject_keycode.keycode);
                strcat(buffer, temp);
                sprintf(temp, "        \"meta_state\":%d\n", msg->inject_keycode.metastate);
                strcat(buffer, temp);
                strcat(buffer, "    }\n");
            }
                break;
            case CONTROL_MSG_TYPE_INJECT_TEXT: {
                sprintf(temp, "    \"msg_type\": \"%s\",\n", "CONTROL_MSG_TYPE_INJECT_TEXT");
                strcat(buffer, temp);
                sprintf(temp, "    \"inject_text\":{\n");
                strcat(buffer, temp);
                sprintf(temp, "        \"text\":%s\n", msg->inject_text.text);
                strcat(buffer, temp);
                strcat(buffer, "    }\n");
            }
                break;
            case CONTROL_MSG_TYPE_EXPAND_NOTIFICATION_PANEL: {
                sprintf(temp, "    \"msg_type\": \"%s\"\n", "CONTROL_MSG_TYPE_EXPAND_NOTIFICATION_PANEL");
                strcat(buffer, temp);
            }
                break;
            case CONTROL_MSG_TYPE_COLLAPSE_NOTIFICATION_PANEL: {
                sprintf(temp, "    \"msg_type\": \"%s\"\n", "CONTROL_MSG_TYPE_COLLAPSE_NOTIFICATION_PANEL");
                strcat(buffer, temp);
            }
                break;
            case CONTROL_MSG_TYPE_ROTATE_DEVICE: {
                sprintf(temp, "    \"msg_type\": \"%s\"\n", "CONTROL_MSG_TYPE_ROTATE_DEVICE");
                strcat(buffer, temp);
            }
                break;

            case CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT: {
                sprintf(temp, "    \"msg_type\": \"%s\",\n", "CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT");
                strcat(buffer, temp);
                sprintf(temp, "    \"touch_event\":{\n");
                strcat(buffer, temp);
                sprintf(temp, "        \"action\":%d,\n", msg->inject_touch_event.action);
                strcat(buffer, temp);
                sprintf(temp, "        \"buttons\":%d,\n", msg->inject_touch_event.buttons);
                strcat(buffer, temp);
                sprintf(temp, "        \"pointer\":%lld,\n", msg->inject_touch_event.pointer_id);
                strcat(buffer, temp);
                sprintf(temp, "        \"pressure\":%f,\n", msg->inject_touch_event.pressure);
                strcat(buffer, temp);
                sprintf(temp, "        \"position\":{\n");
                strcat(buffer, temp);
                sprintf(temp, "            \"screen_size\": {\n");
                strcat(buffer, temp);
                sprintf(temp, "                \"width\": %d,\n", msg->inject_touch_event.position.screen_size.width);
                strcat(buffer, temp);
                sprintf(temp, "                \"height\": %d\n", msg->inject_touch_event.position.screen_size.height);
                strcat(buffer, temp);
                strcat(buffer, "            },\n");
                sprintf(temp, "            \"point\": {\n");
                strcat(buffer, temp);
                sprintf(temp, "                \"x\": %d,\n", msg->inject_touch_event.position.point.x);
                strcat(buffer, temp);
                sprintf(temp, "                \"y\": %d\n", msg->inject_touch_event.position.point.y);
                strcat(buffer, temp);

                strcat(buffer, "            }\n");
                strcat(buffer, "        }\n");
                strcat(buffer, "    }\n");
            }
                break;
            case CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT: {

                sprintf(temp, "    \"msg_type\": \"%s\",\n", "CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT");
                strcat(buffer, temp);
                sprintf(temp, "    \"scroll_event\":{\n");
                strcat(buffer, temp);
                sprintf(temp, "        \"h_scroll\":%d,\n", msg->inject_scroll_event.hscroll);
                strcat(buffer, temp);
                sprintf(temp, "        \"v_scroll\":%d,\n", msg->inject_scroll_event.vscroll);
                strcat(buffer, temp);
                sprintf(temp, "        \"position\":{\n");
                strcat(buffer, temp);
                sprintf(temp, "            \"screen_size\": {\n");
                strcat(buffer, temp);
                sprintf(temp, "                \"width\": %d,\n", msg->inject_touch_event.position.screen_size.width);
                strcat(buffer, temp);
                sprintf(temp, "                \"height\": %d\n", msg->inject_touch_event.position.screen_size.height);
                strcat(buffer, temp);
                strcat(buffer, "            },\n");
                sprintf(temp, "            \"point\": {\n");
                strcat(buffer, temp);
                sprintf(temp, "                \"x\": %d,\n", msg->inject_touch_event.position.point.x);
                strcat(buffer, temp);
                sprintf(temp, "                \"y\": %d\n", msg->inject_touch_event.position.point.y);
                strcat(buffer, temp);

                strcat(buffer, "            }\n");
                strcat(buffer, "        }\n");
                strcat(buffer, "    }\n");
            }
                break;
            default:
                break;

        }
        strcat(buffer, "},\n");
        return buffer;
    }
    return NULL;
}

bool
controller_push_msg(struct controller *controller,
                    const struct control_msg *msg) {
    mutex_lock(controller->mutex);
    bool was_empty = cbuf_is_empty(&controller->queue);
    bool res = cbuf_push(&controller->queue, *msg);
    FILE *fp = controller->fp_events;
    if (fp != NULL) {
        char *json = to_json(msg);
        if(json!=NULL) {
            fprintf(fp, "%s", to_json(msg));
            SDL_free(json);
        }
    }
    if (was_empty) {
        cond_signal(controller->msg_cond);
    }
    mutex_unlock(controller->mutex);
    return res;
}

static bool
process_msg(struct controller *controller,
            const struct control_msg *msg) {
    unsigned char serialized_msg[CONTROL_MSG_SERIALIZED_MAX_SIZE];
    int length = control_msg_serialize(msg, serialized_msg);
    if (!length) {
        return false;
    }
    int w = net_send_all(controller->control_socket, serialized_msg, length);
    return w == length;
}

static int
run_controller(void *data) {
    struct controller *controller = data;

    for (;;) {
        mutex_lock(controller->mutex);
        while (!controller->stopped && cbuf_is_empty(&controller->queue)) {
            cond_wait(controller->msg_cond, controller->mutex);
        }
        if (controller->stopped) {
            // stop immediately, do not process further msgs
            mutex_unlock(controller->mutex);
            break;
        }
        struct control_msg msg;
        bool non_empty = cbuf_take(&controller->queue, &msg);
        assert(non_empty);
        (void) non_empty;
        mutex_unlock(controller->mutex);

        bool ok = process_msg(controller, &msg);
        control_msg_destroy(&msg);
        if (!ok) {
            LOGD("Could not write msg to socket");
            break;
        }
    }
    return 0;
}

bool
controller_start(struct controller *controller) {
    LOGD("Starting controller thread");

    controller->thread = SDL_CreateThread(run_controller, "controller",
                                          controller);
    if (!controller->thread) {
        LOGC("Could not start controller thread");
        return false;
    }

    if (!receiver_start(&controller->receiver)) {
        controller_stop(controller);
        SDL_WaitThread(controller->thread, NULL);
        return false;
    }

    if (!remote_start(&controller->remote)) {
        controller_stop(controller);
        SDL_WaitThread(controller->thread, NULL);
        return false;
    }

    return true;
}

void
controller_stop(struct controller *controller) {
    mutex_lock(controller->mutex);
    controller->stopped = true;
    cond_signal(controller->msg_cond);
    mutex_unlock(controller->mutex);
}

void
controller_join(struct controller *controller) {
    SDL_WaitThread(controller->thread, NULL);
    receiver_join(&controller->receiver);
    remote_join(&controller->remote);
}


void
controller_start_recording(struct controller *controller){

    FILE *fp = controller->fp_events;
    if (fp != NULL) {
        fclose(fp);

    }
    LOGI("Start recording...");
    controller->fp_events = fopen("saved_event.json", "w");
}

void
controller_stop_recording(struct controller *controller){
    FILE *fp = controller->fp_events;
    if (fp != NULL) {
        LOGI("Stop recording");
        fflush(fp);
        fclose(fp);
        controller->fp_events = NULL;
    }
}