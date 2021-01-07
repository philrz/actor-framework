// This file is part of CAF, the C++ Actor Framework. See the file LICENSE in
// the main distribution directory for license terms and copyright or visit
// https://github.com/actor-framework/actor-framework/blob/master/LICENSE.

#pragma once

#include "caf/local_actor.hpp"

#include "caf/detail/invoke_result_visitor.hpp"

namespace caf::detail {

template <class Self>
class default_invoke_result_visitor : public invoke_result_visitor {
public:
  using super = invoke_result_visitor;

  default_invoke_result_visitor(Self* ptr) : self_(ptr) {
    // nop
  }

  ~default_invoke_result_visitor() override {
    // nop
  }

  using super::operator();

  void operator()(error& x) override {
    CAF_LOG_TRACE(CAF_ARG(x));
    delegate(x);
  }

  void operator()(message& x) override {
    CAF_LOG_TRACE(CAF_ARG(x));
    delegate(x);
  }

private:
  void deliver(response_promise& rp, error& x) {
    CAF_LOG_DEBUG("report error back to requesting actor");
    rp.deliver(std::move(x));
  }

  void deliver(response_promise& rp, message& x) {
    CAF_LOG_DEBUG("respond via response_promise");
    // suppress empty messages for asynchronous messages
    if (x.empty() && rp.async())
      return;
    rp.deliver(std::move(x));
  }

  template <class T>
  void delegate(T& x) {
    auto rp = self_->make_response_promise();
    if (!rp.pending()) {
      CAF_LOG_DEBUG("suppress response message: invalid response promise");
      return;
    }
    deliver(rp, x);
  }

  Self* self_;
};

} // namespace caf::detail
