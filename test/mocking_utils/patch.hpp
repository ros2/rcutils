// Copyright 2020 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef MOCKING_UTILS__PATCH_HPP_
#define MOCKING_UTILS__PATCH_HPP_

#include <functional>
#include <string>
#include <type_traits>
#include <utility>

#include "mimick/mimick.h"
#include "rcutils/macros.h"

namespace mocking_utils
{

template<size_t N, typename SignatureT>
struct PatchTraits;

template<size_t N, typename ReturnType>
struct PatchTraits<N, ReturnType(void)>
{
  mmk_mock_define(mock_type, ReturnType);
};

template<size_t N, typename ReturnType, typename ArgType0>
struct PatchTraits<N, ReturnType(ArgType0)>
{
  mmk_mock_define(mock_type, ReturnType, ArgType0);
};

template<size_t N, typename ReturnType,
  typename ArgType0, typename ArgType1>
struct PatchTraits<N, ReturnType(ArgType0, ArgType1)>
{
  mmk_mock_define(mock_type, ReturnType, ArgType0, ArgType1);
};

template<size_t N, typename ReturnType,
  typename ArgType0, typename ArgType1, typename ArgType2>
struct PatchTraits<N, ReturnType(ArgType0, ArgType1, ArgType2)>
{
  mmk_mock_define(mock_type, ReturnType, ArgType0, ArgType1, ArgType2);
};

template<size_t N, typename ReturnType,
  typename ArgType0, typename ArgType1,
  typename ArgType2, typename ArgType3>
struct PatchTraits<N, ReturnType(ArgType0, ArgType1, ArgType2, ArgType3)>
{
  mmk_mock_define(mock_type, ReturnType, ArgType0, ArgType1, ArgType2, ArgType3);
};

template<size_t N, typename ReturnType,
  typename ArgType0, typename ArgType1,
  typename ArgType2, typename ArgType3, typename ArgType4>
struct PatchTraits<N, ReturnType(ArgType0, ArgType1, ArgType2, ArgType3, ArgType4)>
{
  mmk_mock_define(mock_type, ReturnType, ArgType0, ArgType1, ArgType2, ArgType3, ArgType4);
};

template<size_t N, typename ReturnType,
  typename ArgType0, typename ArgType1,
  typename ArgType2, typename ArgType3,
  typename ArgType4, typename ArgType5>
struct PatchTraits<N, ReturnType(ArgType0, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5)>
{
  mmk_mock_define(
    mock_type, ReturnType, ArgType0, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5);
};

template<size_t N, typename SignatureT>
struct Trampoline;

template<size_t N, typename ReturnT, typename ... ArgTs>
struct Trampoline<N, ReturnT(ArgTs...)>
{
  static ReturnT base(ArgTs... args)
  {
    return target(std::forward<ArgTs>(args)...);
  }

  static std::function<ReturnT(ArgTs...)> target;
};

template<size_t N, typename ReturnT, typename ... ArgTs>
std::function<ReturnT(ArgTs...)>
Trampoline<N, ReturnT(ArgTs...)>::target;

template<size_t N, typename SignatureT>
auto prepare_trampoline(std::function<SignatureT> target)
{
  Trampoline<N, SignatureT>::target = target;
  return Trampoline<N, SignatureT>::base;
}

template<size_t N, typename SignatureT>
class Patch;

template<size_t N, typename ReturnT, typename ... ArgTs>
class Patch<N, ReturnT(ArgTs...)>
{
public:
  using mock_type = typename PatchTraits<N, ReturnT(ArgTs...)>::mock_type;

  Patch(const std::string & target, std::function<ReturnT(ArgTs...)> proxy)
  : proxy_(proxy)
  {
    auto MMK_MANGLE(mock_type, create) =
      PatchTraits<N, ReturnT(ArgTs...)>::MMK_MANGLE(mock_type, create);
    mock_ = mmk_mock(target.c_str(), mock_type);
  }

  Patch(const Patch &) = delete;
  Patch & operator=(const Patch &) = delete;

  Patch(Patch && other)
  {
    mock_ = other.mock_;
    other.mock_ = nullptr;
  }

  Patch & operator=(Patch && other)
  {
    if (mock_) {
      mmk_reset(mock_);
    }
    mock_ = other.mock_;
    other.mock_ = nullptr;
  }

  ~Patch()
  {
    if (mock_) {
      mmk_reset(mock_);
    }
  }

  Patch & then_call(std::function<ReturnT(ArgTs...)> replacement) &
  {
    auto type_erased_trampoline =
      reinterpret_cast<mmk_fn>(prepare_trampoline<N>(replacement));
    mmk_when(proxy_(any<ArgTs>()...), .then_call = type_erased_trampoline);
    return *this;
  }

  Patch && then_call(std::function<ReturnT(ArgTs...)> replacement) &&
  {
    auto type_erased_trampoline =
      reinterpret_cast<mmk_fn>(prepare_trampoline<N>(replacement));
    mmk_when(proxy_(any<ArgTs>()...), .then_call = type_erased_trampoline);
    return std::move(*this);
  }

private:
  template<typename T>
  T any() {return mmk_any(T);}

  mock_type mock_;
  std::function<ReturnT(ArgTs...)> proxy_;
};

template<size_t N, typename SignatureT>
auto make_patch(const std::string & target, std::function<SignatureT> proxy)
{
  return Patch<N, SignatureT>(target, proxy);
}

/// Define a dummy operator `op` for a given `type`.
///
/// Useful to enable patching functions that take arguments whose types
/// do not define basic comparison operators required by Mimick.
#define MOCKING_UTILS_DEFINE_DUMMY_OPERATOR(type, op) \
  bool operator op(const type &, const type &) { \
    return false; \
  }

/// Get the exact mocking_utils::Patch type for a given id and function.
///
/// Useful to avoid ignored attribute warnings when using the \b decltype operator.
#define MOCKING_UTILS_PATCH_TYPE(id, function) \
  decltype(mocking_utils::make_patch<id, decltype(function)>("", nullptr))

/// A transparent forwarding proxy to a given `function`.
///
/// Useful to ensure a call to `function` goes through its trampoline.
#define MOCKING_UTILS_PATCH_PROXY(function) \
  [] (auto && ... args)->decltype(auto) { \
    return function(std::forward<decltype(args)>(args)...); \
  }

/// Compute a Mimick-compliant, symbol target string based on `what` binary
/// object and `which` symbol should be targeted.
#define MOCKING_UTILS_PATCH_TARGET(what, which) \
  (std::string(RCUTILS_STRINGIFY(where)) + "@" + (what))

/// Patch a function `with` a used-provided replacement, based on `what` binary
/// object and `which` symbol should be patched.
#define patch(what, which, with) \
  make_patch<__COUNTER__, decltype(which)>( \
    MOCKING_UTILS_PATCH_TARGET(what, which), MOCKING_UTILS_PATCH_PROXY(where) \
  ).then_call(with)

}  // namespace mocking_utils

#endif  // MOCKING_UTILS__PATCH_HPP_
