#include <seastar/core/app-template.hh>
#include <seastar/core/reactor.hh>
#include <seastar/util/log.hh>
#include <stdexcept>

#include <iostream>

extern seastar::future<> f();

int main(int argc, char **argv) {
  seastar::app_template app;
  namespace bpo = boost::program_options;

  // clang-format off
  app.add_options()
      ("flag", "some optional flag")
      ("size,s", bpo::value<int>()->default_value(100), "size")
      ;
  // clang-format on

  app.add_positional_options(
      {{"filename",
        bpo::value<std::vector<seastar::sstring>>()->default_value({}),
        "sstable files to verify", -1}});

  try {
    app.run(argc, argv, [&app] {
      auto &args = app.configuration();
      if (args.count("flag")) {
        std::cout << "Flag is on\n";
      }
      std::cout << "Size is " << args["size"].as<int>() << "\n";
      auto &filenames = args["filename"].as<std::vector<seastar::sstring>>();
      for (auto &&fn : filenames) {
        std::cout << fn << "\n";
      }
      return seastar::make_ready_future<>();
    });
  } catch (...) {
    std::cerr << "Couldn't start application: " << std::current_exception()
              << "\n";
    return 1;
  }
  return 0;
}
