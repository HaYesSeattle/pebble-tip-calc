#include <pebble.h>
#include "calculator.h"
#include "tipcalc.h"

#define BOARDER 3
#define ROW_1_Y 4
#define ROW_2_Y 71
#define ROW_3_Y 139
#define BUTTON_HOLD_REPEAT_MS 100
#define NUM_INPUT_FIELDS 4


static int current_input_idx = 0;  // use in click handlers to select callbacks
static InputField *input_fields[NUM_INPUT_FIELDS];

static Window *main_window;

static Layer *main_layer;
static Layer *selection_indicator;
static Layer *bill_dollars_layer;
static Layer *bill_cents_layer;
static Layer *tip_percent_layer;
static Layer *num_splitting_layer;

static GFont helvetica_18;
static GFont helvetica_22;
static GFont helvetica_24;
static GFont helvetica_26;

static GEdgeInsets helvetica_22_insets = {6 - BOARDER, 1 - BOARDER, 1 - BOARDER, 1 - BOARDER};  // t, r, l, b
static GEdgeInsets helvetica_24_insets = {7 - BOARDER, 1 - BOARDER, 1 - BOARDER, 1 - BOARDER};


static void input_field_update_proc(Layer *layer, GContext *ctx) {
  InputField *input_field = (InputField *)layer_get_data(layer);

  // TODO: Implement dynamic text and selection frames sizing based size of current string? Alignment?
  if (input_field->is_selected) {
    // Set the text color to white.
    graphics_context_set_text_color(ctx, GColorWhite);
    // Draw the selection box.
    graphics_context_set_fill_color(ctx, GColorBlack);
    GRect selection_frame = grect_inset(input_field->text_frame, input_field->selection_insets);
    graphics_fill_rect(ctx, selection_frame, 0, GCornerNone);
  } else {
    // Set the text color to white.
    graphics_context_set_text_color(ctx, GColorBlack);
  }

  // Draw the text.
  char *text = input_field->get_text();
  graphics_draw_text(ctx, text, input_field->font, input_field->text_frame,
                     GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
}


static Layer *input_layer_create(void) {
  Layer *layer = layer_create_with_data(GRect(0, 0, 144, 168), sizeof(InputField));
  layer_set_update_proc(layer, input_field_update_proc);
  layer_add_child(main_layer, layer);
  layer_set_clips(layer, false);  // TODO: check if needed

  return layer;
}


static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  input_fields[current_input_idx]->inc_value();
  layer_mark_dirty(main_layer);
}


static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  input_fields[current_input_idx]->dec_value();
  layer_mark_dirty(main_layer);
}


// TODO: Implement select button long-click functionality
// TODO: Flash selection frame when pressed on last input
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (current_input_idx < NUM_INPUT_FIELDS - 1) {
    input_fields[current_input_idx]->is_selected = false;
    current_input_idx++;  // advance to next input
    input_fields[current_input_idx]->is_selected = true;
  }
  layer_mark_dirty(main_layer);
}


static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (current_input_idx > 0) {
    input_fields[current_input_idx]->is_selected = false;
    current_input_idx--;  // advance to previous input
    input_fields[current_input_idx]->is_selected = true;
  } else {
    window_stack_pop_all(true);  // TODO: Find out if this is the correct way to exit app
  }
  layer_mark_dirty(main_layer);
}


static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, BUTTON_HOLD_REPEAT_MS, up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, BUTTON_HOLD_REPEAT_MS, down_click_handler);
}


static void main_window_load(Window* window) {
  main_layer = window_get_root_layer(main_window);
  helvetica_18 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_HELVETICA_ROUNDED_18));
  helvetica_22 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_HELVETICA_ROUNDED_22));
  helvetica_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_HELVETICA_ROUNDED_24));
  helvetica_26 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_HELVETICA_ROUNDED_26));

  InputField *input_field;
  // Bill dollars
  bill_dollars_layer = input_layer_create();
  input_field = layer_get_data(bill_dollars_layer);
  *input_field = (InputField) {
      .font = helvetica_24,
      .text_frame = GRect(46, ROW_1_Y, 48, 25),
      .selection_insets = helvetica_24_insets,
      .get_text = calc_get_bill_dollars_txt,
      .inc_value = calc_inc_bill_dollars,
      .dec_value = calc_dec_bill_dollars,
      .is_selected = true
  };
  input_fields[0] = input_field;
  // Bill cents
  bill_cents_layer = input_layer_create();
  input_field = layer_get_data(bill_cents_layer);
  *input_field = (InputField) {
      .font = helvetica_24,
      .text_frame = GRect(108, ROW_1_Y, 32, 25),
      .selection_insets = helvetica_24_insets,
      .get_text = calc_get_bill_cents_txt,
      .inc_value = calc_inc_bill_cents,
      .dec_value = calc_dec_bill_cents,
      .is_selected = false
  };
  input_fields[1] = input_field;
  // Tip percent
  tip_percent_layer = input_layer_create();
  input_field = layer_get_data(tip_percent_layer);
  *input_field = (InputField) {
      .font = helvetica_22,
      .text_frame = GRect(4, ROW_2_Y, 30, 23),
      .selection_insets = helvetica_22_insets,
      .get_text = calc_get_tip_percent_txt,
      .inc_value = calc_inc_tip_percent,
      .dec_value = calc_dec_tip_percent,
      .is_selected = false
  };
  input_fields[2] = input_field;
  // Num splitting
  num_splitting_layer = input_layer_create();
  input_field = layer_get_data(num_splitting_layer);
  *input_field = (InputField) {
      .font = helvetica_22,
      .text_frame = GRect(4, ROW_3_Y, 16, 23),
      .selection_insets = helvetica_22_insets,
      .get_text = calc_get_num_splitting_txt,
      .inc_value = calc_inc_num_splitting,
      .dec_value = calc_dec_num_splitting,
      .is_selected = false
  };
  input_fields[3] = input_field;


  layer_mark_dirty(main_layer);
}


static void main_window_unload(Window *window) {
  layer_destroy(selection_indicator);
  layer_destroy(bill_dollars_layer);
  layer_destroy(bill_cents_layer);
  layer_destroy(tip_percent_layer);
  layer_destroy(num_splitting_layer);

  fonts_unload_custom_font(helvetica_18);
  fonts_unload_custom_font(helvetica_22);
  fonts_unload_custom_font(helvetica_24);
  fonts_unload_custom_font(helvetica_26);

}

static void init(void) {
  calc_persist_read();
  main_window = window_create();
  window_set_click_config_provider(main_window, click_config_provider);
  window_set_window_handlers(main_window, (WindowHandlers) {
      .load = main_window_load,
      .unload = main_window_unload
  });
  window_stack_push(main_window, true);
}


static void deinit(void) {
  window_destroy(main_window);
//  calc_persist_store();  // TODO: uncomment before release
}


int main(void) {
  init();
  app_event_loop();
  deinit();
}
