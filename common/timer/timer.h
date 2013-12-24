// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_timer_timer_h)
#define INCLUDE_common_timer_timer_h

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "common/common_export.h"

namespace common {
namespace impl {

struct COMMON_EXPORT TimerEntry;

class COMMON_EXPORT AbstractTimer {
  friend class TimerController;
  private: scoped_refptr<TimerEntry> entry_;
  protected: AbstractTimer();
  public: virtual ~AbstractTimer();
  public: bool is_active() const { return entry_; }
  public: void Start(int next_fire_interval_ms, int repeat_interval_ms);
  public: void Stop();
  protected: virtual void Fire() = 0;
  DISALLOW_COPY_AND_ASSIGN(AbstractTimer);
};

template<class Receiver, class TimerClass>
class Timer : public AbstractTimer {
  public: typedef void (Receiver::*FiredFunction)(TimerClass*);
  private: Receiver* const receiver_;
  private: FiredFunction const function_;
  protected: Timer(Receiver* receiver, FiredFunction function)
      : receiver_(receiver),
        function_(function) {
  }
  private: virtual void Fire() override {
    (receiver_->*function_)(static_cast<TimerClass*>(this));
  }
  DISALLOW_COPY_AND_ASSIGN(Timer);
};

} // namespace impl

template<class Receiver>
class OneShotTimer : public impl::Timer<Receiver, OneShotTimer<Receiver>> {
  public: OneShotTimer(Receiver* receiver, FiredFunction function)
      : Timer(receiver, function) {
  }
  public: void Start(int interval_ms) { Timer::Start(interval_ms, 0); }
  DISALLOW_COPY_AND_ASSIGN(OneShotTimer);
};

template<class Receiver>
class RepeatingTimer : public impl::Timer<Receiver, RepeatingTimer<Receiver>> {
  public: RepeatingTimer(Receiver* receiver, FiredFunction function)
      : Timer(receiver, function) {
  }
  public: void Start(int interval_ms) {
    Timer::Start(interval_ms, interval_ms);
  }
  DISALLOW_COPY_AND_ASSIGN(RepeatingTimer);
};

} // namespace common

#endif //!defined(INCLUDE_common_timer_timer_h)
