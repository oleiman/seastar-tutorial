#include <exception>
#include <iostream>
#include <seastar/core/future.hh>
#include <seastar/util/backtrace.hh>
#include <seastar/util/log.hh>

seastar::future<> g() {
  // return seastar::make_exception_future_with_backtrace<>(
  //     std::runtime_error("hello"));
  seastar::throw_with_backtrace<std::runtime_error>("hello");
}

seastar::future<> f() {
  return g().handle_exception(
      [](std::exception_ptr e) { std::cerr << "Exception: " << e << "\n"; });
}
