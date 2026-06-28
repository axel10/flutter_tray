import 'dart:async';
import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter_tray/flutter_tray.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final FlutterTray _tray = FlutterTray();
  String _lastEvent = 'No events yet';
  StreamSubscription<TrayEvent>? _subscription;

  @override
  void initState() {
    super.initState();
    _initTray();
  }

  Future<void> _initTray() async {
    String iconPath = '';

    if (Platform.isMacOS) {
      iconPath = 'AppIcon';
    } else {
      iconPath = '';
    }

    final ok = await _tray.initTray(
      iconPath: iconPath,
      tooltip: 'Flutter Tray Example',
    );

    if (ok) {
      await _tray.setMenu([
        const MenuItem(id: 1, label: 'Show Window'),
        MenuItem.separator(2),
        const MenuItem(id: 3, label: 'Quit'),
      ]);
    }

    _subscription = _tray.eventStream.listen((event) {
      setState(() {
        switch (event.type) {
          case TrayEventType.leftClick:
            _lastEvent = 'Left click';
            break;
          case TrayEventType.rightClick:
            _lastEvent = 'Right click';
            break;
          case TrayEventType.menuClick:
            _lastEvent = 'Menu click: id=${event.menuId}';
            if (event.menuId == 3) {
              exit(0);
            }
            break;
        }
      });
    });
  }

  @override
  void dispose() {
    _subscription?.cancel();
    _tray.destroy();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('Flutter Tray Example')),
        body: Center(child: Text('Last event: $_lastEvent')),
      ),
    );
  }
}
