// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_TABLE_BLOCK_H_
#define STORAGE_LEVELDB_TABLE_BLOCK_H_

#include <cstddef>
#include <cstdint>

#include "leveldb/iterator.h"

namespace leveldb {

struct BlockContents;
class Comparator;

class Block {
 public:
  // Initialize the block with the specified contents.
  explicit Block(const BlockContents& contents);

  Block(const Block&) = delete;
  Block& operator=(const Block&) = delete;

  ~Block();

  size_t size() const { return size_; }
  // 对 Block 中的键值对进行遍历和查找
  Iterator* NewIterator(const Comparator* comparator);

 private:
  class Iter;

  // 获取最后 4 字节存储复活点的数量
  uint32_t NumRestarts() const;

  // 数据指针
  const char* data_;
  // 数据长度
  size_t size_;
  // 存储复活点列表的位置
  uint32_t restart_offset_;  // Offset in data_ of restart array
  bool owned_;               // Block owns data_[]
};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_TABLE_BLOCK_H_
