#include <seastar/core/sleep.hh>
#include <seastar/core/future.hh>
#include <seastar/core/future-util.hh>

using namespace seastar;

// note that when_all accepts only rvalues
// future<> f() {
//   using namespace std::chrono_literals;
//   future<int> slow_two = sleep(2s).then([] {return 2; });
//   return when_all(sleep(1s),
// 		  std::move(slow_two),
// 		  make_ready_future<double>(3.5)
// 		  ).discard_result();

// }

// future<> f() {
//   using namespace std::chrono_literals;
//   future<int> slow_two = sleep(2s).then([] {return 2; });
//   return when_all(sleep(1s), std::move(slow_two),
// 		  make_ready_future<double>(3.5)
// 		  ).then([] (auto tup) {
// 		    std::cout << std::get<0>(tup).available() << "\n";
// 		    std::cout << std::get<1>(tup).get0() << "\n";
// 		    std::cout << std::get<2>(tup).get0() << "\n";
// 		  });
// }

// if one of the futures resolves to an exception, we still get the result tuple in
// the continuation
// note that the second one is both available AND failed
// future<> f() {
//   using namespace std::chrono_literals;
//   future<> slow_success = sleep(1s);
//   future<> slow_exception = sleep(2s).then([] { throw 1; });
//   return when_all(std::move(slow_success), std::move(slow_exception)
// 		  ).then([] (auto tup) {
// 		    std::cout << std::get<0>(tup).available() << "\n";
// 		    std::cout << std::get<1>(tup).available() << "\n";
// 		    std::cout << std::get<1>(tup).failed() << "\n";
// 		    // ignoring this silently will produce a runtime error
// 		    std::get<1>(tup).ignore_ready_future();
// 		  });
// }

// that was kind of a pain in the butt. tuple syntax and ignoring failures and whatnot
// future<> f() {
//   using namespace std::chrono_literals;
//   future<int> slow_two = sleep(2s).then([] {return 2; });
//   return when_all_succeed(sleep(1s),
// 			  std::move(slow_two),
// 			  make_ready_future<double>(3.5)
// 			  ).then_unpack([] (int i, double d) {
// 			    std::cout << i << " " << d << "\n";
// 			  });
// }


// if none of the futures resolve to a value, then we get a future<tuple<>>
// future<> f() {
//   using namespace std::chrono_literals;
//   return when_all_succeed(sleep(1s),
// 			  sleep(2s),
// 			  sleep(3s)
// 			  ).then_unpack([] () {
// 			    return make_ready_future<>();
// 			  });
// }

future<> f() {
  using namespace std::chrono_literals;
  return when_all_succeed(make_ready_future<int>(2),
			  make_exception_future<double>("oops")
			  ).then_unpack([] (int i, double d) {
			    std::cout << i << " " << d << "\n";
			  }).handle_exception([] (std::exception_ptr e) {
			    try {
			      std::rethrow_exception(e);
			    } catch (const char *e) {
			      std::cout << "exception: " << e << "\n";
			    }
			  });
}
