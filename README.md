[![Build Status](https://travis-ci.org/mbrossard/threadpool.svg?branch=master)](https://travis-ci.org/mbrossard/threadpool)

A simple C thread pool implementation
=====================================

Currently, the implementation:
 * Works with pthreads only, but API is intentionally opaque to allow
   other implementations (Windows for instance).
 * Starts all threads on creation of the thread pool.
 * Reserves one task for signaling the queue is full.
 * Stops and joins all worker threads on destroy.

Possible enhancements
=====================

The API contains addtional unused 'flags' parameters that would allow
some additional options:

 * Lazy creation of threads (easy)
 * Reduce number of threads automatically (hard)
 * Unlimited queue size (medium)
 * Kill worker threads on destroy (hard, dangerous)
 * Support Windows API (medium)
 * Reduce locking contention (medium/hard)
 
Update of Cmake Build Support
=====================
If your operating system installs [cmake](https://cmake.org/), then you could do as follows.   

- create a build dir, in current dir
```zsh
mkdir build
```

- enter into the build dir
```zsh
cd build
```

- then generate the makefile
```zsh
cmake ..
```

- then make in parallel
```zsh
make -j
```
