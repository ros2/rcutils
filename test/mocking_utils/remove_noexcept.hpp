// Copyright 2019 Michael "monkey0506" Rittenhouse
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

// This code originally comes from https://stackoverflow.com/a/55701361,
// and more specifically from the "live demo" at https://wandbox.org/permlink/GUqJFwMnczKEKlzX.
// That code is licensed under the WTFPL license, which allows basically anything.
// We relicense the code to Apache 2.0 for compatibility with the rest of our code,
// with Copyright to the original author, Michael "monkey0506" Rittenhouse.

#ifndef MOCKING_UTILS__REMOVE_NOEXCEPT_HPP_
#define MOCKING_UTILS__REMOVE_NOEXCEPT_HPP_

#include <type_traits>

template<typename T, bool noexcept_state = true>
struct make_noexcept { using type = T; };

// MSVC++ 2019 (v142) doesn't allow `noexcept(x)` with a template parameter `x` in the template
// specialization list.
// (e.g., `struct make_noexcept<R(Args...) noexcept(noexcept_state)>` gives
// - C2057: expected constant expression)
// GCC 7.1.0 and Clang 5.0.0 (and later versions) were tested and do allow this, so MSVC++ is
// probably wrong.
// $ g++ prog.cc -Wall -Wextra -std=c++17 -pedantic
// $ clang++ prog.cc -Wall -Wextra -std=c++17 -pedantic

// Additionally, MSVC++ 2019 (v142) discards `noexcept(noexcept)` state in the actual type
// declaration as well.
// (e.g., `using type = R(Args...) noexcept(noexcept_state);`
// always evaluates to `using type = R(Args...);`)
// GCC and Clang produce the correct results in these cases.

template<typename R, typename ... Args>
struct make_noexcept<R(Args...) noexcept, true>
{
  using type = R(Args ...) noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const noexcept, true>
{
  using type = R(Args ...) const noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) volatile noexcept, true>
{
  using type = R(Args ...) volatile noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const volatile noexcept, true>
{
  using type = R(Args ...) const volatile noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) noexcept, true>
{
  using type = R(Args ..., ...) noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const noexcept, true>
{
  using type = R(Args ..., ...) const noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) volatile noexcept, true>
{
  using type = R(Args ..., ...) volatile noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const volatile noexcept, true>
{
  using type = R(Args ..., ...) const volatile noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) & noexcept, true>
{
  using type = R(Args ... ) & noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const & noexcept, true>
{
  using type = R(Args ...) const & noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) volatile & noexcept, true>
{
  using type = R(Args ...) volatile & noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const volatile & noexcept, true>
{
  using type = R(Args ...) const volatile & noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) & noexcept, true>
{
  using type = R(Args ..., ... ) & noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const & noexcept, true>
{
  using type = R(Args ..., ...) const & noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) volatile & noexcept, true>
{
  using type = R(Args ..., ...) volatile & noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const volatile & noexcept, true>
{
  using type = R(Args ..., ...) const volatile & noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) && noexcept, true>
{
  using type = R(Args ...) && noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const && noexcept, true>
{
  using type = R(Args ...) const && noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) volatile && noexcept, true>
{
  using type = R(Args ...) volatile && noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const volatile && noexcept, true>
{
  using type = R(Args ...) const volatile && noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) && noexcept, true>
{
  using type = R(Args ..., ...) && noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const && noexcept, true>
{
  using type = R(Args ..., ...) const && noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) volatile && noexcept, true>
{
  using type = R(Args ..., ...) volatile && noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const volatile && noexcept, true>
{
  using type = R(Args ..., ...) const volatile && noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) noexcept, true>
{
  using type = R (C::*)(Args...) noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const noexcept, true>
{
  using type = R (C::*)(Args...) const noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) volatile noexcept, true>
{
  using type = R (C::*)(Args...) volatile noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const volatile noexcept, true>
{
  using type = R (C::*)(Args...) const volatile noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) noexcept, true>
{
  using type = R (C::*)(Args..., ...) noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const noexcept, true>
{
  using type = R (C::*)(Args...) const noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) volatile noexcept, true>
{
  using type = R (C::*)(Args...) volatile noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const volatile noexcept, true>
{
  using type = R (C::*)(Args...) const volatile noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) & noexcept, true>
{
  using type = R (C::*)(Args... ) & noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const & noexcept, true>
{
  using type = R (C::*)(Args...) const & noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) volatile & noexcept, true>
{
  using type = R (C::*)(Args...) volatile & noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const volatile & noexcept, true>
{
  using type = R (C::*)(Args...) const volatile & noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) & noexcept, true>
{
  using type = R (C::*)(Args..., ... ) & noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const & noexcept, true>
{
  using type = R (C::*)(Args..., ...) const & noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) volatile & noexcept, true>
{
  using type = R (C::*)(Args..., ...) volatile & noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const volatile & noexcept, true>
{
  using type = R (C::*)(Args..., ...) const volatile & noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) && noexcept, true>
{
  using type = R (C::*)(Args...) && noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const && noexcept, true>
{
  using type = R (C::*)(Args...) const && noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) volatile && noexcept, true>
{
  using type = R (C::*)(Args...) volatile && noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const volatile && noexcept, true>
{
  using type = R (C::*)(Args...) const volatile && noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) && noexcept, true>
{
  using type = R (C::*)(Args..., ...) && noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const && noexcept, true>
{
  using type = R (C::*)(Args..., ...) const && noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) volatile && noexcept, true>
{
  using type = R (C::*)(Args..., ...) volatile && noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const volatile && noexcept, true>
{
  using type = R (C::*)(Args..., ...) const volatile && noexcept;
};

template<typename R, typename ... Args>
struct make_noexcept<R(Args...), true>
{
  using type = R(Args ...) noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const, true>
{
  using type = R(Args ...) const noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) volatile, true>
{
  using type = R(Args ...) volatile noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const volatile, true>
{
  using type = R(Args ...) const volatile noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...), true>
{
  using type = R(Args ..., ...) noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const, true>
{
  using type = R(Args ..., ...) const noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) volatile, true>
{
  using type = R(Args ..., ...) volatile noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const volatile, true>
{
  using type = R(Args ..., ...) const volatile noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) &, true>
{
  using type = R(Args ... ) & noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const &, true>
{
  using type = R(Args ...) const & noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) volatile &, true>
{
  using type = R(Args ...) volatile & noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const volatile &, true>
{
  using type = R(Args ...) const volatile & noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) &, true>
{
  using type = R(Args ..., ... ) & noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const &, true>
{
  using type = R(Args ..., ...) const & noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) volatile &, true>
{
  using type = R(Args ..., ...) volatile & noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const volatile &, true>
{
  using type = R(Args ..., ...) const volatile & noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) &&, true>
{
  using type = R(Args ...) && noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const &&, true>
{
  using type = R(Args ...) const && noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) volatile &&, true>
{
  using type = R(Args ...) volatile && noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const volatile &&, true>
{
  using type = R(Args ...) const volatile && noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) &&, true>
{
  using type = R(Args ..., ...) && noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const &&, true>
{
  using type = R(Args ..., ...) const && noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) volatile &&, true>
{
  using type = R(Args ..., ...) volatile && noexcept;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const volatile &&, true>
{
  using type = R(Args ..., ...) const volatile && noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...), true>
{
  using type = R (C::*)(Args...) noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const, true>
{
  using type = R (C::*)(Args...) const noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) volatile, true>
{
  using type = R (C::*)(Args...) volatile noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const volatile, true>
{
  using type = R (C::*)(Args...) const volatile noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...), true>
{
  using type = R (C::*)(Args..., ...) noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const, true>
{
  using type = R (C::*)(Args...) const noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) volatile, true>
{
  using type = R (C::*)(Args...) volatile noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const volatile, true>
{
  using type = R (C::*)(Args...) const volatile noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) &, true>
{
  using type = R (C::*)(Args... ) & noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const &, true>
{
  using type = R (C::*)(Args...) const & noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) volatile &, true>
{
  using type = R (C::*)(Args...) volatile & noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const volatile &, true>
{
  using type = R (C::*)(Args...) const volatile & noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) &, true>
{
  using type = R (C::*)(Args..., ... ) & noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const &, true>
{
  using type = R (C::*)(Args..., ...) const & noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) volatile &, true>
{
  using type = R (C::*)(Args..., ...) volatile & noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const volatile &, true>
{
  using type = R (C::*)(Args..., ...) const volatile & noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) &&, true>
{
  using type = R (C::*)(Args...) && noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const &&, true>
{
  using type = R (C::*)(Args...) const && noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) volatile &&, true>
{
  using type = R (C::*)(Args...) volatile && noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const volatile &&, true>
{
  using type = R (C::*)(Args...) const volatile && noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) &&, true>
{
  using type = R (C::*)(Args..., ...) && noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const &&, true>
{
  using type = R (C::*)(Args..., ...) const && noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) volatile &&, true>
{
  using type = R (C::*)(Args..., ...) volatile && noexcept;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const volatile &&, true>
{
  using type = R (C::*)(Args..., ...) const volatile && noexcept;
};

template<typename R, typename ... Args>
struct make_noexcept<R(Args...) noexcept, false>
{
  using type = R(Args ...);
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const noexcept, false>
{
  using type = R(Args ...) const;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) volatile noexcept, false>
{
  using type = R(Args ...) volatile;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const volatile noexcept, false>
{
  using type = R(Args ...) const volatile;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) noexcept, false>
{
  using type = R(Args ..., ...);
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const noexcept, false>
{
  using type = R(Args ..., ...) const;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) volatile noexcept, false>
{
  using type = R(Args ..., ...) volatile;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const volatile noexcept, false>
{
  using type = R(Args ..., ...) const volatile;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) & noexcept, false>
{
  using type = R(Args ... ) &;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const & noexcept, false>
{
  using type = R(Args ...) const &;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) volatile & noexcept, false>
{
  using type = R(Args ...) volatile &;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const volatile & noexcept, false>
{
  using type = R(Args ...) const volatile &;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) & noexcept, false>
{
  using type = R(Args ..., ... ) &;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const & noexcept, false>
{
  using type = R(Args ..., ...) const &;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) volatile & noexcept, false>
{
  using type = R(Args ..., ...) volatile &;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const volatile & noexcept, false>
{
  using type = R(Args ..., ...) const volatile &;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) && noexcept, false>
{
  using type = R(Args ...) &&;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const && noexcept, false>
{
  using type = R(Args ...) const &&;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) volatile && noexcept, false>
{
  using type = R(Args ...) volatile &&;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const volatile && noexcept, false>
{
  using type = R(Args ...) const volatile &&;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) && noexcept, false>
{
  using type = R(Args ..., ...) &&;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const && noexcept, false>
{
  using type = R(Args ..., ...) const &&;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) volatile && noexcept, false>
{
  using type = R(Args ..., ...) volatile &&;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const volatile && noexcept, false>
{
  using type = R(Args ..., ...) const volatile &&;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) noexcept, false>
{
  using type = R (C::*)(Args...);
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const noexcept, false>
{
  using type = R (C::*)(Args...) const;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) volatile noexcept, false>
{
  using type = R (C::*)(Args...) volatile;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const volatile noexcept, false>
{
  using type = R (C::*)(Args...) const volatile;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) noexcept, false>
{
  using type = R (C::*)(Args..., ...);
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const noexcept, false>
{
  using type = R (C::*)(Args...) const;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) volatile noexcept, false>
{
  using type = R (C::*)(Args...) volatile;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const volatile noexcept, false>
{
  using type = R (C::*)(Args...) const volatile;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) & noexcept, false>
{
  using type = R (C::*)(Args... ) &;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const & noexcept, false>
{
  using type = R (C::*)(Args...) const &;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) volatile & noexcept, false>
{
  using type = R (C::*)(Args...) volatile &;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const volatile & noexcept, false>
{
  using type = R (C::*)(Args...) const volatile &;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) & noexcept, false>
{
  using type = R (C::*)(Args..., ... ) &;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const & noexcept, false>
{
  using type = R (C::*)(Args..., ...) const &;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) volatile & noexcept, false>
{
  using type = R (C::*)(Args..., ...) volatile &;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const volatile & noexcept, false>
{
  using type = R (C::*)(Args..., ...) const volatile &;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) && noexcept, false>
{
  using type = R (C::*)(Args...) &&;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const && noexcept, false>
{
  using type = R (C::*)(Args...) const &&;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) volatile && noexcept, false>
{
  using type = R (C::*)(Args...) volatile &&;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const volatile && noexcept, false>
{
  using type = R (C::*)(Args...) const volatile &&;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) && noexcept, false>
{
  using type = R (C::*)(Args..., ...) &&;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const && noexcept, false>
{
  using type = R (C::*)(Args..., ...) const &&;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) volatile && noexcept, false>
{
  using type = R (C::*)(Args..., ...) volatile &&;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const volatile && noexcept, false>
{
  using type = R (C::*)(Args..., ...) const volatile &&;
};

template<typename R, typename ... Args>
struct make_noexcept<R(Args...), false>
{
  using type = R(Args ...);
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const, false>
{
  using type = R(Args ...) const;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) volatile, false>
{
  using type = R(Args ...) volatile;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const volatile, false>
{
  using type = R(Args ...) const volatile;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...), false>
{
  using type = R(Args ..., ...);
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const, false>
{
  using type = R(Args ..., ...) const;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) volatile, false>
{
  using type = R(Args ..., ...) volatile;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const volatile, false>
{
  using type = R(Args ..., ...) const volatile;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) &, false>
{
  using type = R(Args ... ) &;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const &, false>
{
  using type = R(Args ...) const &;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) volatile &, false>
{
  using type = R(Args ...) volatile &;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const volatile &, false>
{
  using type = R(Args ...) const volatile &;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) &, false>
{
  using type = R(Args ..., ... ) &;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const &, false>
{
  using type = R(Args ..., ...) const &;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) volatile &, false>
{
  using type = R(Args ..., ...) volatile &;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const volatile &, false>
{
  using type = R(Args ..., ...) const volatile &;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) &&, false>
{
  using type = R(Args ...) &&;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const &&, false>
{
  using type = R(Args ...) const &&;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) volatile &&, false>
{
  using type = R(Args ...) volatile &&;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args...) const volatile &&, false>
{
  using type = R(Args ...) const volatile &&;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) &&, false>
{
  using type = R(Args ..., ...) &&;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const &&, false>
{
  using type = R(Args ..., ...) const &&;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) volatile &&, false>
{
  using type = R(Args ..., ...) volatile &&;
};
template<typename R, typename ... Args>
struct make_noexcept<R(Args..., ...) const volatile &&, false>
{
  using type = R(Args ..., ...) const volatile &&;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...), false>
{
  using type = R (C::*)(Args...);
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const, false>
{
  using type = R (C::*)(Args...) const;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) volatile, false>
{
  using type = R (C::*)(Args...) volatile;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const volatile, false>
{
  using type = R (C::*)(Args...) const volatile;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...), false>
{
  using type = R (C::*)(Args..., ...);
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const, false>
{
  using type = R (C::*)(Args...) const;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) volatile, false>
{
  using type = R (C::*)(Args...) volatile;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const volatile, false>
{
  using type = R (C::*)(Args...) const volatile;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) &, false>
{
  using type = R (C::*)(Args... ) &;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const &, false>
{
  using type = R (C::*)(Args...) const &;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) volatile &, false>
{
  using type = R (C::*)(Args...) volatile &;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const volatile &, false>
{
  using type = R (C::*)(Args...) const volatile &;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) &, false>
{
  using type = R (C::*)(Args..., ... ) &;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const &, false>
{
  using type = R (C::*)(Args..., ...) const &;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) volatile &, false>
{
  using type = R (C::*)(Args..., ...) volatile &;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const volatile &, false>
{
  using type = R (C::*)(Args..., ...) const volatile &;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) &&, false>
{
  using type = R (C::*)(Args...) &&;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const &&, false>
{
  using type = R (C::*)(Args...) const &&;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) volatile &&, false>
{
  using type = R (C::*)(Args...) volatile &&;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args...) const volatile &&, false>
{
  using type = R (C::*)(Args...) const volatile &&;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) &&, false>
{
  using type = R (C::*)(Args..., ...) &&;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const &&, false>
{
  using type = R (C::*)(Args..., ...) const &&;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) volatile &&, false>
{
  using type = R (C::*)(Args..., ...) volatile &&;
};
template<typename R, typename C, typename ... Args>
struct make_noexcept<R (C::*)(Args..., ...) const volatile &&, false>
{
  using type = R (C::*)(Args..., ...) const volatile &&;
};

template<typename T, bool noexcept_state = true>
using make_noexcept_t = typename make_noexcept<T, noexcept_state>::type;

template<typename T>
using remove_noexcept_t = make_noexcept_t<T, false>;

#endif  // MOCKING_UTILS__REMOVE_NOEXCEPT_HPP_
