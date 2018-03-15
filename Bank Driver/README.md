# Simulation of Bank Software

## The project is multi-threaded version of the bank's software; It runs safely in a multithreaded environment while maintaining an accaptable level of concurrency between threads.

The bank itself is subdivided into branches. Each branch has some number of accounts, and each account belongs to exactly one branch.
Tellers process requests against the bank. A request can be:  
*	a deposit or withdrawal from an account  
*	a transfer between two accounts  
*	a query for a branch's balance, or   
*	a query for the bank's balance.  

Transfers can cross branch boundaries; that is, we can request a transfer between any two accounts A and B, even if A and B belong to different branches.

The bank also contains a reporting system that performs the government-mandated tracking of transfers above a specified size and generates nightly reports.

### The test harness uses the Posix threads package (pthreads) to spawn and control the worker threads so you should use the pthread synchronization primitives.
