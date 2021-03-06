// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_SWAP_CHAIN_H_
#define EVITA_GFX_SWAP_CHAIN_H_

#include <memory>
#include <vector>

#include "common/win/scoped_comptr.h"
#include "evita/gfx/rect_f.h"

interface IDXGISwapChain2;

namespace gfx {

class DxDevice;

//////////////////////////////////////////////////////////////////////
//
// SwapChain
//
class SwapChain final {
 public:
  ~SwapChain();

  const gfx::RectF& bounds() const { return bounds_; }
  ID2D1DeviceContext* d2d_device_context() const { return d2d_device_context_; }
  IDXGISwapChain2* swap_chain() const { return swap_chain_; }

  void AddDirtyRect(const RectF& dirty_rects);
  static std::unique_ptr<SwapChain> CreateForComposition(DxDevice* device,
                                                         const RectF& bounds);
  static std::unique_ptr<SwapChain> CreateForHwnd(HWND hwnd);
  void DidChangeBounds(const RectF& new_bounds);
  void Present();
  bool UpdateReadyState() const;

 private:
  SwapChain(ID2D1Device* d2d_device,
            common::ComPtr<IDXGISwapChain2> swap_chain);

  void UpdateDeviceContext();

  RectF bounds_;
  common::ComPtr<ID2D1DeviceContext> d2d_device_context_;
  std::vector<Rect> dirty_rects_;
  bool is_first_present_;
  // To avoid calling |WaitForSingleObject()|, |is_ready_| holds last checked
  // result.
  mutable bool is_ready_;
  const common::ComPtr<IDXGISwapChain2> swap_chain_;
  HANDLE const swap_chain_waitable_;

  DISALLOW_COPY_AND_ASSIGN(SwapChain);
};

}  // namespace gfx

#endif  // EVITA_GFX_SWAP_CHAIN_H_
