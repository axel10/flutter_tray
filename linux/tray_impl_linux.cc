#include "tray_impl_linux.h"

#include <cstring>

int TrayImplLinux::bus_id_counter_ = 0;

static const char kSNIPath[] = "/StatusNotifierItem";

static const char kIntrospectionXML[] =
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
    "    <method name='Scroll'>"
    "      <arg name='delta' type='i' direction='in'/>"
    "      <arg name='orientation' type='s' direction='in'/>"
    "    </method>"
    "    <signal name='NewIcon'/>"
    "    <signal name='NewAttentionIcon'/>"
    "    <signal name='NewOverlayIcon'/>"
    "    <signal name='NewToolTip'/>"
    "    <signal name='NewStatus'>"
    "      <arg name='status' type='s'/>"
    "    </signal>"
    "    <signal name='NewTitle'/>"
    "    <signal name='NewIconThemePath'>"
    "      <arg name='icon_theme_path' type='s'/>"
    "    </signal>"
    "    <property name='Category' type='s' access='read'/>"
    "    <property name='Id' type='s' access='read'/>"
    "    <property name='Title' type='s' access='read'/>"
    "    <property name='Status' type='s' access='read'/>"
    "    <property name='WindowId' type='u' access='read'/>"
    "    <property name='IconName' type='s' access='read'/>"
    "    <property name='IconPixmap' type='a(iiay)' access='read'/>"
    "    <property name='OverlayIconName' type='s' access='read'/>"
    "    <property name='OverlayIconPixmap' type='a(iiay)' access='read'/>"
    "    <property name='AttentionIconName' type='s' access='read'/>"
    "    <property name='AttentionIconPixmap' type='a(iiay)' access='read'/>"
    "    <property name='AttentionMovieName' type='s' access='read'/>"
    "    <property name='ToolTip' type='(sa(iiay)ss)' access='read'/>"
    "    <property name='IconThemePath' type='s' access='read'/>"
    "    <property name='Menu' type='o' access='read'/>"
    "    <property name='ItemIsMenu' type='b' access='read'/>"
    "    <property name='IconAccessibleDesc' type='s' access='read'/>"
    "    <property name='AttentionAccessibleDesc' type='s' access='read'/>"
    "  </interface>"
    "  <interface name='org.freedesktop.StatusNotifierItem'>"
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
    "    <method name='Scroll'>"
    "      <arg name='delta' type='i' direction='in'/>"
    "      <arg name='orientation' type='s' direction='in'/>"
    "    </method>"
    "    <signal name='NewIcon'/>"
    "    <signal name='NewAttentionIcon'/>"
    "    <signal name='NewOverlayIcon'/>"
    "    <signal name='NewToolTip'/>"
    "    <signal name='NewStatus'>"
    "      <arg name='status' type='s'/>"
    "    </signal>"
    "    <signal name='NewTitle'/>"
    "    <signal name='NewIconThemePath'>"
    "      <arg name='icon_theme_path' type='s'/>"
    "    </signal>"
    "    <property name='Category' type='s' access='read'/>"
    "    <property name='Id' type='s' access='read'/>"
    "    <property name='Title' type='s' access='read'/>"
    "    <property name='Status' type='s' access='read'/>"
    "    <property name='WindowId' type='u' access='read'/>"
    "    <property name='IconName' type='s' access='read'/>"
    "    <property name='IconPixmap' type='a(iiay)' access='read'/>"
    "    <property name='OverlayIconName' type='s' access='read'/>"
    "    <property name='OverlayIconPixmap' type='a(iiay)' access='read'/>"
    "    <property name='AttentionIconName' type='s' access='read'/>"
    "    <property name='AttentionIconPixmap' type='a(iiay)' access='read'/>"
    "    <property name='AttentionMovieName' type='s' access='read'/>"
    "    <property name='ToolTip' type='(sa(iiay)ss)' access='read'/>"
    "    <property name='IconThemePath' type='s' access='read'/>"
    "    <property name='Menu' type='o' access='read'/>"
    "    <property name='ItemIsMenu' type='b' access='read'/>"
    "    <property name='IconAccessibleDesc' type='s' access='read'/>"
    "    <property name='AttentionAccessibleDesc' type='s' access='read'/>"
    "  </interface>"
    "  <interface name='com.canonical.dbusmenu'>"
    "    <method name='GetLayout'>"
    "      <arg name='parentId' type='i' direction='in'/>"
    "      <arg name='recursionDepth' type='i' direction='in'/>"
    "      <arg name='propertyNames' type='as' direction='in'/>"
    "      <arg name='revision' type='u' direction='out'/>"
    "      <arg name='layout' type='(ia{sv}av)' direction='out'/>"
    "    </method>"
    "    <method name='Event'>"
    "      <arg name='id' type='i' direction='in'/>"
    "      <arg name='eventId' type='s' direction='in'/>"
    "      <arg name='data' type='v' direction='in'/>"
    "      <arg name='timestamp' type='u' direction='in'/>"
    "    </method>"
    "    <method name='AboutToShow'>"
    "      <arg name='id' type='i' direction='in'/>"
    "      <arg name='needUpdate' type='b' direction='out'/>"
    "    </method>"
    "    <signal name='LayoutUpdated'>"
    "      <arg name='revision' type='u'/>"
    "      <arg name='parentId' type='i'/>"
    "    </signal>"
    "    <signal name='ItemsPropertiesUpdated'>"
    "      <arg name='updatedProps' type='a(ia{sv})'/>"
    "      <arg name='removedProps' type='a(ias)'/>"
    "    </signal>"
    "    <property name='Version' type='u' access='read'/>"
    "    <property name='Status' type='s' access='read'/>"
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
      nullptr,
      nullptr,
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

  g_dbus_connection_emit_signal(
      connection_, nullptr, kSNIPath,
      "org.kde.StatusNotifierItem", "NewIcon", nullptr, nullptr);

  if (!RegisterWithWatcher()) {
    g_warning("[flutter_tray] Failed to register with StatusNotifierWatcher");
  }

  initialized_ = true;
  return true;
}

void TrayImplLinux::SetMenu(const std::vector<TrayMenuItem>& items) {
  menu_items_ = items;
  EmitLayoutUpdated();

  if (gtk_menu_) {
    gtk_widget_destroy(GTK_WIDGET(gtk_menu_));
    gtk_menu_ = nullptr;
  }
}

void TrayImplLinux::Destroy() {
  if (connection_) {
    for (auto id : object_ids_) {
      g_dbus_connection_unregister_object(connection_, id);
    }
    object_ids_.clear();
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

  GDBusInterfaceVTable vtable = {};

  vtable.method_call = [](GDBusConnection* conn, const gchar* sender,
                          const gchar* object_path,
                          const gchar* interface_name,
                          const gchar* method_name, GVariant* parameters,
                          GDBusMethodInvocation* invocation,
                          gpointer user_data) {
    (void)conn;
    (void)sender;
    (void)object_path;
    auto* self = static_cast<TrayImplLinux*>(user_data);

    bool is_sni = (g_strcmp0(interface_name, "org.kde.StatusNotifierItem") == 0 ||
                   g_strcmp0(interface_name, "org.freedesktop.StatusNotifierItem") == 0);

    if (is_sni) {
      if (g_strcmp0(method_name, "Activate") == 0) {
        self->EmitEvent("leftClick", -1);
        g_dbus_method_invocation_return_value(invocation, nullptr);
      } else if (g_strcmp0(method_name, "SecondaryActivate") == 0) {
        self->EmitEvent("rightClick", -1);
        g_dbus_method_invocation_return_value(invocation, nullptr);
      } else if (g_strcmp0(method_name, "ContextMenu") == 0) {
        self->ShowContextMenu();
        g_dbus_method_invocation_return_value(invocation, nullptr);
      } else if (g_strcmp0(method_name, "Scroll") == 0) {
        g_dbus_method_invocation_return_value(invocation, nullptr);
      } else {
        g_dbus_method_invocation_return_error(
            invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD,
            "Unknown method: %s", method_name);
      }
      return;
    }

    if (g_strcmp0(interface_name, "com.canonical.dbusmenu") == 0) {
      if (g_strcmp0(method_name, "GetLayout") == 0) {
        gint32 parent_id = 0;
        gint32 recursion_depth = -1;
        g_variant_get_child(parameters, 0, "i", &parent_id);
        g_variant_get_child(parameters, 1, "i", &recursion_depth);

        g_autoptr(GVariant) layout = g_variant_ref_sink(self->BuildMenuLayout(parent_id, recursion_depth));
        g_dbus_method_invocation_return_value(invocation,
            g_variant_new("(u@(ia{sv}av))",
                          self->dbusmenu_revision_, layout));
      } else if (g_strcmp0(method_name, "Event") == 0) {
        gint32 id = 0;
        g_autofree gchar* event_id = nullptr;
        g_variant_get_child(parameters, 0, "i", &id);
        g_variant_get_child(parameters, 1, "s", &event_id);

        if (g_strcmp0(event_id, "clicked") == 0) {
          for (const auto& item : self->menu_items_) {
            if (item.id == id) {
              self->EmitEvent("menuClick", id);
              break;
            }
          }
        }
        g_dbus_method_invocation_return_value(invocation, nullptr);
      } else if (g_strcmp0(method_name, "AboutToShow") == 0) {
        g_dbus_method_invocation_return_value(invocation,
            g_variant_new("(b)", FALSE));
      } else {
        g_dbus_method_invocation_return_error(
            invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD,
            "Unknown method: %s", method_name);
      }
      return;
    }

    if (g_strcmp0(interface_name, "org.freedesktop.DBus.Properties") == 0) {
      g_dbus_method_invocation_return_error(
          invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD,
          "Use GetAll instead");
      return;
    }

    g_dbus_method_invocation_return_error(
        invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_INTERFACE,
        "Unknown interface: %s", interface_name);
  };

  vtable.get_property = [](GDBusConnection* conn, const gchar* sender,
                           const gchar* object_path,
                           const gchar* interface_name,
                           const gchar* property_name, GError** error,
                           gpointer user_data) -> GVariant* {
    (void)conn;
    (void)sender;
    (void)object_path;
    auto* self = static_cast<TrayImplLinux*>(user_data);

    if (g_strcmp0(interface_name, "com.canonical.dbusmenu") == 0) {
      if (g_strcmp0(property_name, "Version") == 0) {
        return g_variant_new_uint32(3);
      }
      if (g_strcmp0(property_name, "Status") == 0) {
        return g_variant_new_string("normal");
      }
      g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_PROPERTY,
                  "Unknown property: %s", property_name);
      return nullptr;
    }

    bool is_sni = (g_strcmp0(interface_name, "org.kde.StatusNotifierItem") == 0 ||
                   g_strcmp0(interface_name, "org.freedesktop.StatusNotifierItem") == 0);

    if (!is_sni && g_strcmp0(interface_name, "org.freedesktop.DBus.Properties") != 0) {
      g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_INTERFACE,
                  "Unknown interface: %s", interface_name);
      return nullptr;
    }

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
      return g_variant_new_uint32(0);
    }
    if (g_strcmp0(property_name, "ItemIsMenu") == 0) {
      return g_variant_new_boolean(FALSE);
    }

    if (g_strcmp0(property_name, "IconName") == 0) {
      if (!self->icon_path_.empty()) {
        const gchar* sep = g_strrstr(self->icon_path_.c_str(), "/");
        const gchar* basename = sep ? sep + 1 : self->icon_path_.c_str();
        g_autofree gchar* name = g_strdup(basename);
        gchar* dot = g_strrstr(name, ".");
        if (dot) *dot = '\0';
        return g_variant_new_string(name);
      }
      return g_variant_new_string("application-x-executable");
    }

    if (g_strcmp0(property_name, "IconPixmap") == 0) {
      GError* gerr = nullptr;
      g_autoptr(GdkPixbuf) pixbuf = nullptr;
      if (!self->icon_path_.empty()) {
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
        int w = gdk_pixbuf_get_width(pixbuf);
        int h = gdk_pixbuf_get_height(pixbuf);
        int rs = gdk_pixbuf_get_rowstride(pixbuf);
        int nc = gdk_pixbuf_get_n_channels(pixbuf);
        const guchar* px = gdk_pixbuf_read_pixels(pixbuf);

        int size = w * h * 4;
        g_autofree guchar* argb = static_cast<guchar*>(g_malloc(size));
        for (int y = 0; y < h; y++) {
          for (int x = 0; x < w; x++) {
            const guchar* s = px + y * rs + x * nc;
            guchar* d = argb + (y * w + x) * 4;
            d[0] = (nc >= 4) ? s[3] : 255;
            d[1] = s[0];
            d[2] = s[1];
            d[3] = s[2];
          }
        }

        g_variant_builder_add(&builder, "(ii@ay)", w, h,
                              g_variant_new_fixed_array(
                                  G_VARIANT_TYPE_BYTE, argb, size, 1));
      }

      return g_variant_builder_end(&builder);
    }

    if (g_strcmp0(property_name, "AttentionIconName") == 0 ||
        g_strcmp0(property_name, "AttentionMovieName") == 0 ||
        g_strcmp0(property_name, "OverlayIconName") == 0 ||
        g_strcmp0(property_name, "IconAccessibleDesc") == 0 ||
        g_strcmp0(property_name, "AttentionAccessibleDesc") == 0) {
      return g_variant_new_string("");
    }

    if (g_strcmp0(property_name, "AttentionIconPixmap") == 0 ||
        g_strcmp0(property_name, "OverlayIconPixmap") == 0) {
      GVariantBuilder builder;
      g_variant_builder_init(&builder, G_VARIANT_TYPE("a(iiay)"));
      return g_variant_builder_end(&builder);
    }

    if (g_strcmp0(property_name, "ToolTip") == 0) {
      GVariantBuilder pixmaps;
      g_variant_builder_init(&pixmaps, G_VARIANT_TYPE("a(iiay)"));
      return g_variant_new("(s@a(iiay)ss)",
                           "",
                           g_variant_builder_end(&pixmaps),
                           self->tooltip_.c_str(),
                           "");
    }

    if (g_strcmp0(property_name, "IconThemePath") == 0) {
      if (!self->icon_path_.empty()) {
        const gchar* sep = g_strrstr(self->icon_path_.c_str(), "/");
        if (sep) {
          g_autofree gchar* dir = g_strndup(self->icon_path_.c_str(),
                                            sep - self->icon_path_.c_str());
          return g_variant_new_string(dir);
        }
      }
      return g_variant_new_string("");
    }

    if (g_strcmp0(property_name, "Menu") == 0) {
      return g_variant_new_object_path(kSNIPath);
    }

    g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_PROPERTY,
                "Unknown property: %s", property_name);
    return nullptr;
  };

  for (int i = 0; node_info->interfaces[i] != nullptr; i++) {
    GDBusInterfaceInfo* iface_info = node_info->interfaces[i];
    g_clear_error(&error);
    guint id = g_dbus_connection_register_object(
        connection_, kSNIPath, iface_info, &vtable, this, nullptr, &error);
    if (id == 0) {
      g_warning("[flutter_tray] Failed to register D-Bus interface %s: %s",
                iface_info->name, error->message);
      return false;
    }
    object_ids_.push_back(id);
  }

  return true;
}

bool TrayImplLinux::RegisterWithWatcher() {
  const char* watchers[][2] = {
    {"org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher"},
    {"org.freedesktop.StatusNotifierWatcher", "/org/freedesktop/StatusNotifierWatcher"},
    {nullptr, nullptr}
  };

  for (int i = 0; watchers[i][0] != nullptr; i++) {
    g_autoptr(GError) error = nullptr;
    g_autoptr(GVariant) result = g_dbus_connection_call_sync(
        connection_,
        watchers[i][0],
        watchers[i][1],
        watchers[i][0],
        "RegisterStatusNotifierItem",
        g_variant_new("(s)", bus_name_),
        nullptr,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        nullptr,
        &error);

    if (!error) {
      g_print("[flutter_tray] Registered with %s\n", watchers[i][0]);
      return true;
    }
    g_warning("[flutter_tray] %s not available: %s",
              watchers[i][0], error->message);
  }

  return false;
}

GVariant* TrayImplLinux::BuildMenuLayout(int parent_id, int recursion_depth) {
  GVariantBuilder props;
  g_variant_builder_init(&props, G_VARIANT_TYPE("a{sv}"));
  GVariantBuilder children;
  g_variant_builder_init(&children, G_VARIANT_TYPE("av"));

  if (parent_id == 0 && recursion_depth != 0) {
    for (const auto& item : menu_items_) {
      GVariantBuilder iprops;
      g_variant_builder_init(&iprops, G_VARIANT_TYPE("a{sv}"));

      if (item.is_separator) {
        g_variant_builder_add(&iprops, "{sv}", "type",
                              g_variant_new_string("separator"));
      } else {
        g_variant_builder_add(&iprops, "{sv}", "label",
                              g_variant_new_string(item.label.c_str()));
        g_variant_builder_add(&iprops, "{sv}", "enabled",
                              g_variant_new_boolean(!item.disabled));
      }

      GVariantBuilder empty_children;
      g_variant_builder_init(&empty_children, G_VARIANT_TYPE("av"));

      g_variant_builder_add(&children, "v",
                            g_variant_new("(i@a{sv}@av)",
                                          item.id,
                                          g_variant_builder_end(&iprops),
                                          g_variant_builder_end(&empty_children)));
    }
  }

  return g_variant_new("(i@a{sv}@av)",
                       parent_id,
                       g_variant_builder_end(&props),
                       g_variant_builder_end(&children));
}

void TrayImplLinux::EmitLayoutUpdated() {
  if (!connection_) return;
  dbusmenu_revision_++;
  g_dbus_connection_emit_signal(
      connection_, nullptr, kSNIPath,
      "com.canonical.dbusmenu", "LayoutUpdated",
      g_variant_new("(ui)", dbusmenu_revision_, 0),
      nullptr);
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
    GtkWidget* mi;
    if (item.is_separator) {
      mi = gtk_separator_menu_item_new();
    } else {
      mi = gtk_menu_item_new_with_label(item.label.c_str());
      gtk_widget_set_sensitive(mi, !item.disabled);
      g_object_set_data(G_OBJECT(mi), "tray-item-id",
                        GINT_TO_POINTER(item.id));
      g_signal_connect_data(mi, "activate",
                            G_CALLBACK(+[](GtkWidget* w, gpointer d) {
                              auto* self = static_cast<TrayImplLinux*>(d);
                              int id = GPOINTER_TO_INT(
                                  g_object_get_data(G_OBJECT(w), "tray-item-id"));
                              self->EmitEvent("menuClick", id);
                            }),
                            this, nullptr, G_CONNECT_AFTER);
    }
    gtk_menu_shell_append(GTK_MENU_SHELL(gtk_menu_), mi);
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
