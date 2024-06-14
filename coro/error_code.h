#pragma once

#include <system_error>

namespace underthere {
enum class errc {
  UnknownError = 1,
};

class error_category : public std::error_category {
 public:
  const char *name() const noexcept override;
  std::string message(int ev) const override;
};

inline auto error_category::name() const noexcept -> const char * {
  return "underthere";
}

inline auto error_category::message(int ev) const -> std::string {
  return "unknown error";
}

auto make_error_code(errc code) -> std::error_code;

inline auto make_error_code(errc code) -> std::error_code {
  return {static_cast<int>(code), error_category{}};
}
}