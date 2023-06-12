#include <seastar/core/future.hh>
#include <iostream>
#include <exception>

class my_exception : public std::exception {
  virtual const char* what() const noexcept override { return "my exception"; }
};

// An exceptional future?
seastar::future<> fail() {
  // return seastar::make_exception_future<>(my_exception());
  throw my_exception();
}

seastar::future<> f() {
  return fail().finally([] {
    std::cout << "cleaning up\n";
  });
}
