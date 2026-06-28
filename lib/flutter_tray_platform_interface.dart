import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'flutter_tray_method_channel.dart';
import 'models/menu_item.dart';
import 'models/tray_event.dart';

abstract class FlutterTrayPlatform extends PlatformInterface {
  FlutterTrayPlatform() : super(token: _token);

  static final Object _token = Object();

  static FlutterTrayPlatform _instance = MethodChannelFlutterTray();

  static FlutterTrayPlatform get instance => _instance;

  static set instance(FlutterTrayPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<bool> initTray({
    required String iconPath,
    required String tooltip,
  }) {
    throw UnimplementedError('initTray() has not been implemented.');
  }

  Future<void> setMenu(List<MenuItem> items) {
    throw UnimplementedError('setMenu() has not been implemented.');
  }

  Stream<TrayEvent> get eventStream {
    throw UnimplementedError('eventStream has not been implemented.');
  }

  Future<void> destroy() {
    throw UnimplementedError('destroy() has not been implemented.');
  }
}
