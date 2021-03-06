/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SKIPWEAKACCEPTOR_H
#define HERMES_VM_SKIPWEAKACCEPTOR_H

#include "hermes/VM/GC.h"
#include "hermes/VM/SlotAcceptorDefault.h"

namespace hermes {
namespace vm {

/// In WeakMap marking, if we mark the weak ref slots referenced by a
/// WeakMap early, this breaks assertions when we attempt to access
/// the slots while determining which keys are reachable.  This
/// Acceptor type delegates to another Acceptor, except when marking
/// WeakRefs -- those are ignored.  The type of the "target" acceptor, to which
/// the calls are delegated, is a template parameter so that those calls are
/// non-virtual (since we know the exact type).
template <typename TargetAcceptor>
class SkipWeakRefsAcceptor final : public SlotAcceptorDefault,
                                   public WeakRootAcceptor {
 public:
  SkipWeakRefsAcceptor(GC &gc, TargetAcceptor *acceptor)
      : SlotAcceptorDefault(gc), acceptor_(acceptor) {}

  using SlotAcceptorDefault::accept;

  void accept(void *&ptr) override {
    acceptor_->accept(ptr);
  }
#ifdef HERMESVM_COMPRESSED_POINTERS
  void accept(BasedPointer &ptr) override {
    acceptor_->accept(ptr);
  }
#endif
  void accept(HermesValue &hv) override {
    acceptor_->accept(hv);
  }
  void accept(SymbolID sym) override {
    acceptor_->accept(sym);
  }
  void accept(WeakRefBase &wr) override {
    // Do nothing -- will be done later.
  }

  void acceptWeak(void *&ptr) override {
    acceptor_->accept(ptr);
  }
#ifdef HERMESVM_COMPRESSED_POINTERS
  /// This gets a default implementation: extract the real pointer to a local,
  /// call acceptWeak on that, write the result back as a BasedPointer.
  void acceptWeak(BasedPointer &ptr) override {
    acceptor_->accept(ptr);
  }
#endif

 private:
  TargetAcceptor *acceptor_;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SKIPWEAKACCEPTOR_H
