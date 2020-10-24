// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "util/arena.h"

namespace leveldb {

static const int kBlockSize = 4096;

Arena::Arena()
    : alloc_ptr_(nullptr), alloc_bytes_remaining_(0), memory_usage_(0) {}

Arena::~Arena() {
  for (size_t i = 0; i < blocks_.size(); i++) {
    delete[] blocks_[i];
  }
}

// 剩余空间不足时，交给 AllocateFallback 处理
char* Arena::AllocateFallback(size_t bytes) {
  // 如果需要分配的空间大于 kBlockSize/4，则直接申请一个 bytes 大小的 block；
  // 这样能保证浪费的空间小于 kBlockSize/4
  if (bytes > kBlockSize / 4) {
    // Object is more than a quarter of our block size.  Allocate it separately
    // to avoid wasting too much space in leftover bytes.
    char* result = AllocateNewBlock(bytes);
    return result;
  }

  // We waste the remaining space in the current block.
  // 重新开辟新的块空间进行分配，上一个块剩下的空间都浪费了
  alloc_ptr_ = AllocateNewBlock(kBlockSize);
  alloc_bytes_remaining_ = kBlockSize;

  char* result = alloc_ptr_;
  alloc_ptr_ += bytes;
  alloc_bytes_remaining_ -= bytes;
  return result;
}

char* Arena::AllocateAligned(size_t bytes) {
  // 获取分配的最小单位，如果指针大小大于 8 Bytes，则取指针大小，否则使用 8 Bytes 作为最小的对齐单位
  const int align = (sizeof(void*) > 8) ? sizeof(void*) : 8;
  // 保证只有 2 的 n 次方才能满足
  static_assert((align & (align - 1)) == 0,
                "Pointer size should be a power of 2");
  // 取余
  size_t current_mod = reinterpret_cast<uintptr_t>(alloc_ptr_) & (align - 1);
  // 偏移量
  size_t slop = (current_mod == 0 ? 0 : align - current_mod);
  // 总共需要分配的空间
  size_t needed = bytes + slop;
  char* result;
  if (needed <= alloc_bytes_remaining_) {
    result = alloc_ptr_ + slop;
    alloc_ptr_ += needed;
    alloc_bytes_remaining_ -= needed;
  } else {
    // AllocateFallback always returned aligned memory
    result = AllocateFallback(bytes);
  }
  assert((reinterpret_cast<uintptr_t>(result) & (align - 1)) == 0);
  return result;
}

char* Arena::AllocateNewBlock(size_t block_bytes) {
  // 申请一个新的 block，并将块的起始指针存入 blocks_ 中
  char* result = new char[block_bytes];
  blocks_.push_back(result);
  // 更新 memory_usage 的大小
  memory_usage_.fetch_add(block_bytes + sizeof(char*),
                          std::memory_order_relaxed);
  return result;
}

}  // namespace leveldb
