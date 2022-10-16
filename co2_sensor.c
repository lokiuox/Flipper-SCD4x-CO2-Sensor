/* Flipper App to read the values from a SCD4X Sensor  */
#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <gui/modules/submenu.h>
#include <gui/modules/empty_screen.h>
#include <gui/modules/dialog_ex.h>
#include <toolbox/saved_struct.h>
#include <storage/storage.h>
#include <core/log.h>

#include <notification/notification_messages.h>

#include <string.h>
#include "co2_sensor.h"

static bool co2_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    CO2App* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool co2_app_back_event_callback(void* context) {
    furi_assert(context);
    CO2App* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

bool co2_settings_load(CO2SensorSettings* settings) {
    furi_assert(settings);

    return saved_struct_load(
        CO2_SETTINGS_PATH,
        settings,
        sizeof(CO2SensorSettings),
        CO2_SETTINGS_MAGIC,
        CO2_SETTINGS_VERSION);
}
bool co2_settings_save(CO2SensorSettings* settings) {
    furi_assert(settings);

    return saved_struct_save(
        CO2_SETTINGS_PATH,
        settings,
        sizeof(CO2SensorSettings),
        CO2_SETTINGS_MAGIC,
        CO2_SETTINGS_VERSION);
}

CO2App* co2_app_alloc() {
    CO2App* app = malloc(sizeof(CO2App));

    // Load settings
    if(!co2_settings_load(&app->settings)) {
        app->settings.low_power = false;
        app->settings.auto_calibration = false;
    }
    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    ViewPort* view_port = view_port_alloc();
    app->viewport = view_port;

    // Register viewport
    gui_add_view_port(app->gui, app->viewport, GuiLayerFullscreen);

    // Alloc object for main scene context
    CO2AppMainSceneCtx* main_ctx = malloc(sizeof(CO2AppMainSceneCtx));
    app->main_ctx = main_ctx;

    CO2Gui* display_data = malloc(sizeof(display_data));
    app->main_ctx->display_data.temperature = malloc(DATA_BUFFER_SIZE);
    app->main_ctx->display_data.humidity = malloc(DATA_BUFFER_SIZE);
    app->main_ctx->display_data.co2 = malloc(DATA_BUFFER_SIZE);

    // View Dispatcher and Scene Manager
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&co2_sensor_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, co2_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, co2_app_back_event_callback);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Set first scene
    scene_manager_next_scene(app->scene_manager, CO2SensorAppSceneMain);
    return app;
}

void co2_app_free(CO2App* app) {
    furi_assert(app);

    // View Dispatcher and Scene Manager
    view_port_enabled_set(app->viewport, false);
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);
    free(app->main_ctx);

    // Records
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);
    free(app);
}

extern int32_t co2_sensor_app(void* p) {
    UNUSED(p);
    CO2App* app = co2_app_alloc();
    view_dispatcher_run(app->view_dispatcher);
    co2_settings_save(&app->settings);
    co2_app_free(app);
    FURI_LOG_D("SCD4x", "COMPLETE");
    return 0;
}
