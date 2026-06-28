#include "include/flutter_tray/flutter_tray_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "flutter_tray_plugin.h"

void FlutterTrayPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  flutter_tray::FlutterTrayPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
