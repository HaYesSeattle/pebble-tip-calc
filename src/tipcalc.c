#include <pebble.h>
#include "calculator.h"
#include "tipcalc.h"

#define NUM_INPUT_FIELDS 4
#define BUTTON_HOLD_REPEAT_MS 100


static int *current_input_idx;  // use in click handlers to select callbacks
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


// Define ClickCallbackSets and ClickHandlers
// (mark window_layer dirty after each click)

// Define click_config_provider


static void input_field_update_proc(Layer *layer, GContext *ctx) {
  InputField *input_field = (InputField *)layer_get_data(layer);

  if (input_field->is_selected) {
    // Set the text color to white.
    graphics_context_set_text_color(ctx, GColorWhite);
    // Draw the selection box.
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, input_field->selection_frame, 0, GCornerNone);
  } else {
    // Set the text color to white.
    graphics_context_set_text_color(ctx, GColorBlack);
  }

  // Draw the text.
  char *text = input_field->get_text();
  graphics_draw_text(ctx, text, input_field->font, input_field->text_frame,
                     GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
}


static Layer *input_layer_create(GetTxtCallback *get_text, GFont font, IncDecCallback *inc_value,
                                 IncDecCallback *dec_value, GRect text_frame, GRect selection_frame,
                                 bool is_selected) {
  Layer *layer = layer_create_with_data(GRect(0, 0, 144, 168), sizeof(InputField));
  InputField *input_field = layer_get_data(layer);

  // Set defaults.
  *input_field = (InputField) {
      .get_text = get_text,
      .font = font,
      .inc_value = inc_value,
      .dec_value = dec_value,
      .text_frame = text_frame,
      .selection_frame = selection_frame,
      .is_selected = is_selected
  };
  layer_set_update_proc(layer, input_field_update_proc);
  layer_add_child(main_layer, layer);
  layer_set_clips(layer, false);  // TODO: check if needed
  layer_mark_dirty(layer);

  return layer;
}


static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  InputField *input_field = (InputField *)layer_get_data(bill_dollars_layer);
  input_field->inc_value();
  layer_mark_dirty(bill_dollars_layer);
}


static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  InputField *input_field = (InputField *)layer_get_data(bill_dollars_layer);
  input_field->dec_value();
  layer_mark_dirty(bill_dollars_layer);
}


//static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
//  text_layer_set_text(text_layer, "Select");
//}


static void click_config_provider(void *context) {
//  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, BUTTON_HOLD_REPEAT_MS, up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, BUTTON_HOLD_REPEAT_MS, down_click_handler);
}


static void main_window_load(Window* window) {
  helvetica_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_HELVETICA_ROUNDED_24));

  main_layer = window_get_root_layer(main_window);

  // Bill dollars
  bill_dollars_layer = input_layer_create(calc_get_bill_dollars_txt, helvetica_24, calc_inc_bill_dollars,
                                          calc_dec_bill_dollars, GRect(61, 12, 48, 25), GRect(58, 15, 50, 24), true);
}


static void main_window_unload(Window *window) {
  fonts_unload_custom_font(helvetica_18);
  fonts_unload_custom_font(helvetica_22);
  fonts_unload_custom_font(helvetica_24);
  fonts_unload_custom_font(helvetica_26);

  layer_destroy(selection_indicator);
  layer_destroy(bill_dollars_layer);
  layer_destroy(bill_cents_layer);
  layer_destroy(tip_percent_layer);
  layer_destroy(num_splitting_layer);
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
}


int main(void) {
  init();
  app_event_loop();
  deinit();
}
