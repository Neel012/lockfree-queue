##Lockfree queue##

#Task/Goal#
Implement lockfree queue for many consumers and many readers using a garbage collection scheme based on freeing the memory in epoch cycles[1].

Implement classic Michael–Scott queue using tagged pointers to prevent ABA problem.

Compare their performance.

1. Keir Fraser: Practical Lock Freedom (p. 79)
https://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-579.pdf


##Description##

#My queue#
Enqueue - lock-free, even wait-free enqueues, queue atomically exchanges value of "back" and put's there new value.
Dequeue - Isn't excactly lock-free, because it sets "front" to nullptr, which blocks dequeuing more elements and then sets it back to the right value. Fully lock-free version is commented below the actual one, but there is some error.

#Mutex queue#
Simple mutex queue, uses standard mutex from std and queue from std.

