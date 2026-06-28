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
  GVariant* BuildMenuLayout(int parent_id, int recursion_depth);
  void EmitLayoutUpdated();

  FlMethodChannel* channel_;
  std::string icon_path_;
  std::string tooltip_;
  std::vector<TrayMenuItem> menu_items_;

  GDBusConnection* connection_ = nullptr;
  GtkMenu* gtk_menu_ = nullptr;
  guint bus_owner_id_ = 0;
  std::vector<guint> object_ids_;
  gchar* bus_name_ = nullptr;
  guint dbusmenu_revision_ = 1;
  bool initialized_ = false;

  static int bus_id_counter_;
};

#endif  // TRAY_IMPL_LINUX_H_
