#include <flutter_linux/flutter_linux.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "include/flutter_tray/flutter_tray_plugin.h"
#include "flutter_tray_plugin_private.h"

namespace flutter_tray {
namespace test {

TEST(FlutterTrayPlugin, GetType) {
  GType type = flutter_tray_plugin_get_type();
  EXPECT_NE(type, G_TYPE_INVALID);
  EXPECT_TRUE(g_type_is_a(type, G_TYPE_OBJECT));
}

}  // namespace test
}  // namespace flutter_tray
