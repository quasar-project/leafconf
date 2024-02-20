#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <leaf/utils/rtti.h>
#include <leaf/pattern/iobservable.h>
#include <leafconf/serialization.h>

namespace leaf::conf
{
  using std::string;
  using std::string_view;
  using std::ifstream;
  using std::ofstream;
  namespace fs = std::filesystem;

  struct AbstractConfigData : public serialization::Serializable,
                              public serialization::Deserializable,
                              public pattern::IObservable<int>
  {
    virtual ~AbstractConfigData() = default;
  };

  template<typename T>
  concept AbstractConfigDataType = std::is_base_of_v<AbstractConfigData, T>;

  template<AbstractConfigDataType T>
  class Config final
  {
    public:
      enum class SavingPolicy
      {
        SaveOnDestruction,
        Explicitly
      };

      T values;
      T defaults;

      Config(string_view filename, const fs::path& folder, SavingPolicy policy, T default_values);
      ~Config();

      [[nodiscard]] auto path() const -> const fs::path&;

      [[nodiscard]] auto load() -> expected<void, string>;
      [[nodiscard]] auto save() const -> expected<void, string>;
      auto revert_to_defaults() -> void;

    private:
      [[nodiscard]] auto read_from_file() const -> expected<string, string>;
      [[nodiscard]] auto write_to_file(string_view content) const -> expected<void, string>;

    private:
      SavingPolicy m_saving_policy;
      fs::path m_path;
  };

  template<AbstractConfigDataType T>
  Config<T>::Config(const string_view filename, const fs::path& folder, const SavingPolicy policy, T default_values)
    : defaults(default_values),
      m_saving_policy(policy),
      m_path(folder / filename)
  {
    this->load()
      .map_error([this](const auto&& e) {
        llog::error("failed to load config file from {}, reason: {}", this->path().string(), e);
      });
  }

  template <AbstractConfigDataType T>
  Config<T>::~Config()
  {
    if(this->m_saving_policy == SavingPolicy::SaveOnDestruction)
      this->save()
        .map_error([](const auto&& e) { llog::error("failed to save config file: {}", e); });
  }

  template<AbstractConfigDataType T>
  auto Config<T>::path() const -> const fs::path& { return this->m_path; }

  template<AbstractConfigDataType T>
  auto Config<T>::load() -> expected<void, string>
  {
    if(not fs::exists(this->path()))
    {
      llog::debug("config: file does not exist, using defaults");
      this->revert_to_defaults();
      return {};
    }

    const auto content = this->read_from_file();
    if(not content)
      return Err(content.error());
    this->values.deserialize(*content, serialization::Serializer::TOML); // todo: transient serializer
    llog::debug("config: loaded from file");
    this->values.template notify(0);

    return {};
  }

  template<AbstractConfigDataType T>
  auto Config<T>::save() const -> expected<void, string>
  {
    llog::debug("config: saving to file ({})", this->path().string());
    return this->values.serialize(serialization::Serializer::TOML)
      .and_then([this](const auto&& content) { return this->write_to_file(content); });
  }

  template<AbstractConfigDataType T>
  auto Config<T>::revert_to_defaults() -> void
  {
    this->values = this->defaults;
    llog::debug("config: reverted to defaults");
    this->values.template notify(0);
    this->save()
      .map_error([](const auto&& e) { llog::error("failed to save config file when reverting to defaults: {}", e); });
  }

  template<AbstractConfigDataType T>
  auto Config<T>::read_from_file() const -> expected<string, string>
  {
    ifstream handle(this->path());
    if(not handle.is_open())
      return Err("failed to open file handle for reading at {}", this->path().string());

    string content((std::istreambuf_iterator(handle)), std::istreambuf_iterator<char>());
    llog::trace("config: read {} bytes from file");
    return content;
  }

  template<AbstractConfigDataType T>
  auto Config<T>::write_to_file(const string_view content) const -> expected<void, string>
  {
    ofstream handle(this->path());
    if(not handle.is_open())
      return Err("failed to open file handle for writing at {}", this->path().string());

    handle << content;
    if(not handle.good())
      return Err("failed to write to file at {}", this->path().string());
    llog::trace("config: wrote {} bytes to file");
    return {};
  }
}
