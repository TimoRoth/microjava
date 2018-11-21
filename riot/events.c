#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <ringbuffer.h>
#include <xtimer.h>
#include <sema.h>
#include <msg.h>

#include "events.h"


static msg_t msg_queue[1 << 5]; // size must be power of two
static kernel_pid_t event_thread_pid = -1;
static msg_t last_msg = { 0 };

static event_t *make_event_raw(uint8_t id, uint8_t num_params);

void init_events(void)
{
    msg_init_queue(msg_queue, sizeof(msg_queue) / sizeof(msg_queue[0]));
    event_thread_pid = thread_getpid();
}

kernel_pid_t get_event_pid(void)
{
    return event_thread_pid;
}

void free_event(event_t **eventP)
{
    event_t *event = *eventP;

    if (!event)
        return;

    for (int i = 0; i < event->num_params; i++) {
        if (event->params[i].type == EPT_String && event->params[i].val.str_val.needs_free)
            free((void*)(event->params[i].val.str_val.str));
        event->params[i].type = EPT_Null;
    }

    free(event);
    *eventP = NULL;
}

event_t *wait_event(int timeout_us)
{
    if (thread_getpid() != event_thread_pid) {
        printf("Wait from invalid thread!\n");
        return NULL;
    }

    printf("Waiting event for %d us\n", timeout_us);

    int sres;
    if (timeout_us == 0)
        sres = (msg_try_receive(&last_msg) < 0) ? 0 : 1;
    else if (timeout_us > 0)
        sres = (xtimer_msg_receive_timeout(&last_msg, timeout_us) < 0) ? 0 : 1;
    else
        sres = msg_receive(&last_msg);

    if (!sres)
        return NULL;

    event_t *event = NULL;

    if (last_msg.type == EVT_MSG_TYPE) {
        event = last_msg.content.ptr;
    } else {
        event = make_event_raw(EVT_GENERIC, 1);
        if (!event)
            return NULL;

        event->params[0].type = EPT_Raw;
        event->params[0].val.raw_val.type = last_msg.type;
        memcpy(&(event->params[0].val.raw_val.content), &(last_msg.content), sizeof(last_msg.content));
    }

    event->sender_pid = last_msg.sender_pid;

    return event;
}

int post_event(event_t *event)
{
    msg_t msg = {
        .type = EVT_MSG_TYPE,
        .content.ptr = event,
    };

    int res = msg_send(&msg, event_thread_pid);

    return res - 1; // msg_send returns 1 on success, 0 or -1 otherwise, so we make it so 0 is success, rest <0
}

int reply_last_event(event_t *event)
{
    msg_t msg = {
        .type = EVT_MSG_TYPE,
        .content.ptr = event,
    };

    int res = msg_reply(&last_msg, &msg);

    return res - 1; // similar to return of post_event
}

static event_t *make_event_raw(uint8_t id, uint8_t num_params)
{
    event_t *event = malloc(sizeof(event_t) + (sizeof(event_param_t) * num_params));
    event->id = id;
    event->num_params = num_params;
    if (num_params)
        event->params = (event_param_t*)(((uint8_t*)event) + sizeof(event_t));
    else
        event->params = NULL;
    return event;
}

event_t *make_event(uint8_t id)
{
    return make_event_raw(id, 0);
}

event_t *make_event_i(uint8_t id, int val)
{
    event_t *event = make_event_raw(id, 1);

    event->params[0].type = EPT_Int;
    event->params[0].val.int_val = val;

    return event;
}

event_t *make_event_ii(uint8_t id, int val1, int val2)
{
    event_t *event = make_event_raw(id, 2);

    event->params[0].type = EPT_Int;
    event->params[0].val.int_val = val1;

    event->params[1].type = EPT_Int;
    event->params[1].val.int_val = val2;

    return event;
}

event_t *make_event_cs(uint8_t id, const char *str)
{
    event_t *event = make_event_raw(id, 1);

    event->params[0].type = EPT_String;
    event->params[0].val.str_val.str = str;
    event->params[0].val.str_val.needs_free = false;

    return event;
}

event_t *make_event_s(uint8_t id, const char *str)
{
    event_t *event = make_event_raw(id, 1);

    event->params[0].type = EPT_String;
    event->params[0].val.str_val.str = str;
    event->params[0].val.str_val.needs_free = true;

    return event;
}

event_t *make_event_csi(uint8_t id, const char *str, int val)
{
    event_t *event = make_event_raw(id, 2);

    event->params[0].type = EPT_String;
    event->params[0].val.str_val.str = str;
    event->params[0].val.str_val.needs_free = false;

    event->params[1].type = EPT_Int;
    event->params[1].val.int_val = val;

    return event;
}

event_t *make_event_si(uint8_t id, const char *str, int val)
{
    event_t *event = make_event_raw(id, 2);

    event->params[0].type = EPT_String;
    event->params[0].val.str_val.str = str;
    event->params[0].val.str_val.needs_free = true;

    event->params[1].type = EPT_Int;
    event->params[1].val.int_val = val;

    return event;
}
