#include "../co2_sensor.h"
#include "../scd4x.h"

static void render_guimode_normal(Canvas* canvas, void* ctx);
static void render_guimode_bignumbers1(Canvas* canvas, void* ctx);
static void render_guimode_bignumbers2(Canvas* canvas, void* ctx);

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

    if(input_event->type == InputTypePress) {
        switch(input_event->key) {
        case InputKeyBack:
            scene_manager_stop(app->scene_manager);
            break;
        case InputKeyUp:
            app->main_ctx->selected_gui_mode = (app->main_ctx->selected_gui_mode + 1) % GUIModeNum;
            break;
        case InputKeyDown:
            app->main_ctx->selected_gui_mode =
                (app->main_ctx->selected_gui_mode + GUIModeNum - 1) % GUIModeNum;
            break;
        default:
            break;
        }
    }
}

static void render_callback(Canvas* canvas, void* ctx) {
    CO2App* app = ctx;
    char buf[8];

    canvas_clear(canvas);
    switch(app->status) {
    case Initializing:
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 2, 10, "CO2 Sensor");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 30, "Initializing..");
        break;
    case NoSensor:
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 2, 10, "CO2 Sensor");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 30, "No sensor found!");
        break;
    case PendingUpdate: {
        switch(app->main_ctx->selected_gui_mode) {
        case GUIModeNormal:
            render_guimode_normal(canvas, ctx);
            break;
        case GUIModeBigNumbers1:
            render_guimode_bignumbers1(canvas, ctx);
            break;
        case GUIModeBigNumbers2:
            render_guimode_bignumbers2(canvas, ctx);
            break;
        default:
            break;
        }

    } break;
    default:
        break;
    }
}

static void render_guimode_normal(Canvas* canvas, void* ctx) {
    CO2App* app = ctx;
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "CO2 Sensor");
    canvas_set_font(canvas, FontSecondary);

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
}

static void render_guimode_bignumbers1(Canvas* canvas, void* ctx) {
    CO2App* app = ctx;

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 118, 4, AlignRight, AlignTop, "o");

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 46, 18, AlignRight, AlignBottom, "Temp");
    canvas_draw_str_aligned(canvas, 46, 40, AlignRight, AlignBottom, "Humidity");
    canvas_draw_str_aligned(canvas, 46, 62, AlignRight, AlignBottom, "CO2");

    canvas_draw_str_aligned(canvas, 126, 18, AlignRight, AlignBottom, "C");
    canvas_draw_str_aligned(canvas, 126, 40, AlignRight, AlignBottom, "%");
    canvas_draw_str_aligned(canvas, 126, 62, AlignRight, AlignBottom, "ppm");

    // Draw horizontal lines
    canvas_draw_line(canvas, 0, 20, 128, 20);
    canvas_draw_line(canvas, 0, 42, 128, 42);

    // Draw temperature and humidity values
    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_str(canvas, 54, 18, app->main_ctx->display_data.temperature);
    canvas_draw_str(canvas, 54, 40, app->main_ctx->display_data.humidity);
    canvas_draw_str(canvas, 54, 62, app->main_ctx->display_data.co2);
}

static void render_guimode_bignumbers2(Canvas* canvas, void* ctx) {
    CO2App* app = ctx;

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 100, 4, AlignRight, AlignTop, "o");

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 26, 18, AlignRight, AlignBottom, "T");
    canvas_draw_str_aligned(canvas, 26, 40, AlignRight, AlignBottom, "H");
    canvas_draw_str_aligned(canvas, 26, 62, AlignRight, AlignBottom, "CO2");

    canvas_draw_str_aligned(canvas, 102, 18, AlignLeft, AlignBottom, "C");
    canvas_draw_str_aligned(canvas, 102, 40, AlignLeft, AlignBottom, "%");
    canvas_draw_str_aligned(canvas, 102, 62, AlignLeft, AlignBottom, "ppm");

    // Draw horizontal lines
    canvas_draw_line(canvas, 0, 20, 128, 20);
    canvas_draw_line(canvas, 0, 42, 128, 42);

    // Draw temperature and humidity values
    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_str_aligned(
        canvas, 92, 18, AlignRight, AlignBottom, app->main_ctx->display_data.temperature);
    canvas_draw_str_aligned(
        canvas, 92, 40, AlignRight, AlignBottom, app->main_ctx->display_data.humidity);
    canvas_draw_str_aligned(
        canvas, 92, 62, AlignRight, AlignBottom, app->main_ctx->display_data.co2);
}

void co2_sensor_scene_main_on_enter(void* context) {
    CO2App* app = context;

    // Register callbacks
    view_port_draw_callback_set(app->viewport, render_callback, app);
    view_port_input_callback_set(app->viewport, input_callback, app);
    app->main_ctx->selected_gui_mode = app->settings.preferred_mode;

    if(app->settings.backlight_always_on) {
        notification_message(app->notifications, &sequence_display_backlight_enforce_on);
    }

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
    notification_message(app->notifications, &sequence_display_backlight_enforce_auto);
    app->settings.preferred_mode = app->main_ctx->selected_gui_mode;
    view_port_draw_callback_set(app->viewport, NULL, NULL);
    view_port_input_callback_set(app->viewport, NULL, NULL);
    furi_timer_stop(app->main_ctx->timer);
    furi_timer_free(app->main_ctx->timer);
    view_dispatcher_stop(app->view_dispatcher);
}