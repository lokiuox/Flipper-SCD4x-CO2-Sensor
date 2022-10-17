#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <input/input.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/popup.h>
#include <core/log.h>
#include <notification/notification_messages.h>
#include <toolbox/saved_struct.h>
#include <storage/storage.h>
#include <string.h>
#include "scenes/co2_sensor_scene.h"

#define DATA_BUFFER_SIZE 8
#define CO2_SETTINGS_FILE_NAME ".co2sensor.settings"
#define CO2_SETTINGS_PATH EXT_PATH(CO2_SETTINGS_FILE_NAME)
#define CO2_SETTINGS_VERSION (0)
#define CO2_SETTINGS_MAGIC (0x42)

typedef enum {
    Initializing,
    NoSensor,
    PendingUpdate,
} SensorStatus;

typedef struct {
    char* temperature;
    char* humidity;
    char* co2;
} CO2Gui;

typedef enum { GUIModeNormal, GUIModeBigNumbers1, GUIModeBigNumbers2, GUIModeNum } CO2AppGUIMode;

typedef struct {
    bool low_power;
    bool auto_calibration;
    CO2AppGUIMode preferred_mode;
    bool library_debugging; //TODO implement
    bool backlight_always_on;
} CO2AppSettings;

typedef struct {
    FuriTimer* timer;
    CO2Gui display_data;
    CO2AppGUIMode selected_gui_mode;
} CO2AppMainSceneCtx;

typedef struct {
    Gui* gui;
    NotificationApp* notifications;
    ViewPort* viewport;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    SensorStatus status;
    CO2AppSettings settings;
    CO2AppMainSceneCtx* main_ctx;
} CO2App;

typedef enum { SceneEventExit } CO2AppSceneEvent;

extern const NotificationSequence sequence_blink_red_100;
extern const NotificationSequence sequence_blink_blue_100;
extern const NotificationSequence sequence_display_backlight_enforce_on;
extern const NotificationSequence sequence_display_backlight_enforce_auto;

bool co2_settings_load(CO2AppSettings* settings);
bool co2_settings_save(CO2AppSettings* settings);