#include <pebble.h>

#include "calculator.h"
#include "tipcalc.h"

#define BOARDER 3
#define WINDOW_INSET 5
#define ROW_1_Y 17
#define ROW_2_Y 75
#define ROW_3_Y 147
#define BUTTON_HOLD_REPEAT_MS 100
#define NUM_INPUT_FIELDS 4
#define SUM_LINE_GCOLOR GColorFromHEX(0x979797)
#define OVERFLOW_MODE GTextOverflowModeWordWrap


static int current_input_idx = 0;  // use in click handlers to select callbacks
static InputField *input_fields[NUM_INPUT_FIELDS];

static Window *main_window;

static GRect main_bounds;

static Layer *main_layer;
static Layer *bill_cents_layer;
static Layer *bill_period_layer;
static Layer *bill_dollars_layer;
static Layer *bill_dollar_sign_layer;
static Layer *tip_amount_layer;
static Layer *tip_percent_sign_layer;
static Layer *tip_percent_layer;
static Layer *sum_line_layer;
static Layer *total_per_person_layer;
static Layer *num_splitting_layer;
static Layer *num_splitting_symbol_layer;

static GFont helvetica_18;
static GFont helvetica_22;
static GFont helvetica_24;
static GFont helvetica_26;

static GEdgeInsets helvetica_22_insets = {6 - BOARDER - 1, 1 - BOARDER, 0 - BOARDER - 1, 1 - BOARDER};  // t, r, b, l
static GEdgeInsets helvetica_24_insets = {7 - BOARDER - 1, 1 - BOARDER, 0 - BOARDER - 1, 1 - BOARDER};


static GPoint field_get_left_center_point(Field *field, int16_t left_padding) {
  int16_t x = field->right_center_point.x - field->max_width - left_padding;
  int16_t y = field->right_center_point.y;
  return GPoint(x, y);
}


static GRect field_get_text_frame(Field *field) {
  // Get coordinates of left corner of text frame.
  int16_t x = field->right_center_point.x - field->max_width;
  int16_t y = field->right_center_point.y - field->font_size/(int16_t)2;
  return GRect(x, y, field->max_width, field->font_size);
}


static void field_draw_text(Field *field, char *text, GContext *ctx) {
  GRect text_frame = field_get_text_frame(field);
  graphics_draw_text(ctx, text, field->font, text_frame, OVERFLOW_MODE, field->text_alignment, NULL);
}


// ******************************************** LayerUpdateProc callbacks *********************************************

static void input_field_update_proc(Layer *layer, GContext *ctx) {
  InputField *input_field = (InputField *)layer_get_data(layer);

  if(input_field->is_selected) {
    // Make the text white and the selection indicator filled black.
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_context_set_fill_color(ctx, GColorBlack);
    // Create the selection indicator's frame by insetting around the text frame.
    GRect text_frame = field_get_text_frame((Field *)input_field);
    GRect selection_frame = grect_inset(text_frame, input_field->selection_insets);
    // Draw the selection indicator's frame.
    graphics_fill_rect(ctx, selection_frame, 4, GCornersAll);
  } else {
    GRect text_frame = field_get_text_frame((Field *)input_field);
    GRect selection_frame = grect_inset(text_frame, input_field->selection_insets);
    graphics_draw_round_rect(ctx, selection_frame, 4);
    graphics_context_set_text_color(ctx, GColorBlack);
  }

  // Draw the text using the text color set above.
  field_draw_text((Field *)input_field, input_field->get_text(), ctx);
}


static void output_field_layer_update_proc(Layer *layer, GContext *ctx) {
  OutputField *output_field = (OutputField *)layer_get_data(layer);

  graphics_context_set_text_color(ctx, GColorBlack);
  field_draw_text((Field *)output_field, output_field->get_text(), ctx);
}


static void decoration_field_layer_update_proc(Layer *layer, GContext *ctx) {
  DecorationField *decoration_field = (DecorationField *)layer_get_data(layer);

  graphics_context_set_text_color(ctx, GColorBlack);
  field_draw_text((Field *)decoration_field, decoration_field->text, ctx);
}


static void line_layer_update_proc(Layer *layer, GContext *ctx) {
  Line *line = (Line *)layer_get_data(layer);

  graphics_context_set_stroke_color(ctx, line->stroke_color);
  graphics_context_set_stroke_width(ctx, line->stroke_width);
  graphics_draw_line(ctx, line->start_point, line->end_point);
}


// ************************************************ Layer initializers ************************************************

static Layer *input_layer_create(InputField input_field) {
  Layer *layer = layer_create_with_data(main_bounds, sizeof(InputField));

  InputField *data = (InputField *)layer_get_data(layer);
  *data = input_field;

  layer_set_update_proc(layer, input_field_update_proc);
  layer_add_child(main_layer, layer);

  return layer;
}


static Layer *output_layer_create(OutputField output_field) {
  Layer *layer = layer_create_with_data(main_bounds, sizeof(OutputField));

  OutputField *data = (OutputField *)layer_get_data(layer);
  *data = output_field;

  layer_set_update_proc(layer, output_field_layer_update_proc);
  layer_add_child(main_layer, layer);

  return layer;
}


static Layer *decoration_layer_create(DecorationField decoration_field) {
  Layer *layer = layer_create_with_data(main_bounds, sizeof(DecorationField));

  DecorationField *data = (DecorationField *)layer_get_data(layer);
  *data = decoration_field;

  layer_set_update_proc(layer, decoration_field_layer_update_proc);
  layer_add_child(main_layer, layer);

  return layer;
}


static Layer *line_layer_create(Line line) {
  Layer *layer = layer_create_with_data(main_bounds, sizeof(Line));

  Line *data = (Line *)layer_get_data(layer);
  *data = line;

  layer_set_update_proc(layer, line_layer_update_proc);
  layer_add_child(main_layer, layer);

  return layer;
}


// *******************************************  click & tap event handlers  *******************************************

static int get_click_accelerated_delta(int num_clicks) {
  int ms = num_clicks * BUTTON_HOLD_REPEAT_MS;  // milliseconds button has been held down for
  int delta;
  if(ms <= 5000) {  // 0-4 seconds: increment by one on every interval
    delta = 1;
  } else if(5000 < ms && ms <= 10000 && num_clicks % 3 == 0) {  // 4-10 seconds: increment by 10 on every 3rd interval
    delta = 10;
  } else if(10000 < ms  && num_clicks % 5 == 0) {  // 10+ seconds: increment by 100 on every 5th interval
    delta = 100;
  } else {
    delta = 0;  // this is so no increment happens on skipped intervals
  }
  return delta;
}


static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  InputField *input_field = input_fields[current_input_idx];

  int delta;
  if(click_recognizer_is_repeating(recognizer)) {
    delta = get_click_accelerated_delta(click_number_of_clicks_counted(recognizer));
  } else {
    delta = 1;
  }
  input_field->manipulate(delta);
  layer_mark_dirty(main_layer);
}


static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  InputField *input_field = input_fields[current_input_idx];

  int delta;
  if(click_recognizer_is_repeating(recognizer)) {
    delta = -1 * get_click_accelerated_delta(click_number_of_clicks_counted(recognizer));
  } else {
    delta = -1;
  }
  input_field->manipulate(delta);
  layer_mark_dirty(main_layer);
}



static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(current_input_idx < NUM_INPUT_FIELDS - 1) {
    input_fields[current_input_idx]->is_selected = false;
    current_input_idx++;  // advance to next input
    input_fields[current_input_idx]->is_selected = true;
    calc_update_totals();
    layer_mark_dirty(main_layer);
  }
}


static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(current_input_idx > 0) {
    input_fields[current_input_idx]->is_selected = false;
    current_input_idx--;  // advance to previous input
    input_fields[current_input_idx]->is_selected = true;
  } else {
    window_stack_pop(true);
  }
  calc_update_totals();
  layer_mark_dirty(main_layer);
}


static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, BUTTON_HOLD_REPEAT_MS, up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, BUTTON_HOLD_REPEAT_MS, down_click_handler);
}


static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  calc_reset_to_defaults();
  layer_mark_dirty(main_layer);
}


// *******************************************  launch, setup, & teardown  ********************************************

static void main_window_load(Window* window) {
  main_layer = window_get_root_layer(main_window);
  main_bounds = layer_get_bounds(main_layer);

  helvetica_18 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_HELVETICA_ROUNDED_18));
  helvetica_22 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_HELVETICA_ROUNDED_22));
  helvetica_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_HELVETICA_ROUNDED_24));
  helvetica_26 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_HELVETICA_ROUNDED_26));

  // Bill cents
  bill_cents_layer = input_layer_create((InputField){
      .right_center_point = GPoint(main_bounds.size.w - (int16_t)WINDOW_INSET, ROW_1_Y),
      .max_width = 26,
      .font = helvetica_24,
      .font_size = 24,
      .text_alignment = GTextAlignmentCenter,
      .get_text = calc_get_bill_cents_txt,
      .selection_insets = helvetica_24_insets,
      .manipulate = calc_manip_bill_cents,
      .is_selected = false
  });

  // Bill (.)
  bill_period_layer = decoration_layer_create((DecorationField){
      .right_center_point = field_get_left_center_point((Field *)layer_get_data(bill_cents_layer), BOARDER),
      .max_width = 5,
      .font = helvetica_24,
      .font_size = 24,
      .text_alignment = GTextAlignmentCenter,
      .text = "."
  });

  // Bill dollars
  bill_dollars_layer = input_layer_create((InputField){
      .right_center_point = field_get_left_center_point((Field *)layer_get_data(bill_period_layer), BOARDER),
      .max_width = 39,
      .font = helvetica_24,
      .font_size = 24,
      .text_alignment = GTextAlignmentRight,
      .get_text = calc_get_bill_dollars_txt,
      .selection_insets = helvetica_24_insets,
      .manipulate = calc_manip_bill_dollars,
      .is_selected = true
  });

  // Bill ($)
  bill_dollar_sign_layer = decoration_layer_create((DecorationField){
      .right_center_point = field_get_left_center_point((Field *)layer_get_data(bill_dollars_layer), BOARDER),
      .max_width = 9,
      .font = helvetica_18,
      .font_size = 18,
      .text_alignment = GTextAlignmentRight,
      .text = "$"
  });

  tip_amount_layer = output_layer_create((OutputField){
      .right_center_point = GPoint(main_bounds.size.w - (int16_t)WINDOW_INSET, ROW_2_Y),
      .max_width = 89,
      .font = helvetica_26,
      .font_size = 26,
      .text_alignment = GTextAlignmentRight,
      .get_text = calc_get_tip_txt
  });

  // Tip percent (%)
  tip_percent_sign_layer = decoration_layer_create((DecorationField){
      .right_center_point = field_get_left_center_point((Field *)layer_get_data(tip_amount_layer), 4),
      .max_width = 16,
      .font = helvetica_18,
      .font_size = 18,
      .text_alignment = GTextAlignmentLeft,
      .text = "%"
  });

  // Tip percent
  tip_percent_layer = input_layer_create((InputField){
      .right_center_point = field_get_left_center_point((Field *)layer_get_data(tip_percent_sign_layer), BOARDER - 1),
      .max_width = 23,
      .font = helvetica_22,
      .font_size = 22,
      .text_alignment = GTextAlignmentRight,
      .get_text = calc_get_tip_percent_txt,
      .selection_insets = helvetica_22_insets,
      .manipulate = calc_manip_tip_percent,
      .is_selected = false
  });

  // Summation line
  sum_line_layer = line_layer_create((Line){
      .start_point = GPoint(55, ROW_2_Y + 17),
      .end_point = GPoint(main_bounds.size.w - (int16_t)WINDOW_INSET, ROW_2_Y + 17),
      .stroke_width = 3,
      .stroke_color = PBL_IF_COLOR_ELSE(SUM_LINE_GCOLOR, GColorBlack)
  });

  // Total/person
  total_per_person_layer = output_layer_create((OutputField){
      .right_center_point = GPoint(main_bounds.size.w - (int16_t)WINDOW_INSET, ROW_3_Y),
      .max_width = 98,
      .font = helvetica_26,
      .font_size = 26,
      .text_alignment = GTextAlignmentRight,
      .get_text = calc_get_total_per_person_txt
  });

  // Number of people splitting
  num_splitting_layer = input_layer_create((InputField){
      .right_center_point = field_get_left_center_point((Field *)layer_get_data(total_per_person_layer),
                                                        BOARDER + 7),
      .max_width = 13,
      .font = helvetica_22,
      .font_size = 22,
      .text_alignment = GTextAlignmentCenter,
      .get_text = calc_get_num_splitting_txt,
      .selection_insets = helvetica_22_insets,
      .manipulate = calc_manip_num_splitting,
      .is_selected = false
  });

  // Number of people splitting (÷)
  num_splitting_symbol_layer = decoration_layer_create((DecorationField){
      .right_center_point = field_get_left_center_point((Field *)layer_get_data(num_splitting_layer), BOARDER),
      .max_width = 14,
      .font = helvetica_22,
      .font_size = 22,
      .text_alignment = GTextAlignmentRight,
      .text = "÷"
  });

  input_fields[0] = (InputField *)layer_get_data(bill_dollars_layer);
  input_fields[1] = (InputField *)layer_get_data(bill_cents_layer);
  input_fields[2] = (InputField *)layer_get_data(tip_percent_layer);
  input_fields[3] = (InputField *)layer_get_data(num_splitting_layer);

  layer_mark_dirty(main_layer);
}


static void main_window_unload(Window *window) {
  layer_destroy(bill_cents_layer);
  layer_destroy(bill_period_layer);
  layer_destroy(bill_dollars_layer);
  layer_destroy(bill_dollar_sign_layer);
  layer_destroy(tip_amount_layer);
  layer_destroy(tip_percent_sign_layer);
  layer_destroy(tip_percent_layer);
  layer_destroy(sum_line_layer);
  layer_destroy(total_per_person_layer);
  layer_destroy(num_splitting_layer);
  layer_destroy(num_splitting_symbol_layer);

  fonts_unload_custom_font(helvetica_18);
  fonts_unload_custom_font(helvetica_22);
  fonts_unload_custom_font(helvetica_24);
  fonts_unload_custom_font(helvetica_26);
}

static void init(void) {
  calc_persist_read();

  accel_tap_service_subscribe((AccelTapHandler)accel_tap_handler);

  main_window = window_create();
  window_set_click_config_provider(main_window, click_config_provider);
  window_set_window_handlers(main_window, (WindowHandlers){
      .load = main_window_load,
      .unload = main_window_unload
  });
  window_stack_push(main_window, true);
}


static void deinit(void) {
  window_destroy(main_window);
  accel_tap_service_unsubscribe();
  calc_persist_store();
}


int main(void) {
  init();
  app_event_loop();
  deinit();
}
