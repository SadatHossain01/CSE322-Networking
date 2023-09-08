#include <iostream>
#include <string>
#include <bitset>
#include <cassert>
#include <vector>
#include <cstdlib>
#include <windows.h>

int m;
double p;
std::string data_string, gen_polynomial;

#define GREEN_COLOR_CODE 2
#define CYAN_COLOR_CODE 3
#define RED_COLOR_CODE 4
#define WHITE_COLOR_CODE 7

std::string divide(std::string dividend, std::string divisor)
{
}

int main()
{
    HANDLE color = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(color, WHITE_COLOR_CODE);

    std::cout << "enter data string: ";
    std::getline(std::cin, data_string);
    std::cout << "enter number of data bytes in a row <m>: ";
    std::cin >> m;
    std::cout << "enter probability <p>: ";
    std::cin >> p;
    std::cout << "enter generator polynomial: ";
    std::cin >> gen_polynomial;

    std::cout << std::endl
              << std::endl;

    // padding
    while (data_string.size() % m != 0)
        data_string += '~';
    std::cout << "data string after padding: " << data_string << std::endl
              << std::endl;

    const int sz = data_string.size();

    // finding no. of check bits
    int low = 0, high = 1000, n_check_bits = 0;
    while (low <= high)
    {
        int mid = (low + high) / 2;
        if (m * 8 + mid + 1 <= (1LL << mid))
        {
            n_check_bits = mid;
            high = mid - 1;
        }
        else
            low = mid + 1;
    }
    // std::cerr << "no. of check bits: " << n_check_bits << std::endl;

    // printing data block (ascii code of m characters per row)
    assert(sz % m == 0);
    int n_rows = sz / m;
    std::string total_string(8 * sz, '0');

    for (int i = 0; i < sz; i++)
    {
        std::bitset<8> ascii(data_string[i]);
        total_string.replace(8 * i, 8, ascii.to_string());
    }

    std::cout << "data block (ascii code of m characters per row): " << std::endl;
    for (int i = 0; i < n_rows; i++)
        std::cout << total_string.substr(8 * i * m, 8 * m) << std::endl;
    std::cout << std::endl;

    std::vector<std::vector<short>> data_block(n_rows);
    int total_string_idx = 0;
    for (int i = 0; i < n_rows; i++)
    {
        data_block[i].resize(8 * m + n_check_bits + 1, 0); // +1 to make it 1-indexed
        int next_idx = 1;
        for (int j = total_string_idx; j < total_string_idx + 8 * m; j++)
        {
            // next_idx should not be a power of 2, as those positions are reserved for check bits
            while ((next_idx & (next_idx - 1)) == 0)
                next_idx++;
            // std::cerr << total_string[j] - '0';
            // std::cerr << "Placing " << total_string[j] - '0' << " in " << next_idx << std::endl;
            data_block[i][next_idx] = total_string[j] - '0';

            // now check to which bits data_block[i][idx_here] contributes
            // std::cerr << "Checking for position " << idx_here << std::endl;
            for (int bit = 0; bit < n_check_bits; bit++)
            {
                if (!(next_idx & (1 << bit)))
                    continue;
                // std::cerr << "contributes to " << (1 << bit) << std::endl;
                data_block[i][1 << bit] ^= data_block[i][next_idx];
            }

            next_idx++;
        }
        // std::cerr << std::endl;
        total_string_idx += 8 * m;
    }

    // now show the data blocks row by row
    std::cout << "data blocks after adding check bits: " << std::endl;
    for (int row = 0; row < n_rows; row++)
    {
        for (int col = 1; col < data_block[row].size(); col++)
        {
            if (col & (col - 1)) // show it in default white
                SetConsoleTextAttribute(color, WHITE_COLOR_CODE);
            else // show it in green color
                SetConsoleTextAttribute(color, GREEN_COLOR_CODE);

            std::cout << data_block[row][col];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    SetConsoleTextAttribute(color, WHITE_COLOR_CODE);

    std::cout << "data blocks after column-wise serialization: " << std::endl;
    for (int col = 1; col < data_block[0].size(); col++)
    {
        for (int row = 0; row < data_block.size(); row++)
        {
            std::cout << data_block[row][col];
        }
    }
    std::cout << std::endl
              << std::endl;
}