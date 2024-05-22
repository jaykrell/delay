This is a test of a race condition in 14.41pre.

https://developercommunity.visualstudio.com/t/delayload-race-crash-regression-in-1441/10661431

The test has two pages of delayloads.

It uses the trap flag to control the trace.

First it calls through the first page, stepping
to find the instruction that writes to the IAT.
i.e. reading the IAT before/after each instruction.

Once the first thread has written the IAT, it then waits infinite.

The second thread proceeds to call through the second delay IAT page and access violates.