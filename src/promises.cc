#include <iostream>
#include <seastar/core/do_with.hh>
#include <seastar/core/future.hh>
#include <seastar/core/sleep.hh>
#include <seastar/core/thread.hh>

seastar::future<int> slow() {
  using namespace std::chrono_literals;
  return seastar::sleep(100ms).then([] { return 3; });
}

seastar::future<> f() {
  return seastar::do_with(seastar::promise<>(), [](auto &promise) {
    (void)seastar::async([&promise]() {
      using namespace std::chrono_literals;
      for (int i = 0; i < 10; i++) {
        std::cout << i << "..." << std::flush;
        seastar::sleep(1s).wait();
      }
      std::cout << std::endl;
      promise.set_value();
    });
    return promise.get_future();
  });
}
