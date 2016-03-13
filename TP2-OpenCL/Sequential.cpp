#include "Encode.h"

int main()
{
    // The word the program looks for
    auto const SOLUTION = "ayoyoyo";

    // Generate the encoded message we want obtain after applying the encoding algorithm
    auto const ENCODED = encode(SOLUTION);

    // The string that will contain possible solutions
    std::string possibleSolution;
    
    // Fill string with letter 'a'
    possibleSolution.resize(ENCODED.length());
    for (auto i = 0U; i < ENCODED.length(); ++i)
    {
        possibleSolution[i] = 'a';
    }

    // Brute force attack
    uint64_t numTries = 0;
    while (true)
    {
        // Log every million tries
        if (++numTries % 1000000 == 0)
        {
            printf("%llu million tries \n", numTries / 1000000);
        }

        // Pass the possible solution in the encoder
        auto encoded = encode(possibleSolution);

        // Check the result is what we look for
        if (encoded == ENCODED)
        {
            printf("Solution: %s \n", possibleSolution.c_str());
            return EXIT_SUCCESS;
        }

        // Get next possible solution
        uint8_t i = 0;
        while (++possibleSolution[i] == 'z' + 1)
        {
            possibleSolution[i] = 'a';
            ++i;
            // If we fall back on the first solution
            if (i == ENCODED.length())
            {
                printf("ERROR: Could not find any solution");
                return EXIT_FAILURE;
            }
        }
    }
}