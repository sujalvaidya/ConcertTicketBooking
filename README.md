# UE22CS343BB1 Heterogeneous Parallelism

**Assignment 1**: A concert ticket booking system using pthreads

**Name**: Sujal Yatin Vaidya

**SRN**: PES1UG22EC299

**Email**: sujalyatin@gmail.com

## Changes made to achieve parallelism

1. Producer-Consumer Model:
   - A producer thread reads customer requests from the input file and adds them to a shared queue with a limited size (`MAX_BUFFER_SIZE`).
   - Multiple consumer threads (booking agents) take requests from this queue and process them.

2. Request Ordering:
   - Each customer request is assigned a unique ID to maintain the order of processing.
   - An additional mutex (`orderMutex`) and condition variable (`orderCond`) ensure that requests are handled in the order they were received, even though multiple threads are processing them.

3. Synchronization:
   - Several mutexes and condition variables have been added to safely manage shared resources and to handle cases when the queue is full or empty.

4. Ticket Allocation:
   - The ticket allocation logic remains similar: if a customer’s request exceeds the remaining tickets, the customer gets whatever is left (or zero if tickets are exhausted).

## A concert ticket booking system using pthreads
A concert ticket booking system receives booking requests from multiple customers. In the real world scenario, if we don’t handle requests efficiently, the system might crash or slow down due to overwhelming demand. 

You will simulate the concert ticket booking system using multiple customer requests and multiple booking agents (pthreads) running in parallel.
- Customers request a certain number of tickets
- Booking agents (threads) allocate tickets based on availability
- The total ticket count must stay consistent and be synchronized across the booking agents
- You must queue the customer requests in a queue, whose size is limited by the constant MAX_BUFFER_SIZE (set to 5)
- If 0 < `number of tickets remaining` < `number of tickets requested by a customer`, the customer is given `number of tickets remaining` tickets
- If the tickets are exhausted (`number of tickets remaining` = 0), all the customers after that will be allotted 0 tickets

A serial implementation of the system is provided in `concert_serial.cpp`. Your job is to parallelize this and write it in another file `concert_parallel.cpp`. Feel free to write the parallel code from scratch too!

This assignment will be manually evaluated. Grading will be transparent and any loss of marks will be explained.

### Input format
The input has the first line as the total tickets available for the show. Followed by this will be `n` lines with the format <customer_name><space><no_of_tickets_requested>

A sample input is
```
10
A 4 
B 2 
C 1
D 2
```

### Output format
The output format is a bunch of print statements for each customer, and after all customer requests are handled, the number of tickets remaining is printed.
```
Customer <name> requested <number_of_tickets_requested> tickets
Customer <name> given <number_of_tickets_allotted> tickets
...
Remaining tickets: <number_of_tickets_remaining>
```

Example output for the above sample input:
```
Customer 1 requested 4 tickets
Customer 2 requested 2 tickets
Customer 1 given 4 tickets
Customer 3 requested 1 tickets
Customer 2 given 2 tickets
Customer 3 given 1 tickets
Customer 4 requested 2 tickets
Customer 4 given 2 tickets
Remaining tickets: 1
```
