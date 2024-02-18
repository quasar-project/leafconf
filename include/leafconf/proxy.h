#pragma once

#include <functional>
#include <leaf/math.h>

namespace leaf::conf
{
  using std::function;

  template<typename T>
  class Proxy
  {
    public:
      explicit Proxy(T val, function<void (const T&)> on_change);
      Proxy(const Proxy& other) = delete;
      Proxy(const Proxy&& other) = delete;

      [[nodiscard]] auto get() const -> const T&;
      [[nodiscard]] auto operator*() const -> const T&;

      auto set(const T& val) -> void;

      auto operator=(const Proxy& other) -> Proxy&;
      auto operator=(Proxy&& other) noexcept -> Proxy&;

    private:
      T val;
      function<void (const T&)> changed;
  };

  template<typename T>
  Proxy<T>::Proxy(T val, function<void(const T &)> on_change)
    : val(val),
      changed(on_change)
  {}

  template<typename T>
  auto Proxy<T>::get() const -> const T & { return this->val; }

  template<typename T>
  auto Proxy<T>::operator*() const -> const T & { return this->get(); }

  template <typename T>
  auto Proxy<T>::set(const T& val) -> void
  {
    if(eq(val, this->val))
      return;
    this->val = val;
    if(this->changed)
      this->changed(this->val);
  }

  template<typename T>
  auto Proxy<T>::operator=(const Proxy& other) -> Proxy&
  {
    this->set(other.get());
    return *this;
  }

  template<typename T>
  auto Proxy<T>::operator=(Proxy&& other) noexcept -> Proxy&
  {
    this->set(other.get());
    return *this;
  }
}
