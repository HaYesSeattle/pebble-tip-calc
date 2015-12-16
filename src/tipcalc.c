#include <pebble.h>
#include "calculator.h"
#include "tipcalc.h"

#define NUM_INPUT_FIELDS 4
#define FONT_TOP_PADDING -10


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
  graphics_context_set_text_color(ctx, GColorBlack);  // TODO: set white when selected
  InputField *input_field = (InputField *)layer_get_data(layer);
  // Create the needed frame needed to fit text.
  char *text = input_field->get_text();
  GSize size = graphics_text_layout_get_content_size(
      text, input_field->font, GRect(0, 0, 144, 168), GTextOverflowModeWordWrap, GTextAlignmentRight
  );
  GRect frame = layer_get_frame(layer);
  frame.size = size;
  // TODO: adjust frame to arrange next to .layer_to_right
  // Draw text.
  graphics_draw_text(ctx, text, input_field->font, frame, GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
  layer_set_clips(layer, false);  // TODO: check if needed
  // Set selection frame.
  frame.size.h += FONT_TOP_PADDING;
  frame.origin.y += FONT_TOP_PADDING;
  input_field->selection_frame = frame;
}


static Layer *input_layer_create(GetTxtCallback *get_text, GFont font, bool is_selected) {
  Layer *layer = layer_create_with_data(GRect(0, 0, 144, 168), sizeof(InputField));
  InputField *input_field = layer_get_data(layer);
  // Set defaults.
  *input_field = (InputField) {
      .font = font,
      .is_selected = is_selected,
      .get_text = get_text
  };
  layer_set_update_proc(layer, input_field_update_proc);
  layer_add_child(main_layer, layer);
  layer_mark_dirty(layer);
  return layer;
}


static void main_window_load(Window* window) {
  helvetica_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_HELVETICA_ROUNDED_24));

  main_layer = window_get_root_layer(main_window);

  // Bill dollars
  bill_dollars_layer = input_layer_create(calc_get_bill_dollars_txt, helvetica_24, true);

  // Create InputLayerData.
  // Define update procedures
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
