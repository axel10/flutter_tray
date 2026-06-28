#ifndef FLUTTER_PLUGIN_FLUTTER_TRAY_PLUGIN_H_
#define FLUTTER_PLUGIN_FLUTTER_TRAY_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <windows.h>
#include <memory>
#include <string>
#include <vector>

namespace flutter_tray {

struct TrayMenuItem {
  int id;
  std::string label;
  bool is_separator;
  bool disabled;
  bool checked;
};

class FlutterTrayPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows* registrar);

  FlutterTrayPlugin(std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel);
  virtual ~FlutterTrayPlugin();

  FlutterTrayPlugin(const FlutterTrayPlugin&) = delete;
  FlutterTrayPlugin& operator=(const FlutterTrayPlugin&) = delete;

  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

 private:
  bool InitTray(const std::string& icon_path, const std::string& tooltip);
  void SetMenu(const std::vector<TrayMenuItem>& items);
  void DestroyTray();
  void ShowContextMenu();
  void EmitEvent(const std::string& type, int menu_id);
  void CreateHiddenWindow();
  void DestroyHiddenWindow();

  static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel_;
  HWND hwnd_ = nullptr;
  HMENU hmenu_ = nullptr;
  NOTIFYICONDATAW nid_ = {};
  std::string icon_path_;
  std::string tooltip_;
  std::vector<TrayMenuItem> menu_items_;
  bool tray_created_ = false;
  bool initialized_ = false;
};

}  // namespace flutter_tray

#endif  // FLUTTER_PLUGIN_FLUTTER_TRAY_PLUGIN_H_
