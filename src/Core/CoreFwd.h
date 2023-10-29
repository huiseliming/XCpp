#pragma once
// std
#include <any>
#include <atomic>
#include <bitset>
#include <cassert>
#include <chrono>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <shared_mutex>
#include <stack>
#include <string>
#include <thread>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <vector>
// boost
#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/typeof/typeof.hpp>
// fmt
#include <fmt/format.h>
// spdlog
#include <spdlog/common.h>
#if X_DEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#else
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#endif
#include <spdlog/spdlog.h>
// cxxopts
#include <cxxopts.hpp>

// sdl2
#include <SDL.h>
#include <SDL_vulkan.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define X_STRINGIFY(...) #__VA_ARGS__
#define X_COMBINE_IMPL(A, B) A##B  // helper macro
#define X_COMBINE(A, B) X_COMBINE_IMPL(A, B)
#define X_FORMAT(...) fmt::format(__VA_ARGS__)

#define X_THROW_RUNTIME_ERROR(Fmt, ...) throw std::runtime_error(fmt::format(Fmt, ##__VA_ARGS__));
#define X_RUNTIME_ASSERT(Expr, ...)                              \
  if (!(Expr)) [[unlikely]] {                                    \
    X_THROW_RUNTIME_ERROR(                                       \
        "RUNTIME ASSERTION FAILED!\n"                            \
        "FILE: {}\n"                                             \
        "LINE: {}\n"                                             \
        "EXPR: {}\n"                                             \
        "INFO: {}\n",                                            \
        __FILE__, __LINE__, #Expr, fmt::format("" __VA_ARGS__)); \
  }

#if X_DEBUG
#define X_ASSERT(Expr, ...) X_RUNTIME_ASSERT(Expr, __VA_ARGS__)
#else
#define X_ASSERT(Expr, ...) Expr
#endif

#define X_ENUM_FLAGS(Enum)                                                                                                     \
  FORCEINLINE Enum& operator|=(Enum& LHS, Enum RHS) {                                                                          \
    return LHS = static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(LHS) |                                            \
                                   static_cast<std::underlying_type_t<Enum>>(RHS));                                            \
  }                                                                                                                            \
  FORCEINLINE Enum& operator&=(Enum& LHS, Enum RHS) {                                                                          \
    return LHS = static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(LHS) &                                            \
                                   static_cast<std::underlying_type_t<Enum>>(RHS));                                            \
  }                                                                                                                            \
  FORCEINLINE Enum& operator^=(Enum& LHS, Enum RHS) {                                                                          \
    return LHS = static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(LHS) ^                                            \
                                   static_cast<std::underlying_type_t<Enum>>(RHS));                                            \
  }                                                                                                                            \
  FORCEINLINE constexpr Enum operator|(Enum LHS, Enum RHS) {                                                                   \
    return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(LHS) | static_cast<std::underlying_type_t<Enum>>(RHS)); \
  }                                                                                                                            \
  FORCEINLINE constexpr Enum operator&(Enum LHS, Enum RHS) {                                                                   \
    return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(LHS) & static_cast<std::underlying_type_t<Enum>>(RHS)); \
  }                                                                                                                            \
  FORCEINLINE constexpr Enum operator^(Enum LHS, Enum RHS) {                                                                   \
    return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(LHS) ^ static_cast<std::underlying_type_t<Enum>>(RHS)); \
  }                                                                                                                            \
  FORCEINLINE constexpr bool operator!(Enum E) {                                                                               \
    return !static_cast<std::underlying_type_t<Enum>>(E);                                                                      \
  }                                                                                                                            \
  FORCEINLINE constexpr Enum operator~(Enum E) {                                                                               \
    return static_cast<Enum>(~static_cast<std::underlying_type_t<Enum>>(E));                                                   \
  }

template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
  return static_cast<typename std::underlying_type<E>::type>(e);
}

typedef bool _Bool;
typedef uint32_t uint;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

// #ifdef __EXECUTE_CPP_KIT_HEADER_TOOL__
// #define RTYPE(...) \
//   class [[clang::annotate("@" #__VA_ARGS__)]] X_COMBINE(X_COMBINE(_, __COUNTER__), _) {};
// #define RENUM(...) [[clang::annotate("@" #__VA_ARGS__)]]
// #define RPROP(...) [[clang::annotate("@" #__VA_ARGS__)]]
// #define RFUNC(...) [[clang::annotate("@" #__VA_ARGS__)]]
// #else
// #define RTYPE(...)
// #define RENUM(...)
// #define RPROP(...)
// #define RFUNC(...)
// #endif

// template <typename T>
// struct TCodeContainer {};

namespace GVar {

XCORE_API extern const char* Version;
XCORE_API extern const char* BuildType;
XCORE_API extern bool IsDebugBuild;

}  // namespace GVar

namespace GUint8 {
XCORE_API extern uint8 Zero;
XCORE_API extern uint8 One;
}  // namespace GUint8
namespace GUint16 {
XCORE_API extern uint16 Zero;
XCORE_API extern uint16 One;
}  // namespace GUint16
namespace GUint32 {
XCORE_API extern uint32 Zero;
XCORE_API extern uint32 One;
}  // namespace GUint32
namespace GUint64 {
XCORE_API extern uint64 Zero;
XCORE_API extern uint64 One;
}  // namespace GUint64

namespace GInt8 {
XCORE_API extern int8 Zero;
XCORE_API extern int8 One;
}  // namespace GInt8
namespace GInt16 {
XCORE_API extern int16 Zero;
XCORE_API extern int16 One;
}  // namespace GInt16
namespace GInt32 {
XCORE_API extern int32 Zero;
XCORE_API extern int32 One;
}  // namespace GInt32
namespace GInt64 {
XCORE_API extern int64 Zero;
XCORE_API extern int64 One;
}  // namespace GInt64

namespace GFloat {
XCORE_API extern float Zero;
XCORE_API extern float One;
}  // namespace GFloat
namespace GDouble {
XCORE_API extern double Zero;
XCORE_API extern double One;
}  // namespace GDouble

namespace GString {
XCORE_API extern std::string Empty;
XCORE_API extern std::string Zero;
XCORE_API extern std::string One;
XCORE_API extern std::string True;
XCORE_API extern std::string False;
}  // namespace GString

namespace GBoolean {
XCORE_API extern bool True;
XCORE_API extern bool False;
}  // namespace GBoolean