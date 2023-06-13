seastar::future<int> recompute_number(int number);

// note that the future in the loop body must resolve before the body is executed again
seastar::future<> push_until_100(seastar::lw_shared_ptr<std::vector<int>> queue, int element) {
  return seastar::repeat([queue, element] {
    if (queue->size() == 100) {
      return make_ready_future<stop_iteration>(stop_iteration::yes);
    }
    return recompute_number(element).then([queue] (int new_element) {
      queue->push_back(new_element);
      return stop_iteration::no;
    });
  });
}

// same as above, but passing an explicit condition (size limit for queue)
// note that the loop body should return a future to allow composition of continuations
// and whatnot inside the loop.
seastar::future<> push_until_100(seastar::lw_shared_ptr<std::vector<int>> queue, int element) {
  return seastar::do_until([queue] { return queue->size() == 100; }, [queue, element] {
    return recompute_number(element).then([queue] (int new_element) {
      queue->push_back(new_element);
    });
  })
}

seastar::future<> append(seastar::lw_shared_ptr<std::vector<int>> queue1,
			 seastar::lw_shared_ptr<std::vector<int>> queue2) {
  return seastar::do_for_each(queue2, [queue1] (int element) {
    queue1->push_back(element);
  });
}

seastar::future<> append_iota(seastar::lw_shared_ptr<std::vector<int>> queue1, int n) {
  return seastar::do_for_each(boost::make_counting_iterator<size_t>(0),
			      boost::make_counting_iterator<size_t>(n),
			      [queue1] (int element) {
				queue1->push_back(element);
			      });
}

seastar::future<> do_something(int number);

// do_for_each takes an lvalue reference to container, so if you don't want the caller to own
// the container
seastar::future<> do_for_all(std::vector<int> numbers) {
  // so we use do_with to guarantee lifetime

  // do_with takes the rvalue ref from std::move and owns the result
  return seastar::do_with(std::move(numbers), [] (std::vector<int> &numbers) {
    // then do_for_each takes an lvalue reference from the do_with owned numbers vector
    return seastar::do_for_each(numbers, [] (int number) {
      return do_something(number);
    });
  });
}

// parallel for each does not guarantee order of iteration completion (all queued simultaneously)

seastar::future<> flush_all_files(seastar::lw_shared_ptr<std::vector<seastar::file>> files) {
  return seastar::parallel_for_each(files, [] (seastar::file f) {
    // file::flush() returns a future
    return f.flush();
  });
}

// note that it's possible to limit loop parallelism (too much is costly)
// generally a good idea to base the maximum on actual capabilities of the host system.
// e.g. parallel execution units, I/O channels, etc.

seastar::future<> flush_all_files(seastar::lw_shared_ptr<std::vector<seastar::file>> files,
				  size_t max_concurrency) {
  return seastar::max_concurrent_for_each(files, max_concurrent, [] (seastar::file f) {
    return f.flush();
  });
}

