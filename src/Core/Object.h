#pragma once
#include "DeclFwd.h"

class CRefCntController;

enum EObjectFlags : uint32 {
  NoneFlagBits,
};

RTYPE()
class XCORE_API OObject {
 public:
  OObject() {}
  OObject(asIScriptObject* script_object) : ScriptObject(script_object) {
    ScriptObjectWeakRCFlag = ScriptObject->GetWeakRefFlag();
    ScriptObjectWeakRCFlag->AddRef();
  }
  virtual ~OObject() {
    if (ScriptObjectWeakRCFlag) {
      ScriptObjectWeakRCFlag->Release();
      ScriptObjectWeakRCFlag = nullptr;
    }
  }

  RClass* Class{nullptr};
  OObject* Owner{nullptr};

 protected:
  asIScriptObject* ScriptObject{nullptr};
  asILockableSharedBool* ScriptObjectWeakRCFlag{nullptr};
};

RTYPE()
class ORefCntObject : public OObject {
 public:
  ORefCntObject();

  FORCEINLINE void AddSharedReference();
  FORCEINLINE void ReleaseSharedReference();

 protected:
  CRefCntController* RefCntController;

 private:
  friend class CRefCntPtr;
};

class CRefCntController {
 protected:
  std::atomic<int32_t> SharedReferenceCount{1};
  std::atomic<int32_t> WeakReferenceCount{1};
  ORefCntObject* RefCntObject;

 public:
  FORCEINLINE explicit CRefCntController(ORefCntObject* rc_object);
  FORCEINLINE ~CRefCntController();
  FORCEINLINE int32_t GetSharedReferenceCount();
  FORCEINLINE int32_t GetWeakReferenceCount();
  FORCEINLINE void AddSharedReference();
  FORCEINLINE bool ConditionallyAddSharedReference();
  FORCEINLINE void ReleaseSharedReference();
  FORCEINLINE void AddWeakReference();
  FORCEINLINE void ReleaseWeakReference();
};

class CWeakRCReferencer;
class CSharedReferencer;
class CRefCntPtr;
class CWeakRCPtr;

class CSharedReferencer {
 protected:
  FORCEINLINE explicit CSharedReferencer(CRefCntController* rc_controller);

 public:
  FORCEINLINE CSharedReferencer();
  FORCEINLINE CSharedReferencer(CSharedReferencer const& shared_referencer);
  FORCEINLINE CSharedReferencer(CSharedReferencer&& shared_referencer);
  FORCEINLINE CSharedReferencer(CWeakRCReferencer const& weakrc_referencer);
  FORCEINLINE CSharedReferencer(CWeakRCReferencer&& weakrc_referencer);
  FORCEINLINE ~CSharedReferencer();
  FORCEINLINE CSharedReferencer& operator=(CSharedReferencer const& shared_referencer);
  FORCEINLINE CSharedReferencer& operator=(CSharedReferencer&& shared_referencer);
  FORCEINLINE const bool IsValid() const;
  FORCEINLINE const int32_t GetSharedReferenceCount() const;

 protected:
  CRefCntController* RefCntController;

 private:
  friend class CWeakRCReferencer;
  friend class CRefCntPtr;
  friend class CWeakRCPtr;
};

class CWeakRCReferencer {
 public:
  FORCEINLINE CWeakRCReferencer();
  FORCEINLINE CWeakRCReferencer(CWeakRCReferencer const& weakrc_referencer);
  FORCEINLINE CWeakRCReferencer(CWeakRCReferencer&& weakrc_referencer);
  FORCEINLINE CWeakRCReferencer(CSharedReferencer const& shared_referencer);
  FORCEINLINE ~CWeakRCReferencer();
  FORCEINLINE CWeakRCReferencer& operator=(CWeakRCReferencer const& weakrc_referencer);
  FORCEINLINE CWeakRCReferencer& operator=(CWeakRCReferencer&& weakrc_referencer);
  FORCEINLINE CWeakRCReferencer& operator=(CSharedReferencer const& shared_referencer);
  FORCEINLINE const bool IsValid() const;

 protected:
  CRefCntController* RefCntController;

 private:
  friend class CSharedReferencer;
  friend class CRefCntPtr;
  friend class CWeakRCPtr;
};

class CRefCntPtr {
 protected:
  FORCEINLINE CRefCntPtr(ORefCntObject* rc_object);

 public:
  FORCEINLINE CRefCntPtr();
  FORCEINLINE CRefCntPtr(CRefCntPtr const& shared_ptr);
  FORCEINLINE CRefCntPtr(CRefCntPtr&& shared_ptr);
  FORCEINLINE CRefCntPtr(CWeakRCPtr const& weakshared_ptr);
  FORCEINLINE CRefCntPtr& operator=(CRefCntPtr const& shared_ptr);
  FORCEINLINE CRefCntPtr& operator=(CRefCntPtr&& shared_ptr);
  FORCEINLINE ORefCntObject& operator*() const { return *RefCntObject; }
  FORCEINLINE ORefCntObject* operator->() const { return RefCntObject; }
  FORCEINLINE ORefCntObject* Get() const { return RefCntObject; }
  FORCEINLINE const bool IsValid() const { return RefCntObject != nullptr; }
  FORCEINLINE void Reset() { *this = CRefCntPtr(); }
  FORCEINLINE int32_t GetSharedReferenceCount() const { return SharedReferencer.GetSharedReferenceCount(); }

 protected:
  ORefCntObject* RefCntObject;
  CSharedReferencer SharedReferencer;

 private:
  friend class CWeakRCPtr;
};

class CWeakRCPtr {
 public:
  FORCEINLINE CWeakRCPtr();
  FORCEINLINE CWeakRCPtr(CWeakRCPtr const& weakrc_ptr);
  FORCEINLINE CWeakRCPtr(CWeakRCPtr&& weakrc_ptr);
  FORCEINLINE CWeakRCPtr(CRefCntPtr const& shared_ptr);
  FORCEINLINE CWeakRCPtr& operator=(CWeakRCPtr const& weakrc_ptr);
  FORCEINLINE CWeakRCPtr& operator=(CWeakRCPtr&& weakrc_ptr);
  FORCEINLINE CWeakRCPtr& operator=(CRefCntPtr const& shared_ptr);
  FORCEINLINE CRefCntPtr ToShared() { return CRefCntPtr(*this); }
  FORCEINLINE bool IsValid() const { return RefCntObject != nullptr && WeakRCReferencer.IsValid(); }
  FORCEINLINE void Reset() { *this = CWeakRCPtr(); }

 protected:
  ORefCntObject* RefCntObject;
  CWeakRCReferencer WeakRCReferencer;

 private:
  friend class CRefCntPtr;
};

template <typename T>
class TRefCntPtr : public CRefCntPtr {
  FORCEINLINE TRefCntPtr(ORefCntObject* rc_object) : CRefCntPtr(rc_object) {}

 public:
 private:
  template <typename O, typename... Args>
  friend TRefCntPtr<O> NewRefCntObject(Args&&... args);
};

template <typename T>
class TWeakRCPtr : public CWeakRCPtr {
 public:
};

template <typename O, typename... Args>
TRefCntPtr<O> NewRefCntObject(Args&&... args) {
  static_assert(std::is_base_of_v<ORefCntObject, O>);
  return TRefCntPtr<O>(new O(std::forward<Args>(args)...));
}

// ORefCntObject
FORCEINLINE ORefCntObject::ORefCntObject() : OObject(), RefCntController(new CRefCntController(this)) {}

FORCEINLINE void ORefCntObject::AddSharedReference() {
  RefCntController->AddSharedReference();
  if (ScriptObjectWeakRCFlag) {
    X_ASSERT(!ScriptObjectWeakRCFlag->Get());
  }
}

FORCEINLINE void ORefCntObject::ReleaseSharedReference() {
  if (ScriptObjectWeakRCFlag) {
    X_ASSERT(ScriptObjectWeakRCFlag->Get());
  }
  RefCntController->ReleaseSharedReference();
}

FORCEINLINE CRefCntController::CRefCntController(ORefCntObject* rc_object) : RefCntObject(rc_object) {
  X_ASSERT(RefCntObject);
}

FORCEINLINE CRefCntController::~CRefCntController() {}

FORCEINLINE int32_t CRefCntController::GetSharedReferenceCount() {
  return SharedReferenceCount.load(std::memory_order_relaxed);
}

FORCEINLINE int32_t CRefCntController::GetWeakReferenceCount() {
  return WeakReferenceCount.load(std::memory_order_relaxed);
}

FORCEINLINE void CRefCntController::AddSharedReference() {
  SharedReferenceCount.fetch_add(1, std::memory_order_relaxed);
}

FORCEINLINE bool CRefCntController::ConditionallyAddSharedReference() {
  int32_t loaded_ref_count = SharedReferenceCount.load(std::memory_order_relaxed);
  for (;;) {
    if (loaded_ref_count == 0) {
      return false;
    }
    if (SharedReferenceCount.compare_exchange_weak(loaded_ref_count, loaded_ref_count + 1, std::memory_order_relaxed)) {
      return true;
    }
  }
}

FORCEINLINE void CRefCntController::ReleaseSharedReference() {
  int32_t old_ref_count = SharedReferenceCount.fetch_sub(1, std::memory_order_acq_rel);
  if (old_ref_count == 1) {
    delete RefCntObject;
    ReleaseWeakReference();
  }
}

FORCEINLINE void CRefCntController::AddWeakReference() {
  WeakReferenceCount.fetch_add(1, std::memory_order_relaxed);
}

FORCEINLINE void CRefCntController::ReleaseWeakReference() {
  int32_t old_ref_count = WeakReferenceCount.fetch_sub(1, std::memory_order_acq_rel);
  if (old_ref_count == 1) {
    delete this;
  }
}

// CSharedReferencer
FORCEINLINE CSharedReferencer::CSharedReferencer(CRefCntController* rc_controller) : RefCntController(rc_controller) {}

FORCEINLINE CSharedReferencer::CSharedReferencer() : RefCntController(nullptr) {}

FORCEINLINE CSharedReferencer::CSharedReferencer(CSharedReferencer const& shared_referencer)
    : RefCntController(shared_referencer.RefCntController) {
  if (RefCntController != nullptr) {
    RefCntController->AddSharedReference();
  }
}

FORCEINLINE CSharedReferencer::CSharedReferencer(CSharedReferencer&& shared_referencer)
    : RefCntController(shared_referencer.RefCntController) {
  shared_referencer.RefCntController = nullptr;
}

FORCEINLINE CSharedReferencer::CSharedReferencer(CWeakRCReferencer const& weakrc_referencer)
    : RefCntController(weakrc_referencer.RefCntController) {
  if (RefCntController != nullptr) {
    if (!RefCntController->ConditionallyAddSharedReference()) {
      RefCntController = nullptr;
    }
  }
}

FORCEINLINE CSharedReferencer::CSharedReferencer(CWeakRCReferencer&& weakrc_referencer)
    : RefCntController(weakrc_referencer.RefCntController) {
  if (RefCntController != nullptr) {
    if (!RefCntController->ConditionallyAddSharedReference()) {
      RefCntController = nullptr;
    }
    weakrc_referencer.RefCntController->ReleaseWeakReference();
    weakrc_referencer.RefCntController = nullptr;
  }
}

FORCEINLINE CSharedReferencer::~CSharedReferencer() {
  if (RefCntController != nullptr) {
    RefCntController->ReleaseSharedReference();
  }
}

FORCEINLINE CSharedReferencer& CSharedReferencer::operator=(CSharedReferencer const& shared_referencer) {
  auto new_rc_controller = shared_referencer.RefCntController;
  if (new_rc_controller != RefCntController) {
    if (new_rc_controller != nullptr) {
      new_rc_controller->AddSharedReference();
    }
    if (RefCntController != nullptr) {
      RefCntController->ReleaseSharedReference();
    }
    RefCntController = new_rc_controller;
  }
  return *this;
}

FORCEINLINE CSharedReferencer& CSharedReferencer::operator=(CSharedReferencer&& shared_referencer) {
  auto new_rc_controller = shared_referencer.RefCntController;
  auto old_rc_controller = RefCntController;
  if (shared_referencer.RefCntController != RefCntController) {
    if (RefCntController != nullptr) {
      RefCntController->ReleaseSharedReference();
    }
    RefCntController = shared_referencer.RefCntController;
    shared_referencer.RefCntController = nullptr;
  }
  return *this;
}

FORCEINLINE const bool CSharedReferencer::IsValid() const {
  return RefCntController != nullptr;
}

FORCEINLINE const int32_t CSharedReferencer::GetSharedReferenceCount() const {
  return RefCntController != nullptr ? RefCntController->GetSharedReferenceCount() : 0;
}

// CWeakRCReferencer
FORCEINLINE CWeakRCReferencer::CWeakRCReferencer() : RefCntController(nullptr) {}

FORCEINLINE CWeakRCReferencer::CWeakRCReferencer(CWeakRCReferencer const& weakrc_referencer)
    : RefCntController(weakrc_referencer.RefCntController) {
  if (RefCntController != nullptr) {
    RefCntController->AddWeakReference();
  }
}

FORCEINLINE CWeakRCReferencer::CWeakRCReferencer(CWeakRCReferencer&& weakrc_referencer)
    : RefCntController(weakrc_referencer.RefCntController) {
  weakrc_referencer.RefCntController = nullptr;
}

FORCEINLINE CWeakRCReferencer::CWeakRCReferencer(CSharedReferencer const& shared_referencer)
    : RefCntController(shared_referencer.RefCntController) {
  if (RefCntController != nullptr) {
    RefCntController->AddWeakReference();
  }
}

FORCEINLINE CWeakRCReferencer::~CWeakRCReferencer() {
  if (RefCntController != nullptr) {
    RefCntController->ReleaseWeakReference();
  }
}

FORCEINLINE CWeakRCReferencer& CWeakRCReferencer::operator=(CWeakRCReferencer const& weakrc_referencer) {
  if (RefCntController != weakrc_referencer.RefCntController) {
    if (weakrc_referencer.RefCntController != nullptr) {
      weakrc_referencer.RefCntController->AddWeakReference();
    }
    if (RefCntController != nullptr) {
      RefCntController->ReleaseWeakReference();
    }
    RefCntController = weakrc_referencer.RefCntController;
  }
  return *this;
}

FORCEINLINE CWeakRCReferencer& CWeakRCReferencer::operator=(CWeakRCReferencer&& weakrc_referencer) {
  if (RefCntController != nullptr) {
    RefCntController->ReleaseWeakReference();
  }
  RefCntController = weakrc_referencer.RefCntController;
  weakrc_referencer.RefCntController = nullptr;
  return *this;
}

FORCEINLINE CWeakRCReferencer& CWeakRCReferencer::operator=(CSharedReferencer const& shared_referencer) {
  if (RefCntController != shared_referencer.RefCntController) {
    if (shared_referencer.RefCntController != nullptr) {
      shared_referencer.RefCntController->AddWeakReference();
    }
    if (RefCntController != nullptr) {
      RefCntController->ReleaseWeakReference();
    }
    RefCntController = shared_referencer.RefCntController;
  }
  return *this;
}

FORCEINLINE const bool CWeakRCReferencer::IsValid() const {
  return RefCntController != nullptr && RefCntController->GetSharedReferenceCount() > 0;
}

// CRefCntPtr
FORCEINLINE CRefCntPtr::CRefCntPtr(ORefCntObject* rc_object)
    : RefCntObject(rc_object), SharedReferencer(rc_object->RefCntController) {}

FORCEINLINE CRefCntPtr::CRefCntPtr() : RefCntObject(nullptr), SharedReferencer() {}

FORCEINLINE CRefCntPtr::CRefCntPtr(CRefCntPtr const& shared_ptr)
    : RefCntObject(shared_ptr.RefCntObject), SharedReferencer(shared_ptr.SharedReferencer) {}

FORCEINLINE CRefCntPtr::CRefCntPtr(CRefCntPtr&& shared_ptr)
    : RefCntObject(shared_ptr.RefCntObject), SharedReferencer(std::move(shared_ptr.SharedReferencer)) {
  shared_ptr.RefCntObject = nullptr;
}

FORCEINLINE CRefCntPtr::CRefCntPtr(CWeakRCPtr const& weakrc_ptr)
    : RefCntObject(weakrc_ptr.RefCntObject), SharedReferencer(weakrc_ptr.WeakRCReferencer) {}

FORCEINLINE CRefCntPtr& CRefCntPtr::operator=(CRefCntPtr const& shared_ptr) {
  RefCntObject = shared_ptr.RefCntObject;
  SharedReferencer = shared_ptr.SharedReferencer;
  return *this;
}

FORCEINLINE CRefCntPtr& CRefCntPtr::operator=(CRefCntPtr&& shared_ptr) {
  if (this != &shared_ptr) {
    RefCntObject = shared_ptr.RefCntObject;
    shared_ptr.RefCntObject = nullptr;
    SharedReferencer = std::move(shared_ptr.SharedReferencer);
  }
  return *this;
}

// CWeakRCPtr
FORCEINLINE CWeakRCPtr::CWeakRCPtr() : RefCntObject(nullptr), WeakRCReferencer() {}

FORCEINLINE CWeakRCPtr::CWeakRCPtr(CWeakRCPtr const& weakrc_ptr)
    : RefCntObject(weakrc_ptr.RefCntObject), WeakRCReferencer(weakrc_ptr.WeakRCReferencer) {}

FORCEINLINE CWeakRCPtr::CWeakRCPtr(CWeakRCPtr&& weakrc_ptr)
    : RefCntObject(weakrc_ptr.RefCntObject), WeakRCReferencer(std::move(weakrc_ptr.WeakRCReferencer)) {
  weakrc_ptr.RefCntObject = nullptr;
}

FORCEINLINE CWeakRCPtr::CWeakRCPtr(CRefCntPtr const& shared_ptr)
    : RefCntObject(shared_ptr.RefCntObject), WeakRCReferencer(shared_ptr.SharedReferencer) {}

FORCEINLINE CWeakRCPtr& CWeakRCPtr::operator=(CWeakRCPtr const& weakrc_ptr) {
  RefCntObject = weakrc_ptr.RefCntObject;
  WeakRCReferencer = weakrc_ptr.WeakRCReferencer;
  return *this;
}

FORCEINLINE CWeakRCPtr& CWeakRCPtr::operator=(CWeakRCPtr&& weakrc_ptr) {
  if (this != &weakrc_ptr) {
    RefCntObject = weakrc_ptr.RefCntObject;
    weakrc_ptr.RefCntObject = nullptr;
    WeakRCReferencer = std::move(weakrc_ptr.WeakRCReferencer);
  }
  return *this;
}

FORCEINLINE CWeakRCPtr& CWeakRCPtr::operator=(CRefCntPtr const& shared_ptr) {
  RefCntObject = shared_ptr.RefCntObject;
  WeakRCReferencer = shared_ptr.SharedReferencer;
  return *this;
}
