// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_BASE_STRINGS_ATOMIC_STRING_FACTORY_H_
#define EVITA_BASE_STRINGS_ATOMIC_STRING_FACTORY_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "base/strings/string_piece.h"
#include "evita/base/evita_base_export.h"
#include "evita/base/memory/zone_owner.h"

namespace base {

class AtomicString;

//////////////////////////////////////////////////////////////////////
//
// AtomicStringFactory
//
class EVITA_BASE_EXPORT AtomicStringFactory final : public evita::ZoneOwner {
 public:
  AtomicStringFactory();
  ~AtomicStringFactory();

  AtomicString New(base::StringPiece16 value);
  AtomicString NewUniqueString(const base::char16* format);

  static AtomicStringFactory* GetInstance();

 private:
  using EntityMap = std::unordered_map<base::StringPiece16,
                                       base::StringPiece16*,
                                       base::StringPiece16Hash>;

  base::StringPiece16* NewStringPiece(base::StringPiece16 value);

  EntityMap map_;
  int unique_name_counter_ = 0;

  DISALLOW_COPY_AND_ASSIGN(AtomicStringFactory);
};

}  // namespace base

#endif  // EVITA_BASE_STRINGS_ATOMIC_STRING_FACTORY_H_
