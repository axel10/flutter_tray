import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'flutter_tray_platform_interface.dart';
import 'models/menu_item.dart';
import 'models/tray_event.dart';

class MethodChannelFlutterTray extends FlutterTrayPlatform {
  @visibleForTesting
  final methodChannel = const MethodChannel('flutter_tray');

  StreamController<TrayEvent>? _eventController;

  @override
  Stream<TrayEvent> get eventStream {
    _eventController ??= _createEventStream();
    return _eventController!.stream;
  }

  StreamController<TrayEvent> _createEventStream() {
    final controller = StreamController<TrayEvent>.broadcast();

    methodChannel.setMethodCallHandler((MethodCall call) async {
      if (call.method == 'onTrayEvent') {
        final args = call.arguments as Map<dynamic, dynamic>?;
        if (args != null) {
          final event = TrayEvent.fromMap(args.cast<String, dynamic>());
          controller.add(event);
        }
      }
    });

    return controller;
  }

  @override
  Future<bool> initTray({
    required String iconPath,
    required String tooltip,
  }) async {
    final result = await methodChannel.invokeMethod<bool>('initTray', {
      'iconPath': iconPath,
      'tooltip': tooltip,
    });
    return result ?? false;
  }

  @override
  Future<void> setMenu(List<MenuItem> items) async {
    await methodChannel.invokeMethod('setMenu', {
      'items': items.map((e) => e.toMap()).toList(),
    });
  }

  @override
  Future<void> destroy() async {
    await methodChannel.invokeMethod('destroy');
  }
}
