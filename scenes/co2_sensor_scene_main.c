#include "../co2_sensor.h"
#include "../scd4x.h"

static void timer_callback(void* ctx) {
    CO2App* app = ctx;
    furi_assert(app);
    if(readMeasurement()) {
        furi_log_print_format(FuriLogLevelDebug, "SCD4x", "fresh data available");
        app->status = PendingUpdate;
        notification_message(app->notifications, &sequence_blink_blue_100);
        snprintf(
            app->main_ctx->display_data.temperature,
            DATA_BUFFER_SIZE,
            "%.2f",
            (double)getTemperature());
        snprintf(
            app->main_ctx->display_data.humidity, DATA_BUFFER_SIZE, "%.2f", (double)getHumidity());
        snprintf(app->main_ctx->display_data.co2, DATA_BUFFER_SIZE, "%d", getCO2());
    }
}

static void input_callback(InputEvent* input_event, void* ctx) {
    CO2App* app = ctx;
    furi_assert(app);

    if(input_event->key == InputKeyBack) {
        scene_manager_stop(app->scene_manager);
    }
}

static void render_callback(Canvas* canvas, void* ctx) {
    CO2App* app = ctx;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "CO2 Sensor");

    canvas_set_font(canvas, FontSecondary);

    switch(app->status) {
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

        // Draw vertical lines
        canvas_draw_line(canvas, 66, 16, 66, 55);
        canvas_draw_line(canvas, 67, 16, 67, 55);

        // Draw horizontal lines
        canvas_draw_line(canvas, 3, 27, 144, 27);
        canvas_draw_line(canvas, 3, 41, 144, 41);

        // Draw temperature and humidity values
        canvas_draw_str(canvas, 72, 24, app->main_ctx->display_data.temperature);
        canvas_draw_str(canvas, 102, 24, "C");
        canvas_draw_str(canvas, 72, 38, app->main_ctx->display_data.humidity);
        canvas_draw_str(canvas, 102, 38, "%");
        canvas_draw_str(canvas, 72, 52, app->main_ctx->display_data.co2);
        canvas_draw_str(canvas, 102, 52, "ppm");

    } break;
    default:
        break;
    }
}

void co2_sensor_scene_main_on_enter(void* context) {
    CO2App* app = context;

    // Register callbacks
    view_port_draw_callback_set(app->viewport, render_callback, app);
    view_port_input_callback_set(app->viewport, input_callback, app);

    // Custom
    SCD4x_init(SCD4x_SENSOR_SCD40);
    //enableDebugging();
    if(!SCD4x_begin(true, app->settings.auto_calibration, false)) {
        app->status = NoSensor;
        furi_log_print_format(FuriLogLevelDebug, "SCD4x", "Begin: Fail");

    } else {
        app->status = Initializing;
        furi_log_print_format(FuriLogLevelDebug, "SCD4x", "Begin: OK");
    }

    // Create timer and register its callback
    FuriTimer* timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, app);
    app->main_ctx->timer = timer;
    furi_timer_start(timer, furi_ms_to_ticks(1000));
}

bool co2_sensor_scene_main_on_event(void* context, SceneManagerEvent event) {
    CO2App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
    } else if(event.type == SceneManagerEventTypeBack) {
    } else if(event.type == SceneManagerEventTypeTick) {
    }

    return consumed;
}

void co2_sensor_scene_main_on_exit(void* context) {
    CO2App* app = context;
    view_port_draw_callback_set(app->viewport, NULL, NULL);
    view_port_input_callback_set(app->viewport, NULL, NULL);
    furi_timer_stop(app->main_ctx->timer);
    furi_timer_free(app->main_ctx->timer);
    view_dispatcher_stop(app->view_dispatcher);
}