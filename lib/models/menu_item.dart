class MenuItem {
  final int id;
  final String label;
  final bool isSeparator;
  final bool disabled;
  final bool checked;

  const MenuItem({
    required this.id,
    required this.label,
    this.isSeparator = false,
    this.disabled = false,
    this.checked = false,
  });

  static MenuItem separator(int id) {
    return MenuItem(id: id, label: '', isSeparator: true);
  }

  Map<String, dynamic> toMap() {
    return {
      'id': id,
      'label': label,
      'isSeparator': isSeparator,
      'disabled': disabled,
      'checked': checked,
    };
  }

  factory MenuItem.fromMap(Map<String, dynamic> map) {
    return MenuItem(
      id: map['id'] as int,
      label: map['label'] as String? ?? '',
      isSeparator: map['isSeparator'] as bool? ?? false,
      disabled: map['disabled'] as bool? ?? false,
      checked: map['checked'] as bool? ?? false,
    );
  }
}
