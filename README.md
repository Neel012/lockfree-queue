##Lockfree queue##

Here we present two implementations of lockfree queues.
They differ based on dealing with the ABA problem.
Epoch-queue implemetation employs an epoch based memory reclamation to prevent unsafe  recycling addresses of memory addresses.
Michael–Scott queue implemetation uses <pointer, counter> structure to differenciate between different objects with the same address.
