# Cam and Burke Lab 6

# Serial Monitor All Tests Passing
Low priority thread waiting for semaphore
Low priority thread started
High priority thread waiting for semaphore
Middle priority thread started
Low priority thread finished
High priority thread started
High priority thread finished
Low Mutex Waiting
Low Mutex Started
Mutex High Waiting
Low Mutex Finished
Mutex High Start
Mutex High Finished
Mutex Middle Started
Start tests
Log arr[0]: 1, expected: 1
Log arr[1]: 2, expected: 2
Log arr[2]: 1, expected: 1
Log arr[3]: 3, expected: 3
/home/burkedambly/adv_emb/lab6/test/test.c:345:test_priority_inversion:PASS
Mutex Log arr[0]: 1, expected: 1
Mutex Log arr[1]: 3, expected: 3
Mutex Log arr[2]: 2, expected: 2
/home/burkedambly/adv_emb/lab6/test/test.c:346:test_priority_inversion_mutex:PASS
Task 1 Runtime: 499999
Task 2 Runtime: 499907
/home/burkedambly/adv_emb/lab6/test/test.c:348:both_busy_busy:PASS
Task 1 Runtime: 505398
Task 2 Runtime: 493560
/home/burkedambly/adv_emb/lab6/test/test.c:349:both_busy_yield:PASS
Task 1 Runtime: 996761
Task 2 Runtime: 2061
/home/burkedambly/adv_emb/lab6/test/test.c:350:t1_busy_t2_yield:PASS
Task 1 Runtime: 1099881
Task 2 Runtime: 0
/home/burkedambly/adv_emb/lab6/test/test.c:352:busy_busy_different_priority_high_first:PASS
Task 1 Runtime: 0
Task 2 Runtime: 999675
/home/burkedambly/adv_emb/lab6/test/test.c:353:busy_busy_different_priority_low_first:PASS
Task 1 Runtime: 0
Task 2 Runtime: 999333
/home/burkedambly/adv_emb/lab6/test/test.c:354:busy_yield_different_priority:PASS
All done!

-----------------------
8 Tests 0 Failures 0 Ignored 