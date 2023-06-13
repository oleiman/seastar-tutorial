#include <seastar/core/sleep.hh>
#include <seastar/core/future.hh>

using namespace seastar;

future<> f() {
  using namespace std::chrono_literals;
  future<int> slow_two = sleep(2s).then([] {return 2; });
  return experimental::when_all(sleep(1s), std::move(slow_two), make_ready_future<double>(3.5));

}
