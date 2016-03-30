#include "../Common/Encode.h"
#include "../Common/Time.h"
#include <algorithm>
#include <condition_variable>
#include <thread>

// The word the program looks for
auto const SOLUTION = "jeremy";

// Encoded message we want to obtain after applying the encoding algorithm on possible solutions
auto const ENCODED = encode(SOLUTION);

// Length of the word to decode
auto const LENGTH = ENCODED.length();

// Number of worker threads performing a brute-force attack on the encoding algorithm
auto const NUM_WORKERS = 13U;

// Array of solutions found (one entry per worker thread)
std::string g_Solutions[NUM_WORKERS];

// Condition after which the manager thread waits before exiting the program
std::condition_variable g_SolutionIsFound;

// Code executed by each worker thread (or the main thread if NUM_WORKERS is one)
void worker(char i_FirstLetter, char i_LastLetter, uint16_t i_ID)
{
    // Build first possible solution
    std::string possibleSolution;
    possibleSolution.resize(LENGTH);
    for (auto i = 0U; i < LENGTH; ++i)
    {
        possibleSolution[i] = i_FirstLetter;
    }

    // Compute the maximum number of tries possible
    auto maxNumTries = static_cast<unsigned long long>(i_LastLetter - i_FirstLetter + 1);
    for (auto i = 0U; i < LENGTH - 1; ++i)
    {
        maxNumTries *= 26;
    }

    // Number of tries made so far
    auto numTries = 0LLU;

    // Exit when all possible solutions have been tried
    while (++numTries < maxNumTries)
    {
        // Log every million tries
        if (numTries % 1000000 == 0)
        {
            printf("Worker thread %d: %llu/%llu million tries\n",
                i_ID, numTries / 1000000, maxNumTries / 1000000);
        }

        // Pass the possible solution in the encoder
        auto encoded = encode(possibleSolution);

        // Check the result is what we look for
        if (encoded == ENCODED)
        {
            printf("Worker thread %d: FOUND SOLUTION \"%s\"\n", i_ID, possibleSolution.c_str());
            g_Solutions[i_ID] = possibleSolution;
            g_SolutionIsFound.notify_one();
            return;
        }

        // Get next possible solution
        auto i = LENGTH - 1;
        while (++possibleSolution[i] == 'z' + 1)
        {
            possibleSolution[i] = 'a';
            --i;
        }
    }
    printf("Worker thread %d: Could not find any solution\n", i_ID);
}

int main()
{
	// Initial time
	auto start = std::chrono::steady_clock::now();

    // Run a sequential program if number of worker is 1
    if (NUM_WORKERS == 1)
    {
        worker('a', 'z', 0);
    }
    else
    {
        // Cannot have more than 26 workers (one per first letter of the solution)
        if (NUM_WORKERS > 26)
        {
            printf("Manager thread: Current implementation does not \
                support more than 26 workers. Ignore extra threads.\n");
        }
        auto actualNumWorkers = std::min(static_cast<int>(NUM_WORKERS), 26);

        // Computes how many letters each thread will have to search through
        char numLettersPerThread = 26 / actualNumWorkers;

        // Start worker threads
        for (auto i = 0; i < actualNumWorkers; ++i)
        {
            char firstLetter = 'a' +  i      * numLettersPerThread;
            char lastLetter  = i == actualNumWorkers - 1 ? 'z' :
                              'a' + (i + 1) * numLettersPerThread - 1;
            std::thread(worker, firstLetter, lastLetter, i).detach();
        }

        // Wait for a worker thread to signal that a solution was found
        std::mutex mutex;
        std::unique_lock<std::mutex> lock(mutex);
        g_SolutionIsFound.wait(lock);
    }

	// Print time
	using namespace std::chrono;
	printf("Wall Time : %d ms\n", duration_cast<milliseconds>(steady_clock::now() - start).count());
}
