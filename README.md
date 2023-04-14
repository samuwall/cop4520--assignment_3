# Samu Wallace - COP4520, Assignment 3

## Problem 1

The minotaur's original approach is more akin to a non-blocking synchronization implementation of a concurrent linked list (one which does not use locks). The problem arises in that the servants did not properly utilize the complex atomic operations and compareAndSet calls in order to eliminate race conditions.

My implementation uses a simpler, fine-grained synchronization approach. In this vein, the individual operations which make up a linked list are locked and unlocked as threads use them. Correctness is proven in that each thread/servant prints their number of thank yous and removes, which we can validate are perfectly consistent across all threads. Efficiency is ensured through the fine-grained approach, which allows for much greater performance than a coarse-grained approach. The average runtime is ~0.22s. Still, much greater performance can be achieved with lazy synchronization and non-blocking synchronization, though I felt these approaches were overkill for this application.

## Problem 2

This program spawns 8 sensors (threads), and a report thread. The temperatures are retrieved carefully from the shared memory through the use of std::condition_variable. The reporting thread waits to be notified that the shared memory is available, then checks that the number of readings is >= the expected amount for the reporting interval. If so, the reporting thread goes ahead and prints the temperature findings as expected.

The correctness is proven through the hourly (well, in my implementation it is able to be changed to by the minute reporting) report, which shows the temperatures you would expect from 8 sensors concurrently recording 60 times per report. The implementation is efficient in that the critical section is quite short and simple, so every sensor should get to do their reading almost exactly when needed. Temperature is not going to change over a span of microseconds anyway... Runtime is of course exactly the specified reporting interval.

**Note:** sample outputs provided in outputs folder.
