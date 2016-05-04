##Lockfree queue##

#Task/Goal#
Implement lockfree queue for many consumers and many readers using a garbage collection scheme based on freeing the memory in epoch cycles[1].

Implement classic Michael–Scott queue using <pointer, counter> structure to prevent ABA problem.

Compare their performance.

1. Keir Fraser: Practical Lock Freedom (p. 79)
https://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-579.pdf
