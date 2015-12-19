#pragma once

#include <pebble.h>
#include "calculator.h"


// TODO: add .layer_to_right
typedef struct {
    GFont font;
    bool is_selected;
    GRect text_frame;
    GEdgeInsets selection_insets;
    GetTxtCallback *get_text;
    IncDecCallback *inc_value;
    IncDecCallback *dec_value;
} InputField;

typedef struct {
    GFont font;
    GRect text_frame;
    GetTxtCallback *get_text;
} OutputField;