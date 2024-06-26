#pragma once

namespace underthere {

class NoCopy {
 public:
  NoCopy() = default;
  NoCopy(const NoCopy &) = delete;
  NoCopy &operator=(const NoCopy &) = delete;
  NoCopy(NoCopy &&) = default;
  NoCopy &operator=(NoCopy &&) = default;
  virtual ~NoCopy() = default;
};

} // underthere
