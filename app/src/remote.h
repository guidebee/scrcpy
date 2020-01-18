#ifndef REMOTE_H
#define REMOTE_H

#include <stdbool.h>
#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL_thread.h>

#include "config.h"
#include "util/net.h"

// receive events from the device
// managed by the controller
struct remote {
    socket_t control_socket;
    SDL_Thread *thread;
    SDL_mutex *mutex;

};

bool
remote_init(struct remote *remote, socket_t control_socket);

void
remote_destroy(struct remote *remote);

bool
remote_start(struct remote *remote);

// no remote_stop(), it will automatically stop on control_socket shutdown

void
remote_join(struct remote *remote);

#endif
