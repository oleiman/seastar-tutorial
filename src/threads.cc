#include <seastar/core/file-types.hh>
#include <seastar/core/file.hh>
#include <seastar/core/seastar.hh>
#include <seastar/core/sleep.hh>
#include <seastar/core/thread.hh>

// seastar::future<> f() {
//   seastar::thread th([] {
//     std::cout << "Hi.\n";
//     for (int i = 1; i < 4; ++i) {
//       seastar::sleep(std::chrono::seconds(1)).get();
//       std::cout << i << "\n";
//     }
//   });
//   return seastar::do_with(std::move(th), [](auto &th) { return th.join(); });
// }

// even better, we can start and join the thread using seastar::async
seastar::future<> f() {
  return seastar::async([] {
    std::cout << "Hi.\n";
    for (int i = 1; i < 4; ++i) {
      seastar::sleep(std::chrono::seconds(1)).get();
      std::cout << i << "\n";
    }
  });
}

// seastar::async's lambda can return a value
seastar::future<seastar::sstring> read_file(seastar::sstring file_name) {
  return seastar::async([file_name] { // lambda executed in a thread
    seastar::file f = seastar::open_file_dma(file_name, seastar::open_flags::ro)
                          .get0(); // get0 is "blocking"
    auto buf =
        f.dma_read<char>(0ul, static_cast<size_t>(512)).get0(); // "block" again
    return seastar::sstring(buf.get(), buf.size());
  });
}
