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

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    char* temperature;
    char* humidity;
    char* co2;
} CO2Gui;

typedef struct {
    bool low_power;
    bool auto_calibration;
} CO2SensorSettings;

typedef struct {
    FuriTimer* timer;
    ViewPort* viewport;
    FuriMessageQueue* event_queue;
    CO2Gui display_data;
} CO2AppMainSceneCtx;

typedef struct {
    Gui* gui;
    NotificationApp* notifications;
    //ViewPort* view_port; //TODO: not sure if needed
    //FuriMessageQueue* event_queue; //TODO: not sure if needed
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    VariableItemList* var_item_list;
    DialogEx* dialog;
    Popup* popup;
    SensorStatus status;
    CO2SensorSettings settings;
    CO2AppMainSceneCtx* main_ctx;
} CO2App;

typedef enum {
    CO2AppViewMain,
    CO2AppViewVarItemList,
    CO2AppViewAppViewDialog,
    CO2AppViewAppViewPopup,
} CO2AppView;

typedef enum { SceneEventExit } CO2AppSceneEvent;

extern const NotificationSequence sequence_blink_red_100;
extern const NotificationSequence sequence_blink_blue_100;

bool co2_settings_load(CO2SensorSettings* settings);
bool co2_settings_save(CO2SensorSettings* settings);