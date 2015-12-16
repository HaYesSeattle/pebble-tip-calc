#pragma once

#include <pebble.h>
#include "calculator.h"


// TODO: add .layer_to_right
typedef struct InputField {
    GFont font;
    bool is_selected;
    GRect text_frame;
    GRect selection_frame;
    GetTxtCallback *get_text;
    IncDecCallback *inc_value;
    IncDecCallback *dec_value;
} InputField;