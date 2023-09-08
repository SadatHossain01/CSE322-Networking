#include <iostream>
#include <string>
#include <bitset>
#include <cassert>
#include <vector>
#include <cstdlib>
#include <windows.h>
#include <ctime>

int m;
double p;
std::string data_string, gen_polynomial;

#define GREEN_COLOR_CODE 2
#define CYAN_COLOR_CODE 3
#define RED_COLOR_CODE 4
#define WHITE_COLOR_CODE 7

void trim(std::string &s)
{
    // trim initial zeroes
    int idx = 0;
    while (idx < s.size() && s[idx] == '0')
        idx++;
    s = s.substr(idx, s.size() - idx);
}

// returns {quotient, remainder}
std::pair<std::string, std::string> divide(std::string dividend, std::string divisor)
{
    // std::cerr << "After trimming: " << dividend << " " << divisor << std::endl;

    if (dividend.size() < divisor.size())
        return {"0", dividend};

    std::string quotient = "";
    std::string cur_dividend = dividend.substr(0, divisor.size() - 1);
    int dividend_next_idx = divisor.size() - 1;

    while (dividend_next_idx < dividend.size())
    {
        cur_dividend += dividend[dividend_next_idx++];
        assert(divisor.size() == cur_dividend.size());

        std::string subtrahend; // divisor or 0
        if (cur_dividend.front() == '0')
        {
            quotient += '0';
            subtrahend = std::string(divisor.size(), '0');
        }
        else
        {
            quotient += '1';
            subtrahend = divisor;
        }
        // std::cerr << "cur_dividend: " << cur_dividend << " subtrahend: " << subtrahend << std::endl;

        assert(subtrahend.size() == cur_dividend.size());
        for (int i = 0; i < cur_dividend.size(); i++)
            if (cur_dividend[i] == subtrahend[i])
                cur_dividend[i] = '0';
            else
                cur_dividend[i] = '1';
        // drop the MSB from cur_dividend
        cur_dividend = cur_dividend.substr(1, cur_dividend.size() - 1);
    }

    return {quotient, cur_dividend};
}

int main()
{
    // srand(time(NULL));
    srand(0);
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

    std::vector<std::string> data_block(n_rows);
    int total_string_idx = 0;
    for (int i = 0; i < n_rows; i++)
    {
        data_block[i].resize(8 * m + n_check_bits + 1, '0'); // +1 to make it 1-indexed
        int next_idx = 1;
        for (int j = total_string_idx; j < total_string_idx + 8 * m; j++)
        {
            // next_idx should not be a power of 2, as those positions are reserved for check bits
            while ((next_idx & (next_idx - 1)) == 0)
                next_idx++;
            // std::cerr << total_string[j] - '0';
            // std::cerr << "Placing " << total_string[j] - '0' << " in " << next_idx << std::endl;
            data_block[i][next_idx] = total_string[j];

            // now check to which bits data_block[i][idx_here] contributes
            // std::cerr << "Checking for position " << idx_here << std::endl;
            for (int bit = 0; bit < n_check_bits; bit++)
            {
                if (!(next_idx & (1 << bit)))
                    continue;
                // std::cerr << "contributes to " << (1 << bit) << std::endl;
                if (data_block[i][next_idx] == data_block[i][1 << bit])
                    data_block[i][1 << bit] = '0';
                else
                    data_block[i][1 << bit] = '1';
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
    SetConsoleTextAttribute(color, WHITE_COLOR_CODE);
    std::cout << std::endl;

    std::string sent_frame = "";
    std::cout << "data bits after column-wise serialization: " << std::endl;
    for (int col = 1; col < data_block[0].size(); col++)
    {
        for (int row = 0; row < data_block.size(); row++)
        {
            sent_frame += data_block[row][col];
            std::cout << data_block[row][col];
        }
    }
    std::cout << std::endl
              << std::endl;

    sent_frame += std::string(gen_polynomial.size() - 1, '0');
    std::pair<std::string, std::string> result = divide(sent_frame, gen_polynomial);
    // minus the remainder from the sent frame
    sent_frame = sent_frame.substr(0, sent_frame.size() - result.second.size());
    sent_frame += result.second;

    std::cout << "data bits after appending CRC checksum (sent frame): " << std::endl;
    std::cout << sent_frame.substr(0, sent_frame.size() - result.second.size());
    SetConsoleTextAttribute(color, CYAN_COLOR_CODE);
    std::cout << result.second << std::endl
              << std::endl;
    SetConsoleTextAttribute(color, WHITE_COLOR_CODE);

    // Received frame
    std::string received_frame = sent_frame;
    // toggle each bit with probability p
    std::vector<boolean> toggled(received_frame.size(), false);
    std::cout << "received frame: " << std::endl;
    for (int i = 0; i < received_frame.size(); i++)
    {
        double r = (double)rand() / RAND_MAX;
        if (r < p)
        {
            SetConsoleTextAttribute(color, RED_COLOR_CODE);
            received_frame[i] = (received_frame[i] == '0' ? '1' : '0');
            toggled[i] = true;
        }
        else
            SetConsoleTextAttribute(color, WHITE_COLOR_CODE);
        std::cout << received_frame[i];
    }
    SetConsoleTextAttribute(color, WHITE_COLOR_CODE);
    std::cout << std::endl
              << std::endl;

    // Verifying CRC checksum
    std::pair<std::string, std::string> result2 = divide(received_frame, gen_polynomial);
    std::cout << "result of CRC checksum matching: ";
    trim(result2.second);
    if (result2.second.size() == 0)
        std::cout << "no error detected" << std::endl;
    else
        std::cout << "error detected" << std::endl;

    std::cout << "data block after removing CRC checksum bits: " << std::endl;
}