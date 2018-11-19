#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MAX_EVENT_PARAMS 2

typedef enum
{
    EPT_Null = 0,
    EPT_Int,
    EPT_String,
} event_param_type_t;

typedef struct event_param_t
{
    event_param_type_t type;
    union {
        struct {
            const char *str;
            bool needs_free;
        } str_val;
        int int_val;
    } val;
} event_param_t;

#define EVT_NONE 0x00
#define EVT_GPIO 0x01
#define EVT_EXIT 0xFF

typedef struct event_t
{
    uint8_t id;
    event_param_t params[MAX_EVENT_PARAMS];
} event_t;

void free_event(event_t *event);
int wait_event(int timeout_us, event_t *event);
int post_event(event_t *event);

// Helper to post an event with just int parameters
int post_event_i(uint8_t id, int val);
int post_event_ii(uint8_t id, int val1, int val2);

// Helpers to post an event with one string parameter
// Const strings (_cs* variants) won't be freed
int post_event_cs(uint8_t id, const char *str);
int post_event_s(uint8_t id, const char *str);

// Helpers to post an event with one string and one int parameter
// Const strings (_cs* variants) won't be freed
int post_event_csi(uint8_t id, const char *str, int val);
int post_event_si(uint8_t id, const char *str, int val);
