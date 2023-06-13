#include <seastar/core/sleep.hh>
#include <iostream>

// remember to also pass slow_op arg by reference as then its lifetime will be
// managed by do_with. If passed by value, that would be destroyed when the function
// returns.
seastar::future<> slow_op(T& o) {         // <-- pass by ref
  return seastar::sleep(10ms).then([&o] { // <-- capture by ref
    // first continuation, doing something with o
    ...
  }).then([&o]) { // <- another capture by ref
    // another continuation, using o
    // ...
  });
}

seastar::future<> f() {
  return seastar::do_with(T1(), T2(), [] (auto &obj, auto &obj2) { // <-- T() lives on the heap until
                                                                   // the future resolves. note
                                                                   // must pass by ref.
    // pass by ref, totally fine
    return slow_op(obj);
  }
}
