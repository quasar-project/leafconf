#pragma once

#include <string>
#include <leaf/global.h>

namespace leaf::serialization
{
  using std::string;
  using std::string_view;
  using namespace types;

  enum Serializer
  {
    TOML,
    JSON,
    YAML
  };

  class Serializable
  {
    public:
      virtual ~Serializable() = default;
      virtual auto serialize(Serializer) const -> expected<string, string> = 0;
  };

  class Deserializable
  {
    public:
      virtual ~Deserializable() = default;
      virtual auto deserialize(string_view, Serializer) -> expected<void, string> = 0;
  };

  template<typename T>
  concept SerializableType = std::is_base_of_v<Serializable, T>;

  template<typename T>
  concept DeserializableType = std::is_base_of_v<Deserializable, T>;

  template<typename T>
  concept SerdeType = SerializableType<T> and DeserializableType<T>;
}