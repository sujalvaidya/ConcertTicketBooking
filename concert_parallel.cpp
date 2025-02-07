#include <iostream>
#include <pthread.h>
#include <queue>
#include <fstream>
#include <unistd.h>
#include <cstdlib>
#include <string>

#define MAX_BUFFER_SIZE 5

using namespace std;

int totalTickets;
int numAgents = 3;

int nextRequestId = 1;  // Used to ensure requests are processed in the order they arrived
bool finishedReading = false;

struct customer {
    int id;
    string name;
    int tickets;
};
typedef struct customer Customer;

// the size of this queue should be limited to MAX_BUFFER_SIZE
queue<Customer> customerRequests;

pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;  // Protects customerRequests queue
pthread_cond_t notEmptyCond = PTHREAD_COND_INITIALIZER;  // Signals that the queue is not empty
pthread_cond_t notFullCond  = PTHREAD_COND_INITIALIZER;  // Signals that the queue is not full

pthread_mutex_t orderMutex = PTHREAD_MUTEX_INITIALIZER;  // Protects order related variables and ensure inorder processing
pthread_cond_t orderCond = PTHREAD_COND_INITIALIZER;  // Signals when the next request can be processed

pthread_mutex_t printMutex = PTHREAD_MUTEX_INITIALIZER;  // Protects console output

// Consumer thread function to process ticket bookings
void* bookTicket(void* arg) {
    while (true) {
        pthread_mutex_lock(&queueMutex);

        // Wait until there is a request or the producer is done reading
        while (customerRequests.empty() && !finishedReading) {
            pthread_cond_wait(&notEmptyCond, &queueMutex);
        }

        // If no requests remain and file reading is finished, exit loop
        if (customerRequests.empty() && finishedReading) {
            pthread_mutex_unlock(&queueMutex);
            break;
        }

        Customer currentCustomer = customerRequests.front();
        customerRequests.pop();
        pthread_cond_signal(&notFullCond);  // Signal producer if the queue was full
        pthread_mutex_unlock(&queueMutex);  // Release queue lock

        pthread_mutex_lock(&orderMutex);  // Lock to enforce proper processing order

        // Wait until it's this request's turn based on its id
        while (currentCustomer.id != nextRequestId) {
            pthread_cond_wait(&orderCond, &orderMutex);
        }

        int ticketsRequested = currentCustomer.tickets;

        int allocatedTickets = 0;
        if (totalTickets >= ticketsRequested) {
            allocatedTickets = ticketsRequested;
            totalTickets -= ticketsRequested;
        } else if (totalTickets <= 0) {
            allocatedTickets = 0;
        } else {
            allocatedTickets = totalTickets;
            totalTickets = 0;
        }

        pthread_mutex_lock(&printMutex);
        cout << "Customer " << currentCustomer.name << " given " << allocatedTickets << " tickets\n";
        pthread_mutex_unlock(&printMutex);

        nextRequestId++;  // Allow the next request to be processed
        pthread_cond_broadcast(&orderCond);  // Wake up any waiting threads for order check
        pthread_mutex_unlock(&orderMutex);  // Release order lock
    }
    return NULL;
}

// Producer thread function to read customer requests from file
void* producer(void* arg) {
    ifstream file("sample_input.txt");
    if (!file) {
        pthread_mutex_lock(&printMutex);
        cerr << "Error: Could not open sample_input.txt\n";
        pthread_mutex_unlock(&printMutex);
        exit(1);
    }

    string name;

    file >> totalTickets;

    int tickets;

    int id = 1;

    while (file >> name >> tickets) {
        pthread_mutex_lock(&printMutex);
        cout << "Customer " << name << " requested " << tickets << " tickets\n";
        pthread_mutex_unlock(&printMutex);

        Customer cust;
        cust.id = id++;
        cust.name = name;
        cust.tickets = tickets;

        pthread_mutex_lock(&queueMutex);   // Lock the queue before adding a request

        // Wait if the queue is full
        while (customerRequests.size() >= MAX_BUFFER_SIZE) {
            pthread_cond_wait(&notFullCond, &queueMutex);
        }

        customerRequests.push(cust);  // Add the request to the queue
        pthread_cond_signal(&notEmptyCond);  // Signal that the queue now has data
        pthread_mutex_unlock(&queueMutex);  // Unlock the queue
    }
    file.close();

    pthread_mutex_lock(&queueMutex);
    finishedReading = true;
    pthread_cond_broadcast(&notEmptyCond);  // Wake up any consumer threads waiting for data
    pthread_mutex_unlock(&queueMutex);

    return NULL;
}

int main() {
    // Create threads for producer and booking agents (parallel implementation)
    pthread_t prodThread;
    pthread_t* agentThreads = new pthread_t[numAgents];

    // Create the producer thread
    if (pthread_create(&prodThread, NULL, producer, NULL) != 0) {
        pthread_mutex_lock(&printMutex);
        cerr << "Error creating producer thread.\n";
        pthread_mutex_unlock(&printMutex);
        exit(1);
    }

    // Create booking agent threads
    for (int i = 0; i < numAgents; i++) {
        if (pthread_create(&agentThreads[i], NULL, bookTicket, NULL) != 0) {
            pthread_mutex_lock(&printMutex);
            cerr << "Error creating booking agent thread " << i << ".\n";
            pthread_mutex_unlock(&printMutex);
            exit(1);
        }
    }

    pthread_join(prodThread, NULL);  // Wait for the producer thread to finish
    for (int i = 0; i < numAgents; i++) {
        pthread_join(agentThreads[i], NULL);  // Wait for all booking agent threads to finish
    }
    delete[] agentThreads;  // Clean up dynamically allocated thread array

    pthread_mutex_lock(&printMutex);
    cout << "Remaining tickets: " << totalTickets << "\n";
    pthread_mutex_unlock(&printMutex);

    // Destroy all mutexes and condition variables
    pthread_mutex_destroy(&queueMutex);
    pthread_cond_destroy(&notEmptyCond);
    pthread_cond_destroy(&notFullCond);
    pthread_mutex_destroy(&orderMutex);
    pthread_cond_destroy(&orderCond);
    pthread_mutex_destroy(&printMutex);

    return 0;
}

// Since threads are scheduled non-deterministically, output varies every execution but here is the output for one execution for reference:
//
// "C:\Users\sujal\Desktop\College\Sem 6\Heterogeneous Parallelism\Projects\Assignments\ConcertTicketBooking\cmake-build-debug\concert_parallel.exe"
// Customer A requested 4 tickets
// Customer B requested 5 tickets
// Customer A given 4 tickets
// Customer C requested 3 tickets
// Customer B given 5 tickets
// Customer D requested 4 tickets
// Customer E requested 1 tickets
// Customer F requested 9 tickets
// Customer G requested 5 tickets
// Customer H requested 15 tickets
// Customer I requested 3 tickets
// Customer J requested 2 tickets
// Customer C given 3 tickets
// Customer K requested 3 tickets
// Customer D given 4 tickets
// Customer E given 1 tickets
// Customer F given 9 tickets
// Customer G given 5 tickets
// Customer H given 15 tickets
// Customer I given 3 tickets
// Customer J given 1 tickets
// Customer K given 0 tickets
// Remaining tickets: 0
//
// Process finished with exit code 0
