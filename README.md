##Lockfree queue##

#Task/Goal#
Implement lockfree queue for many consumers and many readers using a garbage collection scheme based on freeing the memory in epoch cycles[1].

Implement classic Michael–Scott queue using tagged pointers to prevent ABA problem.

Compare their performance.

1. Keir Fraser: Practical Lock Freedom (p. 79)
https://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-579.pdf

##Interface##
All queues have  common interface:

`void enqueue(T&& value)`

`void enqueue(T& value)`

`std::experimental::optional<T> dequeue()`

##Description##

###my_queue###
Enqueue - lock-free, even wait-free enqueues, queue atomically exchanges value of "back" and put's there new value.
Dequeue - Isn't excactly lock-free, because it sets "front" to nullptr, which blocks dequeuing more elements and then sets it back to the right value. Fully lock-free version is commented below the actual one, but there is some error.

###mutex_queue###
Simple mutex queue, uses standard mutex from std and queue from std.

###ms_queue###
Enqueue operation links a node to the tail of a queue and tries to update
the tail pointer as described in Michael–Scott's publication.

Dequeue operation removes a node of a link-based structure.
Freeing the node takes place withing the operation.

###epoch_queue###
Enqueue operation links a node to the tail of a queue and tries to update
the tail pointer as described in Michael–Scott's publication.

As dequeue operation unlinks a node from a link–based structure, it moves it to
garbage associated with a current epoch.  Unliked nodes can still be referenced
by other concurrent operations. Only after two epochs pass can the unliked node
be freed.

##Comparison##
All lock-free queues are really close, they differ only in some details.
Benchmarked on Intel(R) Core(TM) i5-3230M.

###my_queue###
- enqueue is even wait-free, so really fast and non blocking adding of elements

###mutex_queue###
- really slow for more threads, but really, really fast to implement (so start with this one and implement better only if its a performance bottleneck.

###ms_queue###
- Scales really well with growing number of threads.
- Overall the fastest.

###epoch_queue###
- Synchronization involving the management of unlinked pointers proves to have some overhead.

