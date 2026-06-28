#ifndef TRAY_IMPL_LINUX_H_
#define TRAY_IMPL_LINUX_H_

#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>

#include <string>
#include <vector>

struct TrayMenuItem {
  int id;
  std::string label;
  bool is_separator;
  bool disabled;
  bool checked;
};

class TrayImplLinux {
 public:
  explicit TrayImplLinux(FlMethodChannel* channel);
  ~TrayImplLinux();

  bool Init(const std::string& icon_path, const std::string& tooltip);
  void SetMenu(const std::vector<TrayMenuItem>& items);
  void Destroy();

 private:
  bool SetupDBusObject();
  bool RegisterWithWatcher();
  void ShowContextMenu();
  void EmitEvent(const char* type, int menu_id);

  static void OnNameAcquired(GDBusConnection* connection,
                             const gchar* name,
                             gpointer user_data);
  static void OnNameLost(GDBusConnection* connection,
                         const gchar* name,
                         gpointer user_data);

  static const gchar* kIntrospectionXML;

  FlMethodChannel* channel_;
  std::string icon_path_;
  std::string tooltip_;
  std::vector<TrayMenuItem> menu_items_;

  GDBusConnection* connection_ = nullptr;
  GtkMenu* gtk_menu_ = nullptr;
  guint bus_owner_id_ = 0;
  guint object_id_ = 0;
  gchar* bus_name_ = nullptr;
  bool initialized_ = false;
  static int bus_id_counter_;
};

#endif  // TRAY_IMPL_LINUX_H_
