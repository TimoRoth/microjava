#include <stdlib.h>
#include <stdio.h>

#include <ringbuffer.h>
#include <sema.h>

#include "events.h"

#define MAX_EVENT_COUNT 32

static mutex_t event_rbuf_mutex = MUTEX_INIT;
static char event_rbuf_buf[MAX_EVENT_COUNT * sizeof(event_t)];
static ringbuffer_t event_rbuf = RINGBUFFER_INIT(event_rbuf_buf);
static sema_t event_rbuf_sema = SEMA_CREATE_LOCKED();


void free_event(event_t *event)
{
    if (!event)
        return;

    for (int i = 0; i < MAX_EVENT_PARAMS; i++) {
        if (event->params[i].type == EPT_String && event->params[i].val.str_val.needs_free)
            free((void*)(event->params[i].val.str_val.str));
        event->params[i].type = EPT_Null;
    }

    event->id = 0;
}

int wait_event(int timeout_us, event_t *event)
{
    printf("Waiting event for %d us\n", timeout_us);

    int sres;
    if (timeout_us >= 0)
        sres = sema_wait_timed(&event_rbuf_sema, timeout_us);
    else
        sres = sema_wait(&event_rbuf_sema);

    if (sres != 0)
        return -1;

    mutex_lock(&event_rbuf_mutex);

    if (event_rbuf.avail < sizeof(event_t)) {
        mutex_unlock(&event_rbuf_mutex);
        return -1;
    }

    ringbuffer_get(&event_rbuf, (char*)event, sizeof(event_t));

    mutex_unlock(&event_rbuf_mutex);

    return 0;
}

int post_event(event_t *event)
{
    mutex_lock(&event_rbuf_mutex);

    if (event_rbuf.size - event_rbuf.avail < sizeof(event_t)) {
        mutex_unlock(&event_rbuf_mutex);
        printf("Event buf overflow!\n");
        return -1;
    }

    ringbuffer_add(&event_rbuf, (char*)event, sizeof(event_t));

    mutex_unlock(&event_rbuf_mutex);

    sema_post(&event_rbuf_sema);

    return 0;
}

int post_event_i(uint8_t id, int val)
{
    event_t event;

    event.id = id;

    event.params[0].type = EPT_Int;
    event.params[0].val.int_val = val;

    for (int i = 1; i < MAX_EVENT_PARAMS; i++)
        event.params[i].type = EPT_Null;

    return post_event(&event);
}

int post_event_ii(uint8_t id, int val1, int val2)
{
    event_t event;

    event.id = id;

    event.params[0].type = EPT_Int;
    event.params[0].val.int_val = val1;

    event.params[1].type = EPT_Int;
    event.params[1].val.int_val = val2;

    for (int i = 2; i < MAX_EVENT_PARAMS; i++)
        event.params[i].type = EPT_Null;

    return post_event(&event);
}

int post_event_cs(uint8_t id, const char *str)
{
    event_t event;

    event.id = id;

    event.params[0].type = EPT_String;
    event.params[0].val.str_val.str = str;
    event.params[0].val.str_val.needs_free = false;

    for (int i = 1; i < MAX_EVENT_PARAMS; i++)
        event.params[i].type = EPT_Null;

    return post_event(&event);
}

int post_event_s(uint8_t id, const char *str)
{
    event_t event;

    event.id = id;

    event.params[0].type = EPT_String;
    event.params[0].val.str_val.str = str;
    event.params[0].val.str_val.needs_free = true;

    for (int i = 1; i < MAX_EVENT_PARAMS; i++)
        event.params[i].type = EPT_Null;

    return post_event(&event);
}

int post_event_csi(uint8_t id, const char *str, int val)
{
    event_t event;

    event.id = id;

    event.params[0].type = EPT_String;
    event.params[0].val.str_val.str = str;
    event.params[0].val.str_val.needs_free = false;

    event.params[1].type = EPT_Int;
    event.params[1].val.int_val = val;

    for (int i = 2; i < MAX_EVENT_PARAMS; i++)
        event.params[i].type = EPT_Null;

    return post_event(&event);
}

int post_event_si(uint8_t id, const char *str, int val)
{
    event_t event;

    event.id = id;

    event.params[0].type = EPT_String;
    event.params[0].val.str_val.str = str;
    event.params[0].val.str_val.needs_free = true;

    event.params[1].type = EPT_Int;
    event.params[1].val.int_val = val;

    for (int i = 2; i < MAX_EVENT_PARAMS; i++)
        event.params[i].type = EPT_Null;

    return post_event(&event);
}
