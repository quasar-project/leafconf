#pragma once

#include <leafconf/serialization.h>

namespace leaf::conf
{
  class StaticCOnfigUnderlying : public serialization::Deserializable, public serialization::Serializable
  {
    int a;
    int b;
    struct Elevation
    {
      int g;
    } elevation = { .g = 0 };
  };

  template<typename T>
  class Config
  {
    public:
      Config() = default;
      void load();
      void save();

      T structure;
  };

  inline auto conf = Config<StaticCOnfigUnderlying>{};
  conf.load();
}