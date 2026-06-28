import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:flutter_tray/flutter_tray_method_channel.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  final platform = MethodChannelFlutterTray();
  const MethodChannel channel = MethodChannel('flutter_tray');

  setUp(() {
    TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger
        .setMockMethodCallHandler(channel, (MethodCall methodCall) async {
      if (methodCall.method == 'initTray') {
        return true;
      }
      if (methodCall.method == 'setMenu') {
        return null;
      }
      if (methodCall.method == 'destroy') {
        return null;
      }
      return null;
    });
  });

  tearDown(() {
    TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger
        .setMockMethodCallHandler(channel, null);
  });

  test('initTray', () async {
    final result = await platform.initTray(
      iconPath: '/tmp/icon.png',
      tooltip: 'Test App',
    );
    expect(result, true);
  });

  test('setMenu', () async {
    await platform.setMenu([]);
  });

  test('destroy', () async {
    await platform.destroy();
  });
}
