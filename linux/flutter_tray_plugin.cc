#include "include/flutter_tray/flutter_tray_plugin.h"

#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>

#include <cstring>
#include <string>
#include <vector>

#include "flutter_tray_plugin_private.h"
#include "tray_impl_linux.h"

#define FLUTTER_TRAY_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), flutter_tray_plugin_get_type(), \
                              FlutterTrayPlugin))

struct _FlutterTrayPlugin {
  GObject parent_instance;

  FlMethodChannel* channel = nullptr;
  TrayImplLinux* tray = nullptr;
};

G_DEFINE_TYPE(FlutterTrayPlugin, flutter_tray_plugin, g_object_get_type())

static void flutter_tray_plugin_handle_method_call(
    FlutterTrayPlugin* self,
    FlMethodCall* method_call) {
  g_autoptr(FlMethodResponse) response = nullptr;

  const gchar* method = fl_method_call_get_name(method_call);

  if (strcmp(method, "initTray") == 0) {
    FlValue* args = fl_method_call_get_args(method_call);
    if (!args || fl_value_get_type(args) != FL_VALUE_TYPE_MAP) {
      response = FL_METHOD_RESPONSE(
          fl_method_error_response_new("INVALID_ARGS", "Expected map", nullptr));
      fl_method_call_respond(method_call, response, nullptr);
      return;
    }

    FlValue* icon_val = fl_value_lookup_string(args, "iconPath");
    FlValue* tooltip_val = fl_value_lookup_string(args, "tooltip");

    if (!icon_val || !tooltip_val) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          "INVALID_ARGS", "iconPath and tooltip required", nullptr));
      fl_method_call_respond(method_call, response, nullptr);
      return;
    }

    std::string icon_path(fl_value_get_string(icon_val));
    std::string tooltip(fl_value_get_string(tooltip_val));

    if (self->tray) {
      delete self->tray;
      self->tray = nullptr;
    }

    self->tray = new TrayImplLinux(self->channel);
    bool ok = self->tray->Init(icon_path, tooltip);

    g_autoptr(FlValue) result = fl_value_new_bool(ok);
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  } else if (strcmp(method, "setMenu") == 0) {
    FlValue* args = fl_method_call_get_args(method_call);
    if (!args || fl_value_get_type(args) != FL_VALUE_TYPE_MAP) {
      response = FL_METHOD_RESPONSE(
          fl_method_error_response_new("INVALID_ARGS", "Expected map", nullptr));
      fl_method_call_respond(method_call, response, nullptr);
      return;
    }

    FlValue* items_val = fl_value_lookup_string(args, "items");
    if (!items_val || fl_value_get_type(items_val) != FL_VALUE_TYPE_LIST) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          "INVALID_ARGS", "items list required", nullptr));
      fl_method_call_respond(method_call, response, nullptr);
      return;
    }

    std::vector<TrayMenuItem> items;
    for (size_t i = 0; i < fl_value_get_length(items_val); i++) {
      FlValue* item_val = fl_value_get_list_value(items_val, i);
      if (!item_val || fl_value_get_type(item_val) != FL_VALUE_TYPE_MAP) {
        continue;
      }

      TrayMenuItem item = {};
      FlValue* id_val = fl_value_lookup_string(item_val, "id");
      FlValue* label_val = fl_value_lookup_string(item_val, "label");
      FlValue* sep_val = fl_value_lookup_string(item_val, "isSeparator");
      FlValue* disabled_val = fl_value_lookup_string(item_val, "disabled");
      FlValue* checked_val = fl_value_lookup_string(item_val, "checked");

      item.id = id_val ? fl_value_get_int(id_val) : 0;
      item.label = label_val ? fl_value_get_string(label_val) : "";
      item.is_separator = sep_val ? fl_value_get_bool(sep_val) : false;
      item.disabled = disabled_val ? fl_value_get_bool(disabled_val) : false;
      item.checked = checked_val ? fl_value_get_bool(checked_val) : false;

      items.push_back(item);
    }

    if (self->tray) {
      self->tray->SetMenu(items);
    }

    response = FL_METHOD_RESPONSE(
        fl_method_success_response_new(fl_value_new_null()));
  } else if (strcmp(method, "destroy") == 0) {
    if (self->tray) {
      delete self->tray;
      self->tray = nullptr;
    }

    response = FL_METHOD_RESPONSE(
        fl_method_success_response_new(fl_value_new_null()));
  } else {
    response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
  }

  fl_method_call_respond(method_call, response, nullptr);
}

static void flutter_tray_plugin_dispose(GObject* object) {
  FlutterTrayPlugin* self = FLUTTER_TRAY_PLUGIN(object);

  if (self->tray) {
    delete self->tray;
    self->tray = nullptr;
  }

  g_clear_object(&self->channel);

  G_OBJECT_CLASS(flutter_tray_plugin_parent_class)->dispose(object);
}

static void flutter_tray_plugin_class_init(FlutterTrayPluginClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = flutter_tray_plugin_dispose;
}

static void flutter_tray_plugin_init(FlutterTrayPlugin* self) {}

static void method_call_cb(FlMethodChannel* channel, FlMethodCall* method_call,
                           gpointer user_data) {
  FlutterTrayPlugin* plugin = FLUTTER_TRAY_PLUGIN(user_data);
  flutter_tray_plugin_handle_method_call(plugin, method_call);
}

void flutter_tray_plugin_register_with_registrar(FlPluginRegistrar* registrar) {
  FlutterTrayPlugin* plugin = FLUTTER_TRAY_PLUGIN(
      g_object_new(flutter_tray_plugin_get_type(), nullptr));

  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  plugin->channel =
      fl_method_channel_new(fl_plugin_registrar_get_messenger(registrar),
                            "flutter_tray",
                            FL_METHOD_CODEC(codec));
  fl_method_channel_set_method_call_handler(plugin->channel, method_call_cb,
                                            g_object_ref(plugin),
                                            g_object_unref);

  g_object_unref(plugin);
}
