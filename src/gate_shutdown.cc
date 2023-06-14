#include <seastar/core/sleep.hh>
#include <seastar/core/gate.hh>
#include <seastar/core/future-util.hh>
#include <boost/iterator/counting_iterator.hpp>

seastar::future<> slow(int i) {
  std::cerr << "starting " << i << "\n";
  return seastar::sleep(std::chrono::seconds(10)).then([i] {
    std::cerr << "done " << i << "\n";
  });
}

seastar::future<> slow2(int i, seastar::gate &g) {
  std::cerr << "starting " << i << "\n";
  return seastar::do_for_each(
    boost::counting_iterator<int>(0),
    boost::counting_iterator<int>(10),
    [&g] (int) {
      g.check();
      return seastar::sleep(std::chrono::seconds(1));
    }).finally([i] {
      std::cerr << "done " << i << "\n";
    });
}

seastar::future<> f() {
  return seastar::do_with(seastar::gate(), [] (auto &g) {
    return seastar::do_for_each(
      boost::counting_iterator<int>(1),
      boost::counting_iterator<int>(6),
      [&g] (int i) {
	seastar::with_gate(g, [i, &g] { return slow2(i, g); });
	return seastar::sleep(std::chrono::seconds(1));
      }).then([&] {
	seastar::sleep(std::chrono::seconds(1)).then([&g] {
	  seastar::with_gate(g, [&g] { return slow2(6, g); });
	});
	return g.close();
      });
  });
}

