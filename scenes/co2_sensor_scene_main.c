#include "../co2_sensor.h"
#include "../scd4x.h"

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

        //canvas_draw_str(canvas, 80, 24, "Humidity");

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

    CO2AppMainSceneCtx* main_ctx = malloc(sizeof(CO2AppMainSceneCtx));
    app->main_ctx = main_ctx;

    CO2Gui* display_data = malloc(sizeof(display_data));
    app->main_ctx->display_data.temperature = malloc(DATA_BUFFER_SIZE);
    app->main_ctx->display_data.humidity = malloc(DATA_BUFFER_SIZE);
    app->main_ctx->display_data.co2 = malloc(DATA_BUFFER_SIZE);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));
    app->main_ctx->event_queue = event_queue;

    // Register callbacks
    ViewPort* view_port = view_port_alloc();
    app->main_ctx->viewport = view_port;
    view_port_draw_callback_set(view_port, render_callback, app);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Register viewport
    gui_add_view_port(app->gui, view_port, GuiLayerFullscreen);

    // Custom
    SCD4x_init(SCD4x_SENSOR_SCD40);
    enableDebugging();
    if(!SCD4x_begin(true, app->settings.auto_calibration, false)) {
        app->status = NoSensor;
        furi_log_print_format(FuriLogLevelDebug, "SCD4x", "Begin: Fail");

    } else {
        app->status = Initializing;
        furi_log_print_format(FuriLogLevelDebug, "SCD4x", "Begin: OK");
    }

    // Declare our variables
    PluginEvent tsEvent;
    float celsius, humidity = 0.0;
    uint16_t co2 = 0;

    // Create timer and register its callback
    FuriTimer* timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, event_queue);
    app->main_ctx->timer = timer;
    furi_timer_start(timer, furi_ms_to_ticks(1000));

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
                app->status = PendingUpdate;

                notification_message(app->notifications, &sequence_blink_blue_100);

                snprintf(
                    main_ctx->display_data.temperature, DATA_BUFFER_SIZE, "%.2f", (double)celsius);
                snprintf(
                    main_ctx->display_data.humidity, DATA_BUFFER_SIZE, "%.2f", (double)humidity);
                snprintf(main_ctx->display_data.co2, DATA_BUFFER_SIZE, "%d", (double)co2);
            }
        }
        furi_delay_tick(furi_ms_to_ticks(100));
    }
}

bool co2_sensor_scene_main_on_event(void* context, SceneManagerEvent event) {
    CO2App* app = context;
    bool consumed = false;

    FURI_LOG_D("SCD4x", "Got event.");

    /*
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == BtSettingOn) {
            furi_hal_bt_start_advertising();
            app->settings.enabled = true;
            consumed = true;
        } else if(event.event == BtSettingOff) {
            app->settings.enabled = false;
            furi_hal_bt_stop_advertising();
            consumed = true;
        } else if(event.event == BtSettingsCustomEventForgetDevices) {
            scene_manager_next_scene(app->scene_manager, BtSettingsAppSceneForgetDevConfirm);
            consumed = true;
        }
    }
    */
    return consumed;
}

void co2_sensor_scene_main_on_exit(void* context) {
    CO2App* app = context;

    furi_timer_free(app->main_ctx->timer);
    gui_remove_view_port(app->gui, app->main_ctx->viewport);
    view_port_free(app->main_ctx->viewport);
    furi_message_queue_free(app->main_ctx->event_queue);
    free(app->main_ctx);
    app->main_ctx = NULL;
}