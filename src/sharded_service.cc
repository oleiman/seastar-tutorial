#include <seastar/core/future.hh>
// #include <seastar/core/reactor.hh>
#include <seastar/core/resource.hh>
#include <seastar/core/sharded.hh>

#include <iostream>
#include <seastar/core/smp.hh>

class my_service {
public:
  std::string _str;
  explicit my_service(const std::string &str)
      : _str(str + std::to_string(seastar::this_shard_id())) {}
  seastar::future<> run() {
    std::cerr << "running on " << seastar::this_shard_id()
              << ", _str = " << _str << "\n";
    return invoke_other((seastar::this_shard_id() + 1) % seastar::smp::count);
    // return seastar::make_ready_future<>();
  }

  seastar::future<> invoke_other(int n);

  seastar::future<> stop() { return seastar::make_ready_future<>(); }
};

seastar::sharded<my_service> s;

seastar::future<> f() {
  return s.start(std::string("hello"))
      .then([] {
        return s.invoke_on_all(
            [](my_service &local_service) { return local_service.run(); });
      })
      .then([] { return s.stop(); });
}

seastar::future<> my_service::invoke_other(int n) {
  return s.invoke_on(n, [](my_service &local_service) {
    std::cerr << "invoked on " << seastar::this_shard_id()
              << ", _str = " << local_service._str << "\n";
  });
}
