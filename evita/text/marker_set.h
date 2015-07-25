// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_marker_set_h)
#define INCLUDE_evita_text_marker_set_h

#include <set>

#include "base/basictypes.h"
#include "base/observer_list.h"
#include "evita/text/buffer_mutation_observer.h"
#include "evita/text/marker.h"
#include "evita/text/marker_set_observer.h"

namespace common {
class AtomicString;
}

namespace text {

class MarkerSet;

class MarkerSet : public BufferMutationObserver {
  private: typedef std::set<Marker*> MarkerSetImpl;
  private: class ChangeScope;

  private: MarkerSetImpl markers_;
  private: BufferMutationObservee* const mutation_observee_;
  private: base::ObserverList<MarkerSetObserver> observers_;

  public: MarkerSet(BufferMutationObservee* provider);
  public: virtual ~MarkerSet();

  private: MarkerSetImpl::iterator lower_bound(Posn offset);

  // Remove |observer|
  public: void AddObserver(MarkerSetObserver* observer);

  // Remove all markers in this |MarkerSet|.
  private: void Clear();

  // Get marker at |offset|.
  public: const Marker* GetMarkerAt(Posn offset) const;

  // Get marker starting at |offset| or after |offset|. This function is
  // provided for reducing call for |GetMarkerAt()| on every position in
  // document. See |TextFormatter::TextScanner::spelling()|.
  public: const Marker* GetLowerBoundMarker(Posn offset) const;

  // Notify marker changes to observers.
  private: void NotifyChange(Posn start, Posn end);

  // Insert marker from |start| to |end|, exclusive.
  public: void InsertMarker(Posn start, Posn end,
                            const common::AtomicString& type);

  // Remove marker from |start| to |end|, exclusive.
  private: void RemoveMarker(Posn start, Posn end);
  public: void RemoveMarkerForTesting(Posn start, Posn end) {
    RemoveMarker(start, end);
  }

  // Remove |observer|
  public: void RemoveObserver(MarkerSetObserver* observer);

  // BufferMutationObserver
  private: virtual void DidDeleteAt(Posn offset, size_t length) override;
  private: virtual void DidInsertAt(Posn offset, size_t length) override;
  private: virtual void DidInsertBefore(Posn offset, size_t length) override;

  DISALLOW_COPY_AND_ASSIGN(MarkerSet);
};

}  // namespace text

#endif // !defined(INCLUDE_evita_text_marker_set_h)
