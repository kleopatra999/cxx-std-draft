## A guide to C++ allocators

This repository contains a collection of documents that describe the allocator
concept in the standard library of C++11 and beyond. The main guide covers the
following topics.

* Allocator traits
* Statefulness
* Fancy pointers
* Allocator propagation in breadth (container copy, POC{CA,MA,S}) and depth (`scoped_allocator_adaptor`)

Start reading with [the main guide](https://rawgit.com/google/cxx-std-draft/allocator-paper/allocator_user_guide.html).

Furthermore, there are several worked-out end-to-end examples:

* [Using allocators: A simple arena allocator](https://rawgit.com/google/cxx-std-draft/allocator-paper/allocator_example_usage.html)
* [Writing containers: A simple dynamic container](https://rawgit.com/google/cxx-std-draft/allocator-paper/allocator_example_usage.html)
* [Writing fancy pointers: A simple offset pointer](https://rawgit.com/google/cxx-std-draft/allocator-paper/allocator_example_offset_pointer.html)

The code for the end-to-end examples is available separately in the `example_code` directory.
