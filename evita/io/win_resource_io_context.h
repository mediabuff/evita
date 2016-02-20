// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_IO_WIN_RESOURCE_IO_CONTEXT_H_
#define EVITA_IO_WIN_RESOURCE_IO_CONTEXT_H_

#include <utility>

#include "base/macros.h"
#include "evita/dom/public/io_context_id.h"
#include "evita/io/io_context.h"

namespace io {

//////////////////////////////////////////////////////////////////////
//
// WinResourceIoContext
//
class WinResourceIoContext final : public IoContext {
 public:
  WinResourceIoContext(const domapi::IoContextId& context_id, HMODULE module);
  ~WinResourceIoContext() final;

  std::pair<int, int> Load(const base::string16& type,
                           const base::string16& name,
                           uint8_t* buffer,
                           size_t buffer_size);

  static std::pair<HMODULE, int> Open(const base::string16& file_name);

 private:
  // io::IoContext
  void Close(const domapi::IoIntPromise& promise) final;

  const domapi::IoContextId context_id_;
  HMODULE const module_;

  DISALLOW_COPY_AND_ASSIGN(WinResourceIoContext);
};

}  // namespace io

#endif  // EVITA_IO_WIN_RESOURCE_IO_CONTEXT_H_
