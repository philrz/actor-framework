// This file is part of CAF, the C++ Actor Framework. See the file LICENSE in
// the main distribution directory for license terms and copyright or visit
// https://github.com/actor-framework/actor-framework/blob/master/LICENSE.

#define CAF_SUITE constructor_attach

#include "caf/all.hpp"

#include "core-test.hpp"

using namespace caf;

namespace {

class testee : public event_based_actor {
public:
  testee(actor_config& cfg, actor buddy) : event_based_actor(cfg) {
    attach_functor([=](const error& rsn) { send(buddy, ok_atom_v, rsn); });
  }

  behavior make_behavior() override {
    return {
      [=](delete_atom) { quit(exit_reason::user_shutdown); },
    };
  }
};

class spawner : public event_based_actor {
public:
  spawner(actor_config& cfg)
    : event_based_actor(cfg),
      downs_(0),
      testee_(spawn<testee, monitored>(this)) {
    set_down_handler([=](down_msg& msg) {
      CAF_CHECK_EQUAL(msg.reason, exit_reason::user_shutdown);
      CAF_CHECK_EQUAL(msg.source, testee_.address());
      if (++downs_ == 2)
        quit(msg.reason);
    });
  }

  behavior make_behavior() override {
    return {
      [=](ok_atom, const error& reason) {
        CAF_CHECK_EQUAL(reason, exit_reason::user_shutdown);
        if (++downs_ == 2) {
          quit(reason);
        }
      },
      [=](delete_atom x) { return delegate(testee_, x); },
    };
  }

  void on_exit() override {
    destroy(testee_);
  }

private:
  int downs_;
  actor testee_;
};

} // namespace

CAF_TEST(constructor_attach) {
  actor_system_config cfg;
  actor_system system{cfg};
  anon_send(system.spawn<spawner>(), delete_atom_v);
}
