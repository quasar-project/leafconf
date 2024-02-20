#include <string>
#include <sstream>
#include <gtest/gtest.h>
#include <leafconf/static.h>
#include <leafconf/serializers/toml.h>

using namespace std;
using namespace leaf::types;

struct TestConfigData final : public leaf::conf::AbstractConfigData
{
  virtual ~TestConfigData() override = default;
  [[nodiscard]] virtual auto serialize(leaf::serialization::Serializer) const -> expected<string, string> override;
  [[nodiscard]] virtual auto deserialize(string_view, leaf::serialization::Serializer) -> expected<void, string> override;

  u32 test = 0;
  struct IpAddress
  {
    string ip;
    u16 port;
    struct SockMode
    {
      bool tcp = true;
      bool udp = false;
    } sock_mode;
  } ip_address = { .ip = "127.0.0.1", .port = 25565 };
};

auto TestConfigData::serialize(const leaf::serialization::Serializer serializer) const -> expected<string, string>
{
  const auto out = toml::table {
    {"test", test},
    {"ip_address", toml::table{
      {"ip", ip_address.ip},
      {"port", ip_address.port},
      {"sock_mode", toml::table{
        {"tcp", ip_address.sock_mode.tcp},
        {"udp", ip_address.sock_mode.udp}
      }}
    }}
  };

  stringstream ss;
  switch(serializer)
  {
    case leaf::serialization::TOML: ss << out; break;
    case leaf::serialization::JSON: ss << toml::json_formatter(out); break;
    case leaf::serialization::YAML: ss << toml::yaml_formatter(out); break;
    default: leaf::unreachable();
  }
  return ss.str();
}

auto TestConfigData::deserialize(const string_view data, const leaf::serialization::Serializer serializer) -> expected<void, string>
{
  if(serializer != leaf::serialization::Serializer::TOML)
    return leaf::Err("unsupported deserialization format");
  toml::table in;
  try {
    in = toml::parse(data);
  } catch(const toml::parse_error& err) {
    return leaf::Err(err.what());
  }
  try
  {
    TestConfigData temp;
    temp.test = in["test"].value<u32>().value();
    temp.ip_address = {
      .ip = in["ip_address"]["ip"].value<string>().value(),
      .port = in["ip_address"]["port"].value<u16>().value(),
      .sock_mode = {
        .tcp = in["ip_address"]["sock_mode"]["tcp"].value<bool>().value(),
        .udp = in["ip_address"]["sock_mode"]["udp"].value<bool>().value()
      }
    };
    *this = temp;
  } catch(const bad_optional_access& err) {
    return leaf::Err("error when deserializing: {}", err.what());
  }
  return {};
}

TEST(StaticConfig, Basic)
{
  auto config = leaf::conf::Config<TestConfigData>(
    "test.toml",
    leaf::conf::fs::current_path(),
    leaf::conf::Config<TestConfigData>::SavingPolicy::Explicitly,
    TestConfigData()
  );

  ASSERT_EQ(config.values.test, 0);
  ASSERT_EQ(config.values.ip_address.ip, "127.0.0.1");
  ASSERT_EQ(config.values.ip_address.port, 25565);
  ASSERT_EQ(config.values.ip_address.sock_mode.tcp, true);
  ASSERT_EQ(config.values.ip_address.sock_mode.udp, false);
  config.revert_to_defaults();
  ASSERT_EQ(config.values.test, 0);
  ASSERT_EQ(config.values.ip_address.ip, "127.0.0.1");
  ASSERT_EQ(config.values.ip_address.port, 25565);
  ASSERT_EQ(config.values.ip_address.sock_mode.tcp, true);
  ASSERT_EQ(config.values.ip_address.sock_mode.udp, false);
  ASSERT_TRUE(config.save());
}

TEST(StaticConfig, Save)
{
  auto config = leaf::conf::Config<TestConfigData>(
    "test.toml",
    leaf::conf::fs::current_path(),
    leaf::conf::Config<TestConfigData>::SavingPolicy::Explicitly,
    TestConfigData()
  );

  ASSERT_EQ(config.values.test, 0);
  ASSERT_EQ(config.values.ip_address.ip, "127.0.0.1");
  ASSERT_EQ(config.values.ip_address.port, 25565);
  ASSERT_EQ(config.values.ip_address.sock_mode.tcp, true);
  ASSERT_EQ(config.values.ip_address.sock_mode.udp, false);
  config.values.ip_address = {
    .ip = "127.0.0.1",
    .port = 45555,
    .sock_mode = {
      .tcp = false,
      .udp = true
    }
  };
  const auto save = config.save();
  ASSERT_TRUE(save);
  ASSERT_EQ(config.values.test, 0);
  ASSERT_EQ(config.values.ip_address.ip, "127.0.0.1");
  ASSERT_EQ(config.values.ip_address.port, 45555);
  ASSERT_EQ(config.values.ip_address.sock_mode.tcp, false);
  ASSERT_EQ(config.values.ip_address.sock_mode.udp, true);
  const auto load = config.load();
  ASSERT_TRUE(load);
  ASSERT_EQ(config.values.test, 0);
  ASSERT_EQ(config.values.ip_address.ip, "127.0.0.1");
  ASSERT_EQ(config.values.ip_address.port, 45555);
  ASSERT_EQ(config.values.ip_address.sock_mode.tcp, false);
  ASSERT_EQ(config.values.ip_address.sock_mode.udp, true);
}

class Listener final : public leaf::pattern::IObserver<int>
{
  public:
    bool ok = false;
    virtual auto update(const int value) -> void override final {
      llog::info("new value: {}", value);
      this->ok = true;
    }
};

TEST(StaticConfig, Notifications)
{
  auto config = leaf::conf::Config<TestConfigData>(
    "test2.toml",
    leaf::conf::fs::current_path(),
    leaf::conf::Config<TestConfigData>::SavingPolicy::Explicitly,
    TestConfigData()
  );

  auto listener = new Listener();
  config.values.subscribe(listener);

  const auto load = config.load();
  ASSERT_TRUE(load);
  ASSERT_TRUE(listener->ok);
  delete listener;
}

auto main(int argc, char** argv) -> int
{
  ::testing::InitGoogleTest(&argc, argv);

  filesystem::remove(filesystem::current_path() / "test.toml");
  filesystem::remove(filesystem::current_path() / "test2.toml");

  std::ignore = leaf::log::LogFileConfiguration { "log", 1024 * 1024, 1 }
    .initialize();
  return ::RUN_ALL_TESTS();
}
