#pragma once
#include "Decl.h"

template <typename T>
struct TStdVector : std::false_type {
  using EleType = void;
};

template <typename E>
struct TStdVector<std::vector<E>> : std::true_type {
  using EleType = E;
};

template <typename T>
static constexpr bool IsStdVector = TStdVector<T>::value;

struct RVectorType : public RType {
  RVectorType(uint32 size) : RType("vector", size) {}

 public:
  virtual asUINT GetSize(void* container_ptr) const { return 0; }
  virtual bool IsEmpty(void* container_ptr) const { return true; }
  virtual void Reserve(void* container_ptr, asUINT maxElements) {}
  virtual void Resize(void* container_ptr, asUINT numElements) {}
  virtual void* At(void* container_ptr, asUINT index) { return nullptr; }
  virtual const void* At(void* container_ptr, asUINT index) const { return nullptr; }
  virtual void SetValue(void* container_ptr, asUINT index, void* value) {}
  virtual void InsertAt(void* container_ptr, asUINT index, void* value) {}
  // virtual void InsertAt(void* container_ptr, asUINT index, const CScriptArray& arr) {}
  virtual void InsertLast(void* container_ptr, void* value) {}
  virtual void RemoveAt(void* container_ptr, asUINT index) {}
  virtual void RemoveLast(void* container_ptr) {}
  virtual void RemoveRange(void* container_ptr, asUINT start, asUINT count) {}
  virtual void SortAsc(void* container_ptr) {}
  virtual void SortDesc(void* container_ptr) {}
  virtual void SortAsc(void* container_ptr, asUINT startAt, asUINT count) {}
  virtual void SortDesc(void* container_ptr, asUINT startAt, asUINT count) {}
  virtual void Sort(void* container_ptr, asUINT startAt, asUINT count, bool asc) {}
  virtual void Sort(void* container_ptr, asIScriptFunction* less, asUINT startAt, asUINT count) {}
  virtual void Reverse(void* container_ptr) {}
  virtual int Find(void* container_ptr, void* value) const { return -1; }
  virtual int Find(void* container_ptr, asUINT startAt, void* value) const { return -1; }
  virtual int FindByRef(void* container_ptr, void* ref) const { return -1; }
  virtual int FindByRef(void* container_ptr, asUINT startAt, void* ref) const { return -1; }
  virtual void* GetBuffer(void* container_ptr) { return nullptr; }

  RType* EleType{nullptr};
};

template <typename T>
struct TVectorType : public RVectorType {
  using VectorType = std::vector<T>;

 public:
  TVectorType() : RVectorType(sizeof(std::vector<T>)) {}

  asUINT GetSize(void* container_ptr) const { return reinterpret_cast<std::vector<T>*>(container_ptr)->size(); }
  bool IsEmpty(void* container_ptr) const { return reinterpret_cast<std::vector<T>*>(container_ptr)->empty(); }
  void Reserve(void* container_ptr, asUINT maxElements) {
    reinterpret_cast<std::vector<T>*>(container_ptr)->reserve(maxElements);
  }
  void Resize(void* container_ptr, asUINT numElements) {
    reinterpret_cast<std::vector<T>*>(container_ptr)->resize(numElements);
  }
  void* At(void* container_ptr, asUINT index) { return &(reinterpret_cast<std::vector<T>*>(container_ptr)->at(index)); }
  const void* At(void* container_ptr, asUINT index) const {
    return &(reinterpret_cast<std::vector<T>*>(container_ptr)->at(index));
  }
  void SetValue(void* container_ptr, asUINT index, void* value) {
    (*reinterpret_cast<std::vector<T>*>(container_ptr))[index] = (*reinterpret_cast<T*>(value));
  }
  void InsertAt(void* container_ptr, asUINT index, void* value) {
    std::vector<T>* vector_ptr = reinterpret_cast<std::vector<T>*>(container_ptr);
    vector_ptr->insert(vector_ptr->begin() + index, (*reinterpret_cast<T*>(value)));
  }
  // void InsertAt(void* container_ptr, asUINT index, const CScriptArray& arr);
  void InsertLast(void* container_ptr, void* value) {
    reinterpret_cast<std::vector<T>*>(container_ptr)->push_back(*reinterpret_cast<T*>(value));
  }
  void RemoveAt(void* container_ptr, asUINT index) {
    std::vector<T>* vector_ptr = reinterpret_cast<std::vector<T>*>(container_ptr);
    vector_ptr->erase(vector_ptr->begin() + index);
  }
  void RemoveLast(void* container_ptr) {
    std::vector<T>* vector_ptr = reinterpret_cast<std::vector<T>*>(container_ptr);
    vector_ptr->erase(vector_ptr->end() - 1);
  }
  void RemoveRange(void* container_ptr, asUINT start, asUINT count) {
    std::vector<T>* vector_ptr = reinterpret_cast<std::vector<T>*>(container_ptr);
    vector_ptr->erase(vector_ptr->begin() + start, vector_ptr->begin() + start + count);
  }
  void SortAsc(void* container_ptr) {
    std::vector<T>* vector_ptr = reinterpret_cast<std::vector<T>*>(container_ptr);
    std::sort(vector_ptr->begin(), vector_ptr->end(), std::less<T>());
  }
  void SortDesc(void* container_ptr) {
    std::vector<T>* vector_ptr = reinterpret_cast<std::vector<T>*>(container_ptr);
    std::sort(vector_ptr->begin(), vector_ptr->end(), std::greater<T>());
  }
  void SortAsc(void* container_ptr, asUINT startAt, asUINT count) { Sort(container_ptr, startAt, count, true); }
  void SortDesc(void* container_ptr, asUINT startAt, asUINT count) { Sort(container_ptr, startAt, count, false); }
  void Sort(void* container_ptr, asUINT startAt, asUINT count, bool asc) {
    std::vector<T>* vector_ptr = reinterpret_cast<std::vector<T>*>(container_ptr);
    if (asc) {
      std::sort(vector_ptr->begin() + startAt, vector_ptr->begin() + startAt + count, std::less<T>());
    } else {
      std::sort(vector_ptr->begin() + startAt, vector_ptr->begin() + startAt + count, std::greater<T>());
    }
  }
  void Reverse(void* container_ptr) {
    std::vector<T>* vector_ptr = reinterpret_cast<std::vector<T>*>(container_ptr);
    vector_ptr->reserve(vector_ptr->size());
  }
  int Find(void* container_ptr, void* value) const {
    std::vector<T>* vector_ptr = reinterpret_cast<std::vector<T>*>(container_ptr);
    auto it = std::find(vector_ptr->begin(), vector_ptr->end(), *reinterpret_cast<T*>(value));
    if (it != vector_ptr->end()) return it - vector_ptr->begin();
    return -1;
  }
  int Find(void* container_ptr, asUINT startAt, void* value) const {
    std::vector<T>* vector_ptr = reinterpret_cast<std::vector<T>*>(container_ptr);
    auto it = std::find(vector_ptr->begin() + startAt, vector_ptr->end(), *reinterpret_cast<T*>(value));
    if (it != vector_ptr->end()) return it - vector_ptr->begin();
    return -1;
  }
  int FindByRef(void* container_ptr, void* ref) const {
    std::vector<T>* vector_ptr = reinterpret_cast<std::vector<T>*>(container_ptr);
    auto it = std::find(vector_ptr->begin(), vector_ptr->end(), *reinterpret_cast<T*>(ref));
    if (it != vector_ptr->end()) return it - vector_ptr->begin();
    return -1;
  }
  int FindByRef(void* container_ptr, asUINT startAt, void* ref) const {
    std::vector<T>* vector_ptr = reinterpret_cast<std::vector<T>*>(container_ptr);
    auto it = std::find(vector_ptr->begin() + startAt, vector_ptr->end(), *reinterpret_cast<T*>(ref));
    if (it != vector_ptr->end()) return it - vector_ptr->begin();
    return -1;
  }
  void* GetBuffer(void* container_ptr) { return reinterpret_cast<std::vector<T>*>(container_ptr)->data(); }
};
