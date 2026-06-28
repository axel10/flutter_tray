enum TrayEventType {
  leftClick,
  rightClick,
  menuClick,
}

class TrayEvent {
  final TrayEventType type;
  final int? menuId;

  const TrayEvent({
    required this.type,
    this.menuId,
  });

  factory TrayEvent.fromMap(Map<String, dynamic> map) {
    final typeStr = map['type'] as String;
    TrayEventType type;
    switch (typeStr) {
      case 'leftClick':
        type = TrayEventType.leftClick;
        break;
      case 'rightClick':
        type = TrayEventType.rightClick;
        break;
      case 'menuClick':
        type = TrayEventType.menuClick;
        break;
      default:
        type = TrayEventType.leftClick;
    }
    return TrayEvent(
      type: type,
      menuId: map['menuId'] as int?,
    );
  }
}
