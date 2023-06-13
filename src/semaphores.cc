/*
// semaphore works as you'd expect, with the wait returning a future
// note that the semaphore is thread_local. the semaphore is intended to limit
// usage within a single shard, e.g. if the outstanding operation consumes some
// limited resource.
seastar::future<> g() {
  static thread_local seastar::semaphore limit(100);
  return limit.wait(1).then([] {
    return slow(); // the real work
  }).finally([] {
    limit.signal(1);
  });
}

// buffed semaphore usage
// notice that this is not exception safe. e.g. if slow() throws an
// exception rather than returning an exceptional future.
// very easy to get things wrong using wait/signal separately (as usual)
seastar::future<> g() {
  static thread_local seastar::semaphore limit(100);
  return limit.wait(1).then([] {
    return slow().finally([] { limit.signal(1); });
  });
}

// lambda functions and RAII can fix this
seastar::future<> g() {
  static thread_local seastar::semaphore limit(100);
  return seastar::with_semaphore(limit, 1, [] {
    return slow(); // the real work
  });
}

// even better (or more general at least)
// get_units returns an object that releases the units on destruction
// notice that we need to keep units around in that last continuation,
// otherwise (assuming slow doesn't immediately return) the units will
// be destructed _before_ the future returned by slow resolves.
seastar::future<> g() {
  static thread_local semaphore limit(100);
  return seastar::get_units(limit, 1).then([] (auto units) {
    return slow().finally([units = std::move(units)] {});
  });
}

// can use semaphores in a roundabout way to limit usage of some resource (e.g. memory).
// NOTE: need to make sure that `bytes` doesn't exceed the limit, otherwise the semaphore
// will wait forever
seastar::future<> using_lots_of_memory(size_t bytes) {
  static thread_local seastar::semaphore limit(1000000); // limit to 1MB
  return seastar::with_semaphore(limit, bytes, [bytes] {
    // do something allocating `bytes`B of memory
  });
}
*/

// Limiting loop parallelism
#include <seastar/core/sleep.hh>
#include <seastar/core/future-util.hh>
seastar::future<> slow() {
  std::cerr << ".";
  return seastar::sleep(std::chrono::seconds(1));
}


// this is a serialized loop. each call to slow waits for the previous one to
// complete
// seastar::future<> f() {
//   return seastar::repeat([] {
//     return slow().then([] { return seastar::stop_iteration::no; });
//   });
// }

// what if we want some parallelism on these slow calls?
// Naively
// seastar::future<> f() {
//   return seastar::repeat([] {
//     slow();
//     return seastar::stop_iteration::no;
//   });
// }

// this is no good though, as there's no limit on how many slow() futures we might spawn
// which could lead to OOM and a crash
// instead we use a do_with to create a semaphore for the lifetime of the loop
// note that we don't have a thread_local semaphore here. Each invocation of f will have
// its own semaphore to limit the parallelism of the loop therein.
// Like the naive version, we don't wait for slow to resolve before moving on. we don't
// return the future chain starting at futurize_invoke. we just let it go, but ensure
// that we signal `limit` after it does resolve.
// Also note that with_semaphore won't work here, which wouldn't signal `limit` until
// the lambda's contained future resolves.
// seastar::future<> f() {
//   return seastar::do_with(seastar::semaphore(100), [] (auto &limit) {
//     return seastar::repeat([&limit] {
//       return limit.wait(1).then([&limit] {
// 	seastar::futurize_invoke(slow).finally([&limit] {
// 	  limit.signal(1);
// 	});
// 	return seastar::stop_iteration::no;
//       });
//     });
//   });
// }

// get_units works though
// seastar::future<> f() {
//   return seastar::do_with(seastar::semaphore(100), [] (auto &limit) {
//     return seastar::repeat([&limit] {
//       return seastar::get_units(limit, 1).then([] (auto units) {
// 	slow().finally([units = std::move(units)] {});
// 	return seastar::stop_iteration::no;
//       });
//     });
//   });
// }

// these are infinite loops though. we want loops to end, and at the end
// we need to synchronize on all those background operations
// this can be done by waiting on the original count of the semaphore
// so this will fire of 100 slow operations at a time, wait for them all to
// complete, then fire off at most 100 more, until we reach 456. Maybe not
// precisely that, but that's the effect. Without the finally the future
// returned by f() would resolve before all the slow() operations are
// complete.
#include <boost/iterator/counting_iterator.hpp>
// seastar::future<> f() {
//   return seastar::do_with(seastar::semaphore(100), [] (auto &limit) {
//     return seastar::do_for_each(boost::counting_iterator<int>(0),
// 				boost::counting_iterator<int>(456),
// 				[&limit] (int i) {
// 				  return seastar::get_units(limit, 1).then([] (auto units) {
// 				    slow().finally([units = std::move(units)] {});
// 				  });
// 				}).finally([&limit] {
// 				  return limit.wait(100);
// 				});
//   });
// }

// in the above case we use the same semaphore to limit parallelism AND
// wait for completion. But we might want a single semaphore to control
// the parallelism of multiple loops. Or maybe total usage of some resource.
// In the following case we have an external semaphore and a seastar::gate.
// each iteration enters the gate, which can't close until everyone who has
// entered has left.
#include <seastar/core/gate.hh>

thread_local seastar::semaphore limit(100);
seastar::future<> f() {
  return seastar::do_with(seastar::gate(), [] (auto& gate) {
    return seastar::do_for_each(
      boost::counting_iterator<int>(0),
      boost::counting_iterator<int>(456),
      [&gate] (int i) {
	return seastar::get_units(limit, 1).then([&gate] (auto units) {
	  gate.enter();
	  seastar::futurize_invoke(slow).finally([&gate, units = std::move(units)] {
	    gate.leave();
	  });
	});
      }).finally([&gate] {
	return gate.close();
      });
  });
}

