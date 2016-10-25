#include <pebble.h>
#include <pebble-fctx/fctx.h>
#include <pebble-fctx/ffont.h>

// main window
static Window *s_window;

// layers
static TextLayer *s_text_layer;
static Layer *s_background_layer;

// imported font
static FFont *s_test_font;

// font size
static int s_font_size;

// static char buffer to display text
static char s_font_size_text[5];

// increase font size and update the text layer
static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  s_font_size++;

  // mark dirty to force refresh with update_proc
  layer_mark_dirty(s_background_layer);
}

// decrease font size and update the text layer
static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  s_font_size--;

  // mark dirty to force refresh with update_proc
  layer_mark_dirty(s_background_layer);
}

// set up click provider to bind functions to the buttons
static void prv_click_config_provider(void *context) {
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 100, prv_up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, prv_down_click_handler);
}

static void background_update_proc(Layer *layer, GContext *ctx) {
  // print font size to buffer and update the show font size number
  snprintf(s_font_size_text,sizeof(s_font_size_text),"%d",s_font_size);
  layer_mark_dirty(text_layer_get_layer(s_text_layer));

  // get layer bounds
  GRect bounds = layer_get_bounds(layer);
  
  FContext fctx;
  fctx_init_context(&fctx, ctx);
  fctx_begin_fill(&fctx);
  
  // set font size and font to the context
  fctx_set_text_em_height(&fctx, s_test_font, s_font_size);

  // set fill color to context
  fctx_set_fill_color(&fctx, GColorBlack);
  fctx_set_pivot(&fctx, FPointZero);
  
  // set offset to text layer
  fctx_set_offset(&fctx, FPointI(bounds.size.w/2,bounds.size.h/2));

  // draw the text into the context
  fctx_draw_string(&fctx, "AB1", s_test_font, GTextAlignmentCenter, FTextAnchorCapMiddle);
  
  // end drawing procedure and render font to gcontext
  fctx_end_fill(&fctx);
  fctx_deinit_context(&fctx);
}

// main window load
static void prv_window_load(Window *window) {
  // get layer of root window and its bounds
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // create background layer
  s_background_layer = layer_create(bounds);

  // set update procedure for background layer
  layer_set_update_proc(s_background_layer, background_update_proc);
  
  // add layer to main window
  layer_add_child(window_layer, s_background_layer);

  // default font size
  s_font_size = 50;

  // get font from resource
  s_test_font = ffont_create_from_resource(RESOURCE_ID_TEST_FFONT);
  
  // create text layer
  s_text_layer = text_layer_create(GRect(0, 0, bounds.size.w, 20));
  
  // set actual font size number as text to text layer
  text_layer_set_text(s_text_layer, s_font_size_text);

  // align text in center
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);

  // add the text layer to the main window
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

// destroy layers to unload the window
static void prv_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  layer_destroy(s_background_layer);
}

// initialize the app
static void prv_init(void) {
  // create main window
  s_window = window_create();

  // set up click provider for button actions
  window_set_click_config_provider(s_window, prv_click_config_provider);

  // assign window load and unload function
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });

  // push window with settings to stack
  const bool animated = true;
  window_stack_push(s_window, animated);
}

// deinitialize the app
static void prv_deinit(void) {
  window_destroy(s_window);
}

// main function which represents the apps life cycle
int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
