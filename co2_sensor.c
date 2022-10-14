/* Flipper App to read the values from a SCD4X Sensor  */

#include <gui/gui.h>
#include <input/input.h>
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <core/log.h>

#include <notification/notification_messages.h>

#include <string.h>
#include "scd4x.h"

#define DATA_BUFFER_SIZE 8

typedef enum {
    Initializing,
    NoSensor,
    PendingUpdate,
} SensorStatus;

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

static SensorStatus sensor_current_status = Initializing;

extern const NotificationSequence sequence_blink_red_100;
extern const NotificationSequence sequence_blink_blue_100;

// Temperature and Humidity data buffers, ready to print
char ts_data_buffer_temperature_c[DATA_BUFFER_SIZE];
char ts_data_buffer_humidity[DATA_BUFFER_SIZE];
char ts_data_buffer_co2[DATA_BUFFER_SIZE];

static void render_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "CO2 Sensor");

    canvas_set_font(canvas, FontSecondary);
    //canvas_draw_str(canvas, 2, 62, "Press back to exit.");

    switch(sensor_current_status) {
    case Initializing:
        canvas_draw_str(canvas, 2, 30, "Initializing..");
        break;
    case NoSensor:
        canvas_draw_str(canvas, 2, 30, "No sensor found!");
        break;
    case PendingUpdate: {
        canvas_draw_str(canvas, 6, 24, "Temperature");
        canvas_draw_str(canvas, 6, 38, "Humidity");
        canvas_draw_str(canvas, 6, 52, "CO2");

        //canvas_draw_str(canvas, 80, 24, "Humidity");

        // Draw vertical lines
        canvas_draw_line(canvas, 68, 16, 68, 50);
        canvas_draw_line(canvas, 69, 16, 69, 50);

        // Draw horizontal lines
        canvas_draw_line(canvas, 3, 27, 144, 27);
        canvas_draw_line(canvas, 3, 41, 144, 41);

        // Draw temperature and humidity values
        canvas_draw_str(canvas, 78, 24, ts_data_buffer_temperature_c);
        canvas_draw_str(canvas, 105, 24, "C");
        canvas_draw_str(canvas, 78, 38, ts_data_buffer_humidity);
        canvas_draw_str(canvas, 105, 38, "%");
        canvas_draw_str(canvas, 78, 52, ts_data_buffer_co2);
        canvas_draw_str(canvas, 105, 52, "ppm");

    } break;
    default:
        break;
    }
}

static void timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

int32_t co2_sensor_app(void* p) {
    UNUSED(p);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));

    // Register callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, NULL);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Register viewport
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Custom
    SCD4x_init(SCD4x_SENSOR_SCD40);
    enableDebugging();
    if(!SCD4x_begin(true, false, false)) {
        sensor_current_status = NoSensor;
        furi_log_print_format(FuriLogLevelDebug, "SCD4x", "Begin: Fail");

    } else {
        sensor_current_status = Initializing;
        furi_log_print_format(FuriLogLevelDebug, "SCD4x", "Begin: OK");
    }

    // Declare our variables
    PluginEvent tsEvent;
    bool sensorFound = false;
    float celsius, humidity = 0.0;
    uint16_t co2 = 0;

    // Create timer and register its callback
    FuriTimer* timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_ms_to_ticks(1000));

    // Used to notify the user by blinking red (error) or blue (fetch successful)
    NotificationApp* notifications = furi_record_open(RECORD_NOTIFICATION);

    while(1) {
        furi_check(furi_message_queue_get(event_queue, &tsEvent, FuriWaitForever) == FuriStatusOk);

        // Handle events
        if(tsEvent.type == EventTypeKey) {
            // We dont check for type here, we can check the type of keypress like: (event.input.type == InputTypeShort)
            // Exit on back key
            if(tsEvent.input.key == InputKeyBack) break;

        } else if(tsEvent.type == EventTypeTick) {
            // Update sensor data
            // Fetch data and set the sensor current status accordingly
            if(readMeasurement()) {
                furi_log_print_format(FuriLogLevelDebug, "SCD4x", "fresh data available");
                celsius = getTemperature();
                humidity = getHumidity();
                co2 = getCO2();
                sensor_current_status = PendingUpdate;

                notification_message(notifications, &sequence_blink_blue_100);

                snprintf(ts_data_buffer_temperature_c, DATA_BUFFER_SIZE, "%.2f", celsius);
                snprintf(ts_data_buffer_humidity, DATA_BUFFER_SIZE, "%.2f", humidity);
                snprintf(ts_data_buffer_co2, DATA_BUFFER_SIZE, "%d", co2);
            }
        }
        furi_delay_tick(furi_ms_to_ticks(100));
    }

    // Dobby is freee (free our variables, Flipper will crash if we don't do this!)
    furi_timer_free(timer);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);

    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);

    return 0;
}
