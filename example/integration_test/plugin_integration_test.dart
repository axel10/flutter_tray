import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

import 'package:flutter_tray/flutter_tray.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('initTray test', (WidgetTester tester) async {
    final FlutterTray plugin = FlutterTray();
    final bool ok = await plugin.initTray(
      iconPath: '',
      tooltip: 'Integration Test',
    );
    expect(ok, isA<bool>());
    await plugin.destroy();
  });
}
