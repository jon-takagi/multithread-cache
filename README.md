Jon Takagi and Eli Poppele

## Multithreading!
For this part of the project, we used essentially the same code as last week, only now we have removed the driver class and replaced it with a program in `thread_test.cc` to run our multithreaded benchmark and record values. Since we already had an asynchronous server, the only changes we needed to make were those in the new thread test program, as well as some mutex locks in the server. In general the implementation for this part of the project was fairly minimal, and we spent most of our time debugging (currently mostly sorted out now).

### Multi-Client Benchmarking
For the client side, we simply re-wrote our driver class as a single test program and added multiple threads to it. Each thread has its own client object, although only the first one will warm the cache. Each thread then calls `do_nreq_requests` and returns the results in a promise object, which are then collected and printed into an output file to give the latencies and total throughput.

### Multi-threaded Server
As noted above, the server itself required no changes other than a modification to the default number of threads. The only changes were in the `request_processor.hh` file, where we added a mutex lock onto all of the operations involving the server cache. Initially, this was a shared read/write lock, so that get and space_used requests could be going at the same time as other read-only operations, while set, delete, and reset all needed a unique write-access lock. However, this resulted in the server locking up, likely with most threads waiting on access to a write lock while two or more others calling get resulted in a read lock always being in use, and thus preventing any write-access operations. However, it seems likely that another issue was also occurring then, as it would reason that eventually the threads doing read operations would either finish 1mil requests or get a write-access operation, both of which would release the read-access lock, and thus the starved threads should eventually get access to the necessary locks. This did not seem to be the case, as the server would hang indefinitely, but in any case, the problem has not occurred since we switched all mutex locks to unique (no more read-only).

### Results
numbers go here
