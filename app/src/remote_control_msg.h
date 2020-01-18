#ifndef REMOTECONTROLMSG_H
#define REMOTECONTROLMSG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "config.h"
#include "android/input.h"
#include "android/keycodes.h"
#include "common.h"
#include "control_msg.h"


size_t
remote_control_msg_deserialize(const unsigned char *buf, size_t len,
                        struct control_msg *msg);

#endif
