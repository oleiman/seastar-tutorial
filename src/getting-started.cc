#include <seastar/core/sleep.hh>
#include <iostream>

seastar::future<int> fast() {
  using namespace std::chrono_literals;
  // return seastar::sleep(100ms).then([] { return 3; });
  return seastar::make_ready_future<int>(3);
}

seastar::future<> f() {
  return fast().then([] (int val) {
    std::cout << "Got " << val << "\n";
  });
}
