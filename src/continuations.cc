#include <seastar/core/sleep.hh>
#include <iostream>

seastar::future<int> incr(int i) {
  using namespace std::chrono_literals;
  return seastar::sleep(10ms).then([&i] { return i + 1; });
}

seastar::future<int> slow_do_something(std::unique_ptr<T> obj) {
  using namespace std::chrono::literals;

  // () mutable allows us to std::move the captured-by-move object inside
  // the lambda/continuation
  return seastar::sleep(10ms).then([obj = std::move(obj)] () mutable {
    return do_something(std::move(obj));
  });
}

seastar::future<> f() {
  return incr(3).then([] (int val) {
    std::cout << "Got " << val << "\n";
  });
}
