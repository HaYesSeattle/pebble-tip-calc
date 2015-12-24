#pragma once

#include <pebble.h>
#include "calculator.h"


typedef struct {
    GPoint right_center_point;
    int16_t max_width;
    GFont font;
    int16_t font_size;
    GTextAlignment text_alignment;
} Field;

typedef struct{
    GPoint right_center_point;
    int16_t max_width;
    GFont font;
    int16_t font_size;
    GTextAlignment text_alignment;
    char *text;
} DecorationField;

typedef struct {
    GPoint right_center_point;
    int16_t max_width;
    GFont font;
    int16_t font_size;
    GTextAlignment text_alignment;
    GetTxtCallback *get_text;
} OutputField;

typedef struct {
    GPoint right_center_point;
    int16_t max_width;
    GFont font;
    int16_t font_size;
    GTextAlignment text_alignment;
    GetTxtCallback *get_text;
    GEdgeInsets selection_insets;
    CalcManipCallback *manipulate;
    bool is_selected;
} InputField;

typedef struct {
    GPoint start_point;
    GPoint end_point;
    GColor stroke_color;
    uint8_t stroke_width;
} Line;