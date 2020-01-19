#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdbool.h>
#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL_thread.h>

#include "config.h"
#include "control_msg.h"
#include "receiver.h"
#include "remote.h"
#include "util/cbuf.h"
#include "util/net.h"

struct control_msg_queue CBUF(struct control_msg, 64);

struct controller {
    socket_t control_socket;
    SDL_Thread *thread;
    SDL_mutex *mutex;
    SDL_cond *msg_cond;
    bool stopped;
    FILE *fp_events;
    struct control_msg_queue queue;
    struct receiver receiver;
    struct remote remote;
};

bool
controller_init(struct controller *controller, socket_t control_socket,
        socket_t remote_control_socket,socket_t remote_client_socket);

void
controller_destroy(struct controller *controller);

bool
controller_start(struct controller *controller);

void
controller_stop(struct controller *controller);

void
controller_start_recording(struct controller *controller);

void
controller_stop_recording(struct controller *controller);

void
controller_join(struct controller *controller);

bool
controller_push_msg(struct controller *controller,
                    const struct control_msg *msg);

#endif
