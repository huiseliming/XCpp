#pragma once
#include "Utils.h"

struct XRENDERER_API IRHIObject {
 protected:
  // IObject* Owner{nullptr};

 public:
  IRHIObject(const IRHIObject&) = delete;
  IRHIObject& operator=(const IRHIObject&) = delete;
  virtual ~IRHIObject() = default;

 protected:
  IRHIObject() = default;
};
