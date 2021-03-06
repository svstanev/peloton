//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// gc_manager_factory.h
//
// Identification: src/include/gc/gc_manager_factory.h
//
// Copyright (c) 2015-16, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//


#pragma once

#include "gc/gc_manager.h"
#include "gc/transaction_level_gc_manager.h"


namespace peloton {
namespace gc {

class GCManagerFactory {
 public:
  static GCManager &GetInstance() {
    switch (gc_type_) {

      case GARBAGE_COLLECTION_TYPE_ON:
        return TransactionLevelGCManager::GetInstance(gc_thread_count_);

      default:
        return GCManager::GetInstance();
    }
  }

  static void Configure(int thread_count = 1) { 
    gc_type_ = GARBAGE_COLLECTION_TYPE_ON;
    gc_thread_count_ = thread_count;
  }

  static GarbageCollectionType GetGCType() { return gc_type_; }

 private:
  // GC type
  static GarbageCollectionType gc_type_;

  static int gc_thread_count_;
};

}  // namespace gc
}  // namespace peloton
