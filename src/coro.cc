#include <seastar/core/circular_buffer.hh>
#include <seastar/core/coroutine.hh>
#include <seastar/core/sleep.hh>
#include <seastar/coroutine/generator.hh>
#include <seastar/coroutine/maybe_yield.hh>
#include <seastar/coroutine/all.hh>
#include <seastar/coroutine/parallel_for_each.hh>

seastar::future<int> read();
seastar::future<> write(int n);

seastar::future<int> slow_fetch_and_increment() {
  auto n = co_await read();
  co_await seastar::sleep(1s);
  auto new_n = n + 1;
  co_await write(new_n);
  co_return n;
}

seastar::future<> foo() {
  int n = 3;
  int m =
    co_await seastar::yield().then(seastar::coroutine::lambda([n] () -> seastar::future<int> {
	co_await seastar::coroutine::maybe_yield();
	// `n` can be safely used here
	co_return n;
      }));

  assert(n == m);
}

seastar::future<Preprocessed> prepare_ingredients(Ingredients&&);
seastar::future<Dish> cook_a_dish(Preprocessed&&);
seastar::future<> consume_a_dish(Dish&&);

seastar::coroutine::experimental::generator<Dish, seastar::circular_buffer>
make_dishes(coroutine::experimental::buffer_size_t max_dishes_on_table,
	    Ingredients&& ingredients) {
  while (ingredients) {
    auto some_ingredients = ingredients.alloc();
    auto preprocessed = co_await prepare_ingredients(std::move(some_ingredients));
    co_yield co_await cook_a_dish(std::move(preprocessed));
  }
}

seastar::future<> have_a_dinner(unsigned max_dishes_on_table) {
  Ingredients ingredients;
  auto dishes = make_dishes(std::move(ingredients));
  while (auto dish = co_await dishes()) {
    co_await consume_a_dish(std::move(dish));
  }
}

seastar::future<> function_returning_an_exceptional_future();

seastar::future<> exception_handling() {
  try {
    co_await function_returning_an_exceptional_future();
  } catch (...) {
    // exception handled here
  }
  throw 3; // captured by coroutine and returned as
           // an exceptional future
}

seastar::future<> exception_propagating() {
  std::exception_ptr eptr;
  try {
    co_await function_returning_an_exceptional_future();
  } catch (...) {
    eptr = std::current_exception();
  }
  if (eptr) {
    co_return seastar::coroutine::exception(eptr); // Saved exception pointer can be
                                                   // propagated w/o throwing
  }
  co_return seastar::coroutine::make_exception(3); // Custom exceptions can be propagated
                                                   // without throwing
}

seastar::future<int> read(int key);

seastar::future<int> parallel_sum(int key1, int key2) {
  int [a, b] = co_await seastar::coroutine::all
    (
     [&] {
       return read(key1);
     },
     [&] {
       return read(key2);
     });
  co_return a + b;
}

seastar::future<bool> all_exist(std::vector<sstring> filenames) {
  bool res = true;
  co_await seastar::coroutine::parallel_for_each(filenames,
						 [&res] (const seastar::sstring& name) ->
						 seastar::future<> {
						   res &= co_await seastar::file_exists(name);
						 });
  co_return res;
}

seastar::future<int> long_loop(int n) {
  float acc = 0;
  for (int i = 0; i < n; ++i) {
    acc += std::sin(float(i));
    // Give the Seastar reactor opportunity to perform I/O or schedule
    // other tasks
    co_await seastar::coroutine::maybe_yield();
  }
}

// Bypassing preemption checks

struct resource;
seastar::future<int> compute_always_ready(int i, resource &r);

seastar::future<int> accumulate(int n, resource &important_resource) {
  float acc = 0;
  for (int i = 0; i < n; ++i) {
    // This await will not yield control, so we're sure that nobody will be
    // able to touch important_resource while we accumulate all the results.
    acc +=
      co_await seastar::coroutine::without_preemption_check(
          compute_always_ready(i, important_resource));
  }
}
