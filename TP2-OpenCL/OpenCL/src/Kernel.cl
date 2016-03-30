#define MSG_LEN              7U
#define ALPHABET_LEN        26U
#define CUBICRT_NUM_THREADS (2 << 3)
#define NUM_THREADS         (CUBICRT_NUM_THREADS * CUBICRT_NUM_THREADS * CUBICRT_NUM_THREADS)

unsigned long power(unsigned int i_Base, unsigned int i_Exp)
{
    unsigned long answer = 1;
    for (int i = 0; i < i_Exp; ++i)
    {
        answer *= i_Base;
    }
    return answer;
}

void add(char * io_ToEncode, int i_Key)
{
    for (int i = 0; i < MSG_LEN; ++i)
    {
        io_ToEncode[i] += i_Key * i;
    }
}

void xor(char * io_ToEncode, char const * i_Key)
{
    for (int i = 0; i < MSG_LEN; ++i)
    {
        io_ToEncode[i] ^= i_Key[i];
    }
}

void shift(char * io_ToEncode, int i_Offset)
{
    char temp[2];
    for (int i = 0; i < i_Offset; ++i)
    {
        temp[i] = io_ToEncode[MSG_LEN - 1 - i];
    }
    for (int i = MSG_LEN - 1; i >= i_Offset; --i)
    {
        io_ToEncode[i] = io_ToEncode[i - i_Offset];
    }
    for (int i = 0; i < i_Offset; ++i)
    {
        io_ToEncode[i] = temp[i_Offset - i - 1];
    }
}

void swap(char * io_ToEncode, int i_Offset)
{
    for (int i = 0U; i + i_Offset < MSG_LEN; ++i)
    {
        char temp = io_ToEncode[i];
        io_ToEncode[i] = io_ToEncode[i + i_Offset];
        io_ToEncode[i + i_Offset] = temp;
    }
}

int getKey(char const * i_ToEncode)
{
    unsigned int temp = 0;
    for (int i = 0; i < MSG_LEN; ++i)
    {
        temp += (unsigned int) i_ToEncode[i];
    }
    return abs(temp % 4 + 1);
}

void encode(char const * i_ToEncode, char * o_Encoded)
{
    for (int i = 0; i < MSG_LEN; ++i)
    {
        o_Encoded[i] = i_ToEncode[i];
    }

    for (int i = 0; i < 3; ++i)
    {
        int key = getKey(o_Encoded);
        shift(o_Encoded, key / 2);
        add  (o_Encoded, key);
        swap (o_Encoded, key);
        xor  (o_Encoded, i_ToEncode);
    }
}

__kernel void main(__constant char const * i_EncodedMsg, __global char * o_Solution)
{
    // Obtain global thread ID
    unsigned int threadID = get_global_id(0) + get_global_id(1) * CUBICRT_NUM_THREADS + get_global_id(2) * CUBICRT_NUM_THREADS * CUBICRT_NUM_THREADS;

    // Compute how many message threads will encode
    unsigned long numPossibilities = power(ALPHABET_LEN, MSG_LEN);
    unsigned long numTries = numPossibilities / NUM_THREADS;

    // Compute the first message number to encode
    unsigned long firstMsgNum = threadID * numTries;

    // Last thread must also do remaining possibilities
    if (threadID == NUM_THREADS - 1)
    {
        numTries += numPossibilities % NUM_THREADS;
    }

    // Contains the messages that we try to encode
    char attempt[2 * MSG_LEN];

    // Compute the first message to try
    for (int i = 0; i < MSG_LEN; ++i)
    {
        unsigned long factor = power(ALPHABET_LEN, MSG_LEN - i - 1);
        char num = (char)(firstMsgNum / factor);
        attempt[i] = num + 'a';
        firstMsgNum -= num * factor;
    }

    // Contains encoded messages
    //char encodedAttempt[MSG_LEN];

    // Try all possible solutions
    unsigned long currTry = 0;
    while (++currTry < numTries)
    {
        // Pass the possible solution in the encoder
        encode(attempt, attempt + MSG_LEN);

        // Check the result is what we are looking for
        bool isValidSolution = true;
        for (int i = 0; i < MSG_LEN; ++i)
        {
            if (attempt[i + MSG_LEN] != i_EncodedMsg[i])
            {
                isValidSolution = false;
                break;
            }
        }
        if (isValidSolution)
        {
            // Copy solution to output buffer
            for (int i = 0; i < MSG_LEN; ++i)
            {
                o_Solution[i] = attempt[i];
            }
            return;
        }
        // Get next possible solution
        int i = MSG_LEN - 1;
        while (++attempt[i] == 'z' + 1)
        {
            attempt[i] = 'a';
            --i;
        }
    }
}
