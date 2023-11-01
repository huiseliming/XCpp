#pragma once

template <typename T>
struct TCodeContainer {};

struct XCORE_API ICppTraits {
 public:
  void (*Construct)(void* A) = nullptr;
  void (*CopyConstruct)(void* A, void* B) = nullptr;
  void (*MoveConstruct)(void* A, void* B) = nullptr;
  void (*CopyAssign)(void* A, void* B) = nullptr;
  void (*MoveAssign)(void* A, void* B) = nullptr;
  void (*Destruct)(void* A) = nullptr;
  bool (*Compare)(void* A, void* B) = nullptr;
  void (*Hash)(void* A) = nullptr;
};

template <typename CppType>
struct TCppTraits : ICppTraits {
 public:
  TCppTraits() {
    if constexpr (std::is_default_constructible_v<CppType>) Construct = &CppConstruct;
    if constexpr (std::is_copy_constructible_v<CppType>) CopyConstruct = &CppCopyConstruct;
    if constexpr (std::is_move_constructible_v<CppType>) MoveConstruct = &CppMoveConstruct;
    if constexpr (std::is_copy_assignable_v<CppType>) CopyAssign = &CppCopyAssign;
    if constexpr (std::is_move_assignable_v<CppType>) MoveAssign = &CppMoveAssign;
    if constexpr (std::is_destructible_v<CppType>) Destruct = &CppDestruct;
    if constexpr (std::equality_comparable<CppType>) Compare = &CppCompare;
  }
  static void CppConstruct(void* A) { new (A) CppType(); }
  static void CppCopyConstruct(void* A, void* B) { new (A) CppType(*static_cast<CppType*>(B)); }
  static void CppMoveConstruct(void* A, void* B) { new (A) CppType(std::move(*static_cast<CppType*>(B))); }
  static void CppCopyAssign(void* A, void* B) { *static_cast<CppType*>(A) = *static_cast<CppType*>(B); }
  static void CppMoveAssign(void* A, void* B) { *static_cast<CppType*>(A) = std::move(*static_cast<CppType*>(B)); }
  static bool CppCompare(void* A, void* B) { return *static_cast<CppType*>(A) == *static_cast<CppType*>(B); }
  static void CppDestruct(void* A) { static_cast<CppType*>(A)->~CppType(); }
  static void CppHash(void* A) {}
};

enum class EDeclFlags : uint64 {
  NoneFlagBits = 0ULL,
  Static = 1ULL << 0,
  Object = 1ULL << 30,
  RefCntObject = 1ULL << 31,
};

X_ENUM_FLAGS(EDeclFlags);

struct RDecl;
struct RType;
struct REnum;
struct RStruct;
struct RClass;
struct RProperty;
struct RFunction;

class OObject;
class ORefCntObject;