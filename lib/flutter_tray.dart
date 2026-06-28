import 'flutter_tray_platform_interface.dart';
import 'models/menu_item.dart';
import 'models/tray_event.dart';

export 'models/menu_item.dart';
export 'models/tray_event.dart';

class FlutterTray {
  Future<bool> initTray({
    required String iconPath,
    required String tooltip,
  }) {
    return FlutterTrayPlatform.instance.initTray(
      iconPath: iconPath,
      tooltip: tooltip,
    );
  }

  Future<void> setMenu(List<MenuItem> items) {
    return FlutterTrayPlatform.instance.setMenu(items);
  }

  Stream<TrayEvent> get eventStream {
    return FlutterTrayPlatform.instance.eventStream;
  }

  Future<void> destroy() {
    return FlutterTrayPlatform.instance.destroy();
  }
}
