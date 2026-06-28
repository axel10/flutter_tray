#include "flutter_tray_plugin.h"

#include <windows.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <sstream>
#include <string>

namespace flutter_tray {

constexpr UINT WM_TRAY_CALLBACK = WM_APP + 1;
static UINT WM_TASKBAR_CREATED = 0;

std::wstring Utf8ToWide(const std::string& utf8) {
  if (utf8.empty()) return std::wstring();
  int size_needed = MultiByteToWideChar(CP_UTF8, 0, &utf8[0], (int)utf8.size(), NULL, 0);
  std::wstring wstrTo(size_needed, 0);
  MultiByteToWideChar(CP_UTF8, 0, &utf8[0], (int)utf8.size(), &wstrTo[0], size_needed);
  return wstrTo;
}

void FlutterTrayPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows* registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "flutter_tray",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<FlutterTrayPlugin>(std::move(channel));

  plugin->channel_->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto& call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

FlutterTrayPlugin::FlutterTrayPlugin(
    std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel)
    : channel_(std::move(channel)) {
  WM_TASKBAR_CREATED = RegisterWindowMessageW(L"TaskbarCreated");
  memset(&nid_, 0, sizeof(nid_));
  nid_.cbSize = sizeof(NOTIFYICONDATAW);
}

FlutterTrayPlugin::~FlutterTrayPlugin() {
  DestroyTray();
  DestroyHiddenWindow();
}

void FlutterTrayPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  const std::string& method = method_call.method_name();

  if (method == "initTray") {
    const auto* args = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!args) {
      result->Error("INVALID_ARGS", "Expected map");
      return;
    }

    std::string icon_path;
    std::string tooltip;

    auto icon_it = args->find(flutter::EncodableValue("iconPath"));
    if (icon_it != args->end()) {
      icon_path = std::get<std::string>(icon_it->second);
    }

    auto tooltip_it = args->find(flutter::EncodableValue("tooltip"));
    if (tooltip_it != args->end()) {
      tooltip = std::get<std::string>(tooltip_it->second);
    }

    bool ok = InitTray(icon_path, tooltip);
    result->Success(flutter::EncodableValue(ok));
  } else if (method == "setMenu") {
    const auto* args = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!args) {
      result->Error("INVALID_ARGS", "Expected map");
      return;
    }

    auto items_it = args->find(flutter::EncodableValue("items"));
    if (items_it == args->end()) {
      result->Error("INVALID_ARGS", "items list required");
      return;
    }

    const auto* items_list = std::get_if<flutter::EncodableList>(&items_it->second);
    if (!items_list) {
      result->Error("INVALID_ARGS", "items list required");
      return;
    }

    std::vector<TrayMenuItem> items;
    for (const auto& item_val : *items_list) {
      const auto* item_map = std::get_if<flutter::EncodableMap>(&item_val);
      if (!item_map) continue;

      TrayMenuItem item = {};

      auto id_it = item_map->find(flutter::EncodableValue("id"));
      if (id_it != item_map->end()) {
        item.id = std::get<int32_t>(id_it->second);
      }

      auto label_it = item_map->find(flutter::EncodableValue("label"));
      if (label_it != item_map->end()) {
        item.label = std::get<std::string>(label_it->second);
      }

      auto sep_it = item_map->find(flutter::EncodableValue("isSeparator"));
      if (sep_it != item_map->end()) {
        item.is_separator = std::get<bool>(sep_it->second);
      }

      auto disabled_it = item_map->find(flutter::EncodableValue("disabled"));
      if (disabled_it != item_map->end()) {
        item.disabled = std::get<bool>(disabled_it->second);
      }

      auto checked_it = item_map->find(flutter::EncodableValue("checked"));
      if (checked_it != item_map->end()) {
        item.checked = std::get<bool>(checked_it->second);
      }

      items.push_back(item);
    }

    SetMenu(items);
    result->Success(flutter::EncodableValue());
  } else if (method == "destroy") {
    DestroyTray();
    result->Success(flutter::EncodableValue());
  } else {
    result->NotImplemented();
  }
}

bool FlutterTrayPlugin::InitTray(const std::string& icon_path,
                                  const std::string& tooltip) {
  icon_path_ = icon_path;
  tooltip_ = tooltip;

  if (!hwnd_) {
    CreateHiddenWindow();
  }

  HICON hIcon = nullptr;
  if (!icon_path.empty()) {
    std::wstring wpath = Utf8ToWide(icon_path);
    hIcon = (HICON)LoadImageW(nullptr, wpath.c_str(), IMAGE_ICON,
                              GetSystemMetrics(SM_CXSMICON),
                              GetSystemMetrics(SM_CYSMICON),
                              LR_LOADFROMFILE | LR_SHARED);
  }

  if (!hIcon) {
    hIcon = LoadIconW(nullptr, IDI_APPLICATION);
  }

  if (nid_.hIcon) {
    DestroyIcon(nid_.hIcon);
  }

  nid_.hWnd = hwnd_;
  nid_.uID = 1;
  nid_.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
  nid_.uCallbackMessage = WM_TRAY_CALLBACK;
  nid_.hIcon = hIcon;

  std::wstring wtooltip = Utf8ToWide(tooltip);
  wcscpy_s(nid_.szTip, sizeof(nid_.szTip) / sizeof(WCHAR), wtooltip.c_str());

  if (tray_created_) {
    if (!Shell_NotifyIconW(NIM_MODIFY, &nid_)) {
      return false;
    }
  } else {
    if (!Shell_NotifyIconW(NIM_ADD, &nid_)) {
      if (nid_.hIcon) {
        DestroyIcon(nid_.hIcon);
        nid_.hIcon = nullptr;
      }
      return false;
    }
    tray_created_ = true;
  }
  return true;
}

void FlutterTrayPlugin::SetMenu(const std::vector<TrayMenuItem>& items) {
  menu_items_ = items;

  if (hmenu_) {
    DestroyMenu(hmenu_);
    hmenu_ = nullptr;
  }

  if (items.empty()) return;

  hmenu_ = CreatePopupMenu();

  int pos = 0;
  for (const auto& item : items) {
    if (item.is_separator) {
      InsertMenuW(hmenu_, pos, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
    } else {
      UINT flags = MF_BYPOSITION | MF_STRING;
      if (item.disabled) {
        flags |= MF_DISABLED | MF_GRAYED;
      }
      if (item.checked) {
        flags |= MF_CHECKED;
      }
      std::wstring wlabel = Utf8ToWide(item.label);
      InsertMenuW(hmenu_, pos, flags, item.id, wlabel.c_str());
    }
    pos++;
  }
}

void FlutterTrayPlugin::DestroyTray() {
  if (tray_created_) {
    Shell_NotifyIconW(NIM_DELETE, &nid_);
    tray_created_ = false;
  }
  if (nid_.hIcon) {
    DestroyIcon(nid_.hIcon);
    nid_.hIcon = nullptr;
  }
  if (hmenu_) {
    DestroyMenu(hmenu_);
    hmenu_ = nullptr;
  }
  menu_items_.clear();
}

void FlutterTrayPlugin::ShowContextMenu() {
  if (menu_items_.empty() || !hmenu_) {
    EmitEvent("rightClick", -1);
    return;
  }

  POINT pt;
  GetCursorPos(&pt);
  SetForegroundWindow(hwnd_);
  TrackPopupMenu(hmenu_, TPM_BOTTOMALIGN | TPM_LEFTALIGN,
                 pt.x, pt.y, 0, hwnd_, nullptr);
  PostMessageW(hwnd_, WM_NULL, 0, 0);
}

void FlutterTrayPlugin::EmitEvent(const std::string& type, int menu_id) {
  if (!channel_) return;

  flutter::EncodableMap args;
  args[flutter::EncodableValue("type")] = flutter::EncodableValue(type);
  args[flutter::EncodableValue("menuId")] = flutter::EncodableValue(menu_id);

  channel_->InvokeMethod("onTrayEvent",
                         std::make_unique<flutter::EncodableValue>(args));
}

void FlutterTrayPlugin::CreateHiddenWindow() {
  WNDCLASSW wc = {};
  wc.lpfnWndProc = WndProc;
  wc.hInstance = GetModuleHandleW(nullptr);
  wc.lpszClassName = L"FlutterTrayHiddenWindow";
  RegisterClassW(&wc);

  hwnd_ = CreateWindowExW(0, L"FlutterTrayHiddenWindow", L"",
                          WS_OVERLAPPED, 0, 0, 0, 0,
                          nullptr, nullptr, GetModuleHandleW(nullptr), this);
}

void FlutterTrayPlugin::DestroyHiddenWindow() {
  if (hwnd_) {
    DestroyWindow(hwnd_);
    hwnd_ = nullptr;
    UnregisterClassW(L"FlutterTrayHiddenWindow", GetModuleHandleW(nullptr));
  }
}

LRESULT CALLBACK FlutterTrayPlugin::WndProc(HWND hwnd, UINT msg,
                                            WPARAM wParam, LPARAM lParam) {
  FlutterTrayPlugin* plugin = nullptr;
  if (msg == WM_NCCREATE) {
    CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
    plugin = reinterpret_cast<FlutterTrayPlugin*>(cs->lpCreateParams);
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(plugin));
  } else {
    plugin = reinterpret_cast<FlutterTrayPlugin*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  }

  if (!plugin) {
    return DefWindowProcW(hwnd, msg, wParam, lParam);
  }

  if (msg == WM_TASKBAR_CREATED) {
    if (plugin->tray_created_) {
      Shell_NotifyIconW(NIM_ADD, &plugin->nid_);
    }
    return 0;
  }

  if (msg == WM_COMMAND) {
    int menu_id = LOWORD(wParam);
    plugin->EmitEvent("menuClick", menu_id);
    return 0;
  }

  if (msg == WM_TRAY_CALLBACK) {
    if (lParam == WM_LBUTTONUP) {
      plugin->EmitEvent("leftClick", -1);
    } else if (lParam == WM_RBUTTONUP) {
      plugin->ShowContextMenu();
    }
    return 0;
  }

  return DefWindowProcW(hwnd, msg, wParam, lParam);
}

}  // namespace flutter_tray
