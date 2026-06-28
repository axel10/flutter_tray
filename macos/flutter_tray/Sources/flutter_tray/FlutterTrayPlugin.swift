import Cocoa
import FlutterMacOS

public class FlutterTrayPlugin: NSObject, FlutterPlugin, NSMenuDelegate {
  private var channel: FlutterMethodChannel
  private var statusItem: NSStatusItem?
  private var menuItems: [[String: Any]] = []

  init(channel: FlutterMethodChannel) {
    self.channel = channel
    super.init()
  }

  public static func register(with registrar: FlutterPluginRegistrar) {
    let channel = FlutterMethodChannel(name: "flutter_tray", binaryMessenger: registrar.messenger)
    let instance = FlutterTrayPlugin(channel: channel)
    registrar.addMethodCallDelegate(instance, channel: channel)
  }

  public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
    switch call.method {
    case "initTray":
      guard let args = call.arguments as? [String: Any],
            let iconPath = args["iconPath"] as? String,
            let tooltip = args["tooltip"] as? String else {
        result(FlutterError(code: "INVALID_ARGS", message: "iconPath and tooltip required", details: nil))
        return
      }
      let ok = initTray(iconPath: iconPath, tooltip: tooltip)
      result(ok)

    case "setMenu":
      guard let args = call.arguments as? [String: Any],
            let items = args["items"] as? [[String: Any]] else {
        result(FlutterError(code: "INVALID_ARGS", message: "items list required", details: nil))
        return
      }
      setMenu(items: items)
      result(nil)

    case "destroy":
      destroy()
      result(nil)

    default:
      result(FlutterMethodNotImplemented)
    }
  }

  private func initTray(iconPath: String, tooltip: String) -> Bool {
    destroy()

    let image: NSImage?
    if iconPath.isEmpty {
      image = NSImage(size: NSSize(width: 18, height: 18))
    } else {
      image = NSImage(contentsOfFile: iconPath)
    }

    guard let img = image else {
      return false
    }

    statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)

    if let button = statusItem?.button {
      img.size = NSSize(width: 18, height: 18)
      img.isTemplate = true
      button.image = img
      button.toolTip = tooltip
      button.target = self
      button.action = #selector(onStatusItemLeftClick)
      button.sendAction(on: [.leftMouseUp, .rightMouseUp])
    }

    return true
  }

  private func setMenu(items: [[String: Any]]) {
    menuItems = items

    let menu = NSMenu()
    menu.delegate = self
    menu.autoenablesItems = false

    for itemDict in items {
      let isSeparator = itemDict["isSeparator"] as? Bool ?? false
      let label = itemDict["label"] as? String ?? ""
      let disabled = itemDict["disabled"] as? Bool ?? false
      let id = itemDict["id"] as? Int ?? 0

      if isSeparator {
        menu.addItem(NSMenuItem.separator())
      } else {
        let menuItem = NSMenuItem(title: label, action: #selector(onMenuItemClick(_:)), keyEquivalent: "")
        menuItem.tag = id
        menuItem.isEnabled = !disabled
        menuItem.target = self
        menu.addItem(menuItem)
      }
    }

    statusItem?.menu = menu
  }

  private func destroy() {
    if let item = statusItem {
      NSStatusBar.system.removeStatusItem(item)
      statusItem = nil
    }
    menuItems = []
  }

  @objc private func onStatusItemLeftClick() {
    guard let event = NSApp.currentEvent else { return }

    if event.type == .rightMouseUp {
      return
    }

    channel.invokeMethod("onTrayEvent", arguments: [
      "type": "leftClick",
      "menuId": -1,
    ])
  }

  @objc private func onMenuItemClick(_ sender: NSMenuItem) {
    channel.invokeMethod("onTrayEvent", arguments: [
      "type": "menuClick",
      "menuId": sender.tag,
    ])
  }

  public func menuWillOpen(_ menu: NSMenu) {
    channel.invokeMethod("onTrayEvent", arguments: [
      "type": "rightClick",
      "menuId": -1,
    ])
  }

  public func menuDidClose(_ menu: NSMenu) {
  }
}
