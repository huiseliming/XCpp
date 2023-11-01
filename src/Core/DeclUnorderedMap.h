#pragma once
#include "Decl.h"

template <typename T>
struct TStdUnorderedMap : std::false_type {
  using KeyType = void;
  using ValType = void;
};

template <typename K, typename V>
struct TStdUnorderedMap<std::unordered_map<K, V>> : std::true_type {
  using KeyType = K;
  using ValType = V;
};

template <typename T>
static constexpr bool IsStdUnorderedMap = TStdUnorderedMap<T>::value;

struct RUnorderedMapType : public RType {
  RUnorderedMapType(uint32 size) : RType("unordered_map", size) {}

  virtual void Set(void* container_ptr, const void* key, void* value, int typeId) {}
  virtual bool Get(void* container_ptr, const void* key, void* value, int typeId) const { return false; }
  virtual bool Exists(void* container_ptr, const void* key) const { return false; }
  virtual bool IsEmpty(void* container_ptr) const { return true; }
  virtual asUINT GetSize(void* container_ptr) const { return 0; }
  virtual bool Delete(void* container_ptr, const void* key) { return false; }
  virtual void DeleteAll(void* container_ptr) {}
  virtual CScriptArray* GetKeys(void* container_ptr) const { return nullptr; }

  RType* KeyType{nullptr};
  RType* ValType{nullptr};
};

template <typename K, typename V>
struct TUnorderedMapType : public RUnorderedMapType {
  using UnorderedMapType = std::unordered_map<K, V>;

 public:
  TUnorderedMapType() : RUnorderedMapType(sizeof(std::unordered_map<K, V>)) {}

  virtual void Set(void* container_ptr, const void* key, void* value, int typeId) {
    std::unordered_map<K, V>* unordered_map_ptr = reinterpret_cast<std::unordered_map<K, V>*>(container_ptr);
    auto it = unordered_map_ptr->find(*reinterpret_cast<const K*>(key));
    unordered_map_ptr->insert_or_assign(*reinterpret_cast<const K*>(key), *reinterpret_cast<const V*>(value));
  }
  virtual bool Get(void* container_ptr, const void* key, void* value, int typeId) const {
    std::unordered_map<K, V>* unordered_map_ptr = reinterpret_cast<std::unordered_map<K, V>*>(container_ptr);
    auto it = unordered_map_ptr->find(*reinterpret_cast<const K*>(key));
    if (it != unordered_map_ptr->end()) {
      *reinterpret_cast<V*>(value) = it->second;
      return true;
    }
    return false;
  }
  virtual bool Exists(void* container_ptr, const void* key) const {
    std::unordered_map<K, V>* unordered_map_ptr = reinterpret_cast<std::unordered_map<K, V>*>(container_ptr);
    auto it = unordered_map_ptr->find(*reinterpret_cast<const K*>(key));
    return it != unordered_map_ptr->end();
  }
  virtual bool IsEmpty(void* container_ptr) const {
    return reinterpret_cast<std::unordered_map<K, V>*>(container_ptr)->empty();
  }
  virtual asUINT GetSize(void* container_ptr) const {
    return reinterpret_cast<std::unordered_map<K, V>*>(container_ptr)->size();
  }
  virtual bool Delete(void* container_ptr, const void* key) {
    return reinterpret_cast<std::unordered_map<K, V>*>(container_ptr)->erase(*reinterpret_cast<const K*>(key)) > 0;
  }
  virtual void DeleteAll(void* container_ptr) { reinterpret_cast<std::unordered_map<K, V>*>(container_ptr)->clear(); }
  virtual CScriptArray* GetKeys(void* container_ptr) const { return nullptr; }
};
