#include "remote.h"

#include <assert.h>


#include "config.h"
#include "control_msg.h"
#include "util/lock.h"
#include "util/log.h"
#include "util/net.h"



//#define IPV4_LOCALHOST 0x7F000001
//static socket_t
//listen_on_port(uint16_t port) {
//    return net_listen(IPV4_LOCALHOST, port, 1);
//}
static void
close_socket(socket_t *socket) {
    assert(*socket != INVALID_SOCKET);
    net_shutdown(*socket, SHUT_RDWR);
    if (!net_close(*socket)) {
        LOGW("Could not close socket");
        return;
    }
    *socket = INVALID_SOCKET;
}


bool
remote_init(struct remote *remote, socket_t control_socket, socket_t client_socket) {
    if (!(remote->mutex = SDL_CreateMutex())) {
        return false;
    }
    remote->control_socket = control_socket;
    remote->remote_client_socket=client_socket;
    return true;
}

bool remote_reboot_client_socket(struct remote *remote){
    if (remote->remote_client_socket != INVALID_SOCKET) {
        close_socket(&remote->remote_client_socket);

    }
    remote->remote_client_socket = net_accept(remote->control_socket);
    if (remote->remote_client_socket == INVALID_SOCKET) {
        return 0;
    }
    return 1;
}

void
remote_destroy(struct remote *remote) {
    SDL_DestroyMutex(remote->mutex);


}

static void
process_msg(struct control_msg *msg) {
//    switch (msg->type) {
//        case DEVICE_MSG_TYPE_CLIPBOARD:
//            
//            SDL_SetClipboardText(msg->clipboard.text);
//            break;
//    }
    LOGI("Remote control message received");
}

static ssize_t
process_msgs(const unsigned char *buf, size_t len) {
    size_t head = 0;
    for (;;) {
        struct control_msg msg;
        ssize_t r = control_msg_deserialize(&buf[head], len - head, &msg);
        if (r == -1) {
            return -1;
        }
        if (r == 0) {
            return head;
        }

        process_msg(&msg);
        control_msg_destroy(&msg);

        head += r;
        assert(head <= len);
        if (head == len) {
            return head;
        }
    }
}

static int
run_remote(void *data) {
    struct remote *remote = data;

    unsigned char buf[CONTROL_MSG_SERIALIZED_MAX_SIZE];
    size_t head = 0;

    remote->remote_client_socket = net_accept(remote->control_socket);
    if (remote->remote_client_socket == INVALID_SOCKET) {

        return 0;
    }

    for (;;) {
        assert(head < CONTROL_MSG_SERIALIZED_MAX_SIZE);
        ssize_t r = net_recv(remote->remote_client_socket, buf,
                             CONTROL_MSG_SERIALIZED_MAX_SIZE - head);
        if (r <= 0 ) {

            remote->remote_client_socket=INVALID_SOCKET;
            LOGD("Remote Stopped, restarting");
            if(!remote_reboot_client_socket(remote)) {
                break;
            }
        }

        ssize_t consumed = process_msgs(buf, r);
        if (consumed == -1) {
            // an error occurred
            break;
        }

        if (consumed) {
            // shift the remaining data in the buffer
            memmove(buf, &buf[consumed], r - consumed);
            head = r - consumed;
        }
    }

    return 0;
}

bool
remote_start(struct remote *remote) {
    LOGD("Starting remote thread");

    remote->thread = SDL_CreateThread(run_remote, "remote", remote);
    if (!remote->thread) {
        LOGC("Could not start remote thread");
        return false;
    }

    return true;
}

void
remote_join(struct remote *remote) {
    SDL_WaitThread(remote->thread, NULL);
}
