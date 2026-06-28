import 'package:flutter_test/flutter_test.dart';
import 'package:flutter_tray/flutter_tray.dart';
import 'package:flutter_tray/flutter_tray_platform_interface.dart';
import 'package:flutter_tray/flutter_tray_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockFlutterTrayPlatform
    with MockPlatformInterfaceMixin
    implements FlutterTrayPlatform {
  @override
  Future<bool> initTray({
    required String iconPath,
    required String tooltip,
  }) {
    return Future.value(true);
  }

  @override
  Future<void> setMenu(List<MenuItem> items) => Future.value();

  @override
  Stream<TrayEvent> get eventStream => const Stream.empty();

  @override
  Future<void> destroy() => Future.value();
}

void main() {
  final FlutterTrayPlatform initialPlatform = FlutterTrayPlatform.instance;

  test('$MethodChannelFlutterTray is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelFlutterTray>());
  });

  test('initTray', () async {
    FlutterTray flutterTrayPlugin = FlutterTray();
    MockFlutterTrayPlatform fakePlatform = MockFlutterTrayPlatform();
    FlutterTrayPlatform.instance = fakePlatform;

    expect(
      await flutterTrayPlugin.initTray(iconPath: '/tmp/icon.png', tooltip: 'Test'),
      true,
    );
  });

  test('setMenu', () async {
    FlutterTray flutterTrayPlugin = FlutterTray();
    MockFlutterTrayPlatform fakePlatform = MockFlutterTrayPlatform();
    FlutterTrayPlatform.instance = fakePlatform;

    await flutterTrayPlugin.setMenu([
      MenuItem(id: 1, label: 'Item 1'),
      MenuItem.separator(2),
      MenuItem(id: 3, label: 'Quit'),
    ]);
  });
}
