#include "Encode.h"

void add(std::string & io_PW, uint8_t i_Key)
{
    auto len = io_PW.length();
    for (auto i = 0U; i < len; ++i)
    {
        io_PW[i] += i_Key * i;
    }
}

void xorx(std::string & io_PW, std::string const & i_Key)
{
    auto len = io_PW.length();
    for (auto i = 0U; i < len; ++i)
    {
        io_PW[i] ^= i_Key[i];
    }
}

void shift(std::string & io_PW, uint8_t i_Offset)
{
    char temp[2];
    auto max = io_PW.length() - 1;
    for (auto i = 0U; i < i_Offset; ++i)
    {
        temp[i] = io_PW[max - i];
    }
    for (int i = max; i >= static_cast<int>(i_Offset); --i)
    {
        io_PW[i] = io_PW[i - i_Offset];
    }
    for (auto i = 0U; i < i_Offset; ++i)
    {
        io_PW[i] = temp[i_Offset - i - 1];
    }
}

void swap(std::string & io_PW, uint8_t i_Offset)
{
    auto len = io_PW.length();
    for (auto i = 0U; i + i_Offset < len; ++i)
    {
        auto temp = io_PW[i];
        io_PW[i] = io_PW[i + i_Offset];
        io_PW[i + i_Offset] = temp;
    }
}

uint8_t getKey(std::string const & i_PW)
{
    auto temp = 0U;
    auto len = i_PW.length();
    for (auto i = 0U; i < len; ++i)
    {
        temp += static_cast<unsigned int>(i_PW[i]);
    }
    return abs(static_cast<int32_t>(temp % 4 + 1));
}

std::string encode(std::string const & i_PW)
{
    auto encoded = i_PW;

    for (auto i = 0U; i < 3; ++i)
    {
        auto key = getKey(encoded);
        shift(encoded, key / 2);
        add  (encoded, key);
        swap (encoded, key);
        xorx (encoded, i_PW);
    }

    return encoded;
}
