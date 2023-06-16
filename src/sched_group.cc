#include <seastar/core/future-util.hh>
#include <seastar/core/future.hh>
#include <seastar/core/loop.hh>
#include <seastar/core/seastar.hh>
#include <seastar/core/sleep.hh>
#include <seastar/core/smp.hh>

#include <boost/range/irange.hpp>

seastar::future<long> loop(int parallelism, bool &stop) {
  return seastar::do_with(0L, [parallelism, &stop](long &counter) {
    return seastar::parallel_for_each(
               boost::irange<unsigned>(0, parallelism),
               [&stop, &counter](unsigned c) {
                 std::cout << seastar::this_shard_id() << std::endl; //
                 return seastar::do_until(
                     [&stop] { return stop; },
                     [&counter] {
                       ++counter;
                       return seastar::make_ready_future<>();
                     });
               })
        .then([&counter] { return counter; });
  });
}

seastar::future<long> loop_in_sg(int parallelism, bool &stop,
                                 seastar::scheduling_group sg) {
  return seastar::with_scheduling_group(
      sg, [parallelism, &stop] { return loop(parallelism, stop); });
}

// seastar::future<> f() {
//   return seastar::do_with(false, [](bool &stop) {
//     using namespace std::chrono_literals;
//     seastar::sleep(10s).then([&stop] { stop = true; });
//     return seastar::when_all_succeed(loop(1, stop), loop(10, stop)) //
//         .then_unpack([](long n1, long n2) {
//           std::cout << "Counters: " << n1 << ", " << n2 << "\n";
//         }); //
//   });
// }

seastar::future<> f() {
  return seastar::when_all_succeed(
             seastar::create_scheduling_group("loop1", 100),
             seastar::create_scheduling_group("loop2", 100))
      .then_unpack(
          [](seastar::scheduling_group sg1, seastar::scheduling_group sg2) {
            return seastar::do_with(false, [sg1, sg2](bool &stop) {
              using namespace std::chrono_literals;
              (void)seastar::sleep(10s).then([&stop] { stop = true; });
              return seastar::when_all_succeed(loop_in_sg(1, stop, sg1),
                                               loop_in_sg(10, stop, sg2))
                  .then_unpack([](long n1, long n2) {
                    std::cout << "Counters: " << n1 << ", " << n2 << "\n";
                  }); //
            });
          });
}
