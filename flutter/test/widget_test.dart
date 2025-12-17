import 'package:flutter_test/flutter_test.dart';

import 'package:bthome_scanner/main.dart';

void main() {
  testWidgets('App shows BTHome Devices title', (WidgetTester tester) async {
    await tester.pumpWidget(const BthomeScannerApp());
    expect(find.text('BTHome Devices'), findsOneWidget);
  });
}
