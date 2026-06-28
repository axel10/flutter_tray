#include "tray_impl_linux.h"

#include <cstring>

int TrayImplLinux::bus_id_counter_ = 0;

static const char kSNIPath[] = "/StatusNotifierItem";

const gchar* TrayImplLinux::kIntrospectionXML =
    "<node>"
    "  <interface name='org.kde.StatusNotifierItem'>"
    "    <method name='Activate'>"
    "      <arg name='x' type='i' direction='in'/>"
    "      <arg name='y' type='i' direction='in'/>"
    "    </method>"
    "    <method name='SecondaryActivate'>"
    "      <arg name='x' type='i' direction='in'/>"
    "      <arg name='y' type='i' direction='in'/>"
    "    </method>"
    "    <method name='ContextMenu'>"
    "      <arg name='x' type='i' direction='in'/>"
    "      <arg name='y' type='i' direction='in'/>"
    "    </method>"
    "    <property name='Category' type='s' access='read'/>"
    "    <property name='Id' type='s' access='read'/>"
    "    <property name='Title' type='s' access='read'/>"
    "    <property name='Status' type='s' access='read'/>"
    "    <property name='WindowId' type='i' access='read'/>"
    "    <property name='IconName' type='s' access='read'/>"
    "    <property name='IconPixmap' type='a(iiay)' access='read'/>"
    "    <property name='ItemIsMenu' type='b' access='read'/>"
    "  </interface>"
    "</node>";

TrayImplLinux::TrayImplLinux(FlMethodChannel* channel)
    : channel_(channel) {
  g_object_ref(channel_);
}

TrayImplLinux::~TrayImplLinux() {
  Destroy();
  g_object_unref(channel_);
}

bool TrayImplLinux::Init(const std::string& icon_path,
                          const std::string& tooltip) {
  icon_path_ = icon_path;
  tooltip_ = tooltip;

  GError* error = nullptr;
  connection_ = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
  if (!connection_) {
    g_warning("[flutter_tray] Failed to connect to session bus: %s",
              error ? error->message : "unknown");
    g_clear_error(&error);
    return false;
  }

  bus_id_counter_++;
  g_free(bus_name_);
  bus_name_ = g_strdup_printf("org.fluttertray.FlutterTray%d", bus_id_counter_);

  bus_owner_id_ = g_bus_own_name_on_connection(
      connection_,
      bus_name_,
      G_BUS_NAME_OWNER_FLAGS_NONE,
      OnNameAcquired,
      OnNameLost,
      this,
      nullptr);

  if (!bus_owner_id_) {
    g_warning("[flutter_tray] Failed to own bus name");
    g_object_unref(connection_);
    connection_ = nullptr;
    return false;
  }

  if (!SetupDBusObject()) {
    return false;
  }

  if (!RegisterWithWatcher()) {
    g_warning("[flutter_tray] Failed to register with StatusNotifierWatcher");
  }

  initialized_ = true;
  return true;
}

void TrayImplLinux::SetMenu(const std::vector<TrayMenuItem>& items) {
  menu_items_ = items;

  if (gtk_menu_) {
    gtk_widget_destroy(GTK_WIDGET(gtk_menu_));
    gtk_menu_ = nullptr;
  }
}

void TrayImplLinux::Destroy() {
  if (object_id_ > 0) {
    g_dbus_connection_unregister_object(connection_, object_id_);
    object_id_ = 0;
  }
  if (bus_owner_id_ > 0) {
    g_bus_unown_name(bus_owner_id_);
    bus_owner_id_ = 0;
  }
  g_free(bus_name_);
  bus_name_ = nullptr;

  if (connection_) {
    g_object_unref(connection_);
    connection_ = nullptr;
  }

  if (gtk_menu_) {
    gtk_widget_destroy(GTK_WIDGET(gtk_menu_));
    gtk_menu_ = nullptr;
  }

  initialized_ = false;
}

bool TrayImplLinux::SetupDBusObject() {
  g_autoptr(GError) error = nullptr;
  g_autoptr(GDBusNodeInfo) node_info =
      g_dbus_node_info_new_for_xml(kIntrospectionXML, &error);
  if (!node_info) {
    g_warning("[flutter_tray] Failed to parse introspection XML: %s",
              error->message);
    return false;
  }

  GDBusInterfaceInfo* iface_info = node_info->interfaces[0];

  GDBusInterfaceVTable vtable = {};
  vtable.method_call = [](GDBusConnection* conn, const gchar* sender,
                          const gchar* object_path,
                          const gchar* interface_name,
                          const gchar* method_name, GVariant* parameters,
                          GDBusMethodInvocation* invocation,
                          gpointer user_data) {
    auto* self = static_cast<TrayImplLinux*>(user_data);
    (void)conn;
    (void)sender;
    (void)object_path;
    (void)interface_name;
    (void)parameters;

    if (g_strcmp0(method_name, "Activate") == 0) {
      self->EmitEvent("leftClick", -1);
      g_dbus_method_invocation_return_value(invocation, nullptr);
    } else if (g_strcmp0(method_name, "SecondaryActivate") == 0) {
      self->ShowContextMenu();
      g_dbus_method_invocation_return_value(invocation, nullptr);
    } else if (g_strcmp0(method_name, "ContextMenu") == 0) {
      self->ShowContextMenu();
      g_dbus_method_invocation_return_value(invocation, nullptr);
    } else {
      g_dbus_method_invocation_return_error(
          invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD,
          "Unknown method: %s", method_name);
    }
  };

  vtable.get_property = [](GDBusConnection* conn, const gchar* sender,
                           const gchar* object_path,
                           const gchar* interface_name,
                           const gchar* property_name, GError** error,
                           gpointer user_data) -> GVariant* {
    (void)conn;
    (void)sender;
    (void)object_path;
    (void)interface_name;
    auto* self = static_cast<TrayImplLinux*>(user_data);

    if (g_strcmp0(property_name, "Category") == 0) {
      return g_variant_new_string("ApplicationStatus");
    }
    if (g_strcmp0(property_name, "Id") == 0) {
      return g_variant_new_string("flutter_tray");
    }
    if (g_strcmp0(property_name, "Title") == 0) {
      return g_variant_new_string(self->tooltip_.c_str());
    }
    if (g_strcmp0(property_name, "Status") == 0) {
      return g_variant_new_string("Active");
    }
    if (g_strcmp0(property_name, "WindowId") == 0) {
      return g_variant_new_int32(0);
    }
    if (g_strcmp0(property_name, "IconName") == 0) {
      return g_variant_new_string("");
    }
    if (g_strcmp0(property_name, "IconPixmap") == 0) {
      g_autoptr(GdkPixbuf) pixbuf = nullptr;
      if (!self->icon_path_.empty()) {
        GError* gerr = nullptr;
        pixbuf = gdk_pixbuf_new_from_file(self->icon_path_.c_str(), &gerr);
        if (!pixbuf) {
          g_warning("[flutter_tray] Failed to load icon: %s",
                    gerr ? gerr->message : "unknown");
          g_clear_error(&gerr);
        }
      }

      GVariantBuilder builder;
      g_variant_builder_init(&builder, G_VARIANT_TYPE("a(iiay)"));

      if (pixbuf) {
        int width = gdk_pixbuf_get_width(pixbuf);
        int height = gdk_pixbuf_get_height(pixbuf);
        int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
        int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
        const guchar* pixels = gdk_pixbuf_read_pixels(pixbuf);

        int size = width * height * 4;
        g_autofree guchar* argb = static_cast<guchar*>(g_malloc(size));

        for (int y = 0; y < height; y++) {
          for (int x = 0; x < width; x++) {
            const guchar* src = pixels + y * rowstride + x * n_channels;
            guchar* dst = argb + (y * width + x) * 4;
            dst[0] = (n_channels >= 4) ? src[3] : 255;
            dst[1] = src[0];
            dst[2] = src[1];
            dst[3] = src[2];
          }
        }

        g_variant_builder_add(&builder, "(ii@ay)", width, height,
                              g_variant_new_fixed_array(
                                  G_VARIANT_TYPE_BYTE, argb, size, 1));
      }

      return g_variant_builder_end(&builder);
    }
    if (g_strcmp0(property_name, "ItemIsMenu") == 0) {
      return g_variant_new_boolean(FALSE);
    }

    g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_PROPERTY,
                "Unknown property: %s", property_name);
    return nullptr;
  };

  object_id_ = g_dbus_connection_register_object(
      connection_, kSNIPath, iface_info, &vtable, this, nullptr, &error);

  if (!object_id_) {
    g_warning("[flutter_tray] Failed to register D-Bus object: %s",
              error->message);
    return false;
  }

  return true;
}

bool TrayImplLinux::RegisterWithWatcher() {
  const char* watcher_service = "org.kde.StatusNotifierWatcher";
  const char* watcher_path = "/StatusNotifierWatcher";

  g_autoptr(GError) error = nullptr;
  g_autoptr(GVariant) result = g_dbus_connection_call_sync(
      connection_,
      watcher_service,
      watcher_path,
      watcher_service,
      "RegisterStatusNotifierItem",
      g_variant_new("(s)", bus_name_),
      nullptr,
      G_DBUS_CALL_FLAGS_NONE,
      -1,
      nullptr,
      &error);

  if (error) {
    g_warning("[flutter_tray] Failed to register with watcher: %s",
              error->message);
    return false;
  }

  return true;
}

void TrayImplLinux::ShowContextMenu() {
  if (menu_items_.empty()) {
    EmitEvent("rightClick", -1);
    return;
  }

  if (gtk_menu_) {
    gtk_widget_destroy(GTK_WIDGET(gtk_menu_));
    gtk_menu_ = nullptr;
  }

  gtk_menu_ = GTK_MENU(gtk_menu_new());

  for (const auto& item : menu_items_) {
    GtkWidget* menu_item;
    if (item.is_separator) {
      menu_item = gtk_separator_menu_item_new();
    } else {
      menu_item = gtk_menu_item_new_with_label(item.label.c_str());
      gtk_widget_set_sensitive(menu_item, !item.disabled);
      g_object_set_data(G_OBJECT(menu_item), "tray-item-id",
                        GINT_TO_POINTER(item.id));
      g_signal_connect_data(
          menu_item, "activate",
          G_CALLBACK(+[](GtkWidget* widget, gpointer user_data) {
            auto* self = static_cast<TrayImplLinux*>(user_data);
            int id = GPOINTER_TO_INT(
                g_object_get_data(G_OBJECT(widget), "tray-item-id"));
            self->EmitEvent("menuClick", id);
          }),
          this, nullptr, G_CONNECT_AFTER);
    }
    gtk_menu_shell_append(GTK_MENU_SHELL(gtk_menu_), menu_item);
  }

  gtk_widget_show_all(GTK_WIDGET(gtk_menu_));

  gtk_menu_popup_at_pointer(gtk_menu_, nullptr);
}

void TrayImplLinux::EmitEvent(const char* type, int menu_id) {
  if (!channel_) return;

  g_autoptr(FlValue) args = fl_value_new_map();
  fl_value_set_string_take(args, "type", fl_value_new_string(type));
  fl_value_set_string_take(args, "menuId", fl_value_new_int(menu_id));

  fl_method_channel_invoke_method(channel_, "onTrayEvent", args,
                                  nullptr, nullptr, nullptr);
}

void TrayImplLinux::OnNameAcquired(GDBusConnection* connection,
                                    const gchar* name,
                                    gpointer user_data) {
  (void)connection;
  (void)user_data;
  g_print("[flutter_tray] Name acquired: %s\n", name);
}

void TrayImplLinux::OnNameLost(GDBusConnection* connection,
                                const gchar* name,
                                gpointer user_data) {
  (void)connection;
  (void)user_data;
  g_warning("[flutter_tray] Name lost: %s\n", name);
}
