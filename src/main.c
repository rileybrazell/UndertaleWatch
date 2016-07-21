#include <pebble.h>

#define KEY_TEMPERATURE 0

// Window
static Window *s_main_window;

// TextLayer
static TextLayer *s_time_layer, *s_weather_layer, *s_date_layer;

// Font
static GFont s_time_font, s_weather_font, s_date_font;

// Bitmap
static GBitmap *s_sans_head_bitmap, *s_sans_wink_bitmap;
static BitmapLayer *s_sans_head_layer;

// Get current time, write to timelayer
static void update_time() {
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	
	static char s_buffer[8];
	strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
	
	text_layer_set_text(s_time_layer, s_buffer);
	
	// Copy date into buffer from tm structure
	static char date_buffer[8];
	strftime(date_buffer, sizeof(date_buffer), "%b %d", tick_time);

	// Show the date
	text_layer_set_text(s_date_layer, date_buffer);
}

static void main_window_load(Window *window) {
	// Set window layer and get bounds of window
	Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
	
	// Create bitmap images from resources
	s_sans_head_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SANS_HEAD);
	s_sans_wink_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SANS_WINK);
	
	// Check if watch is round or rectangular, affects font size and layer position/size
	#if defined(PBL_RECT)
		// Create background layer
		s_sans_head_layer = bitmap_layer_create(
			GRect(0, -7, bounds.size.w, bounds.size.h));
	
  	// Set font of time layer
		s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_42));

		// Set time layer to new text layer
		s_time_layer = text_layer_create(
			GRect(0, 120, bounds.size.w, 50));
	
		// Set the font of the weather layer
		s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_18));	
	
		// Set weather layer to new layer
		s_weather_layer = text_layer_create(
			GRect(5, 4, 60, 20));
	
		// Set the font of the weather layer
		s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_12));
		
		// Create date layer
		s_date_layer = text_layer_create(
			GRect(113, 6, 30, 30));

	#elif defined(PBL_ROUND)
		// Create background layer
		s_sans_head_layer = bitmap_layer_create(
			GRect(0, 25, bounds.size.w, 105));
	
  	// Set font of time layer
		s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_32));

		// Set time layer to new text layer
		s_time_layer = text_layer_create(
			GRect(0, 125, bounds.size.w, 50));
	
		// Set the font of the weather layer
		s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_12));	

		// Set weather layer to new layer
		s_weather_layer = text_layer_create(
			GRect(5, 77, 30, 20));
	
		// Set the font of the weather layer
		s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_12));
	
		// Create date layer
		s_date_layer = text_layer_create(
			GRect(147, 77, 30, 30));
	#endif
	
	// Set the bitmap onto the background layer and add to window
	bitmap_layer_set_bitmap(s_sans_head_layer, s_sans_head_bitmap);
	bitmap_layer_set_compositing_mode(s_sans_head_layer, GCompOpSet);
	layer_add_child(window_layer, bitmap_layer_get_layer(s_sans_head_layer));
	
	// Style the time layer
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorWhite);
	text_layer_set_font(s_time_layer, s_time_font);
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	
	// Add time layer to window
	layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
	
	// Style the weather layer
	text_layer_set_background_color(s_weather_layer, GColorClear);
	text_layer_set_text_color(s_weather_layer, GColorWhite);
	text_layer_set_text_alignment(s_weather_layer, GTextAlignmentLeft);
	text_layer_set_font(s_weather_layer, s_weather_font);
	text_layer_set_text(s_weather_layer, "...");
	
	// Add weather layer to window
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
	
	// Style the date layer
	text_layer_set_background_color(s_date_layer, GColorClear);
	text_layer_set_text_color(s_date_layer, GColorWhite);
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentRight);
	text_layer_set_font(s_date_layer, s_date_font);
	
	// Add date layer to window
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
}

// Destroy layers, fonts, bitmaps on unload
static void main_window_unload(Window *window) {
	text_layer_destroy(s_time_layer);
	text_layer_destroy(s_weather_layer);
	text_layer_destroy(s_date_layer);
	
	fonts_unload_custom_font(s_time_font);
	fonts_unload_custom_font(s_weather_font);
	fonts_unload_custom_font(s_date_font);
	
	gbitmap_destroy(s_sans_head_bitmap);
	gbitmap_destroy(s_sans_wink_bitmap);
	bitmap_layer_destroy(s_sans_head_layer);
}

// On minute tick call update_time function to update displayed time
// and update weather every 30 mins
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
	
	// Get weather update every 30 minutes
	if(tick_time->tm_min % 30 == 0) {
  	text_layer_set_text(s_weather_layer, "...");
		
		// Begin dictionary
  	DictionaryIterator *iter;
  	app_message_outbox_begin(&iter);

	  // Add a key-value pair
  	dict_write_uint8(iter, 0, 0);

  	// Send the message!
	  app_message_outbox_send();
	}
}

// Sets sans back to default face
void unwink_callback(){
	bitmap_layer_set_bitmap(s_sans_head_layer, s_sans_head_bitmap);
}
			
// Called by shake event - winks face, waits a bit, then calls unwink function
void wink_callback(){
	bitmap_layer_set_bitmap(s_sans_head_layer, s_sans_wink_bitmap);
	app_timer_register(1000, (AppTimerCallback) unwink_callback, NULL);
}
			
// tap handler to change bitmap and get weather update on tap event
static void tap_handler(AccelAxisType axis, int32_t direction) {
	if (direction > 0) {
		app_timer_register(1000, (AppTimerCallback) wink_callback, NULL);

		text_layer_set_text(s_weather_layer, "...");

		// Send message to AppMessage to get temperature info
		// Begin dictionary
  	DictionaryIterator *iter;
  	app_message_outbox_begin(&iter);

 	  // Add a key-value pair
  	dict_write_uint8(iter, 0, 0);

  	// Send the message!
  	app_message_outbox_send();
		}
}

// When a valid message is received with weather info
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
	// Store incoming information
	static char temperature_buffer[8];
	static char weather_layer_buffer[32];
	
	// Read tuples for data
	Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);

	// If correct data is available, use it
  snprintf(temperature_buffer, sizeof(temperature_buffer), "%dF", (int)temp_tuple->value->int32);
			
	// Assemble full string and display
	snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s", temperature_buffer);
	text_layer_set_text(s_weather_layer, weather_layer_buffer);
}

// Other console messages for logging
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
	// Create main window and assign to pointer
	s_main_window = window_create();
	
	// Set handlers to manage the elements inside the window
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});
	
	window_set_background_color(s_main_window, GColorBlack);
	
	// Show the window on the watch, with animated=true
	window_stack_push(s_main_window, true);
	
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	
	accel_tap_service_subscribe(tap_handler);
	
	// Register callbacks
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);
	
	// Open AppMessage
	const int inbox_size = 128;
	const int outbox_size = 128;
	app_message_open(inbox_size, outbox_size);
}

static void deinit() {
	// Unsubscribe from tick_handler
	tick_timer_service_unsubscribe();
	
	// Unsubscribe from accel_handler
	accel_tap_service_unsubscribe();
	
	// Destroy Window
	window_destroy(s_main_window);
}

// Main loop of program
int main(void) {
  init();
  app_event_loop();
  deinit();
}