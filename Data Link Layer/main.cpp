#include <iostream>
#include <string>
#include <bitset>
#include <cassert>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <map>

#ifdef _WIN32
#include <windows.h>
#endif

int m;
double p;
std::string data_string, generator_polynomial;

std::map<std::string, int> win_color_code{{"green", 2}, {"cyan", 3}, {"red", 4}, {"white", 7}};
std::map<std::string, int> unix_color_code{{"green", 32}, {"cyan", 36}, {"red", 31}, {"white", 37}};

template <typename T>
inline void print_colored(const T &s, const std::string &color)
{
    if (color == "white")
    {
        std::cout << s;
        return;
    }
#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    assert(unix_color_code.count(color));
    std::cout << "\033[1;" << unix_color_code[color] << "m" << s << "\033[0m";
#else
    assert(win_color_code.count(color));
    static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, win_color_code[color]);
    std::cout << s;
    SetConsoleTextAttribute(hConsole, win_color_code["white"]);
#endif
}

void trim(std::string &s)
{
    // trim initial zeroes
    int idx = 0;
    while (idx < s.size() && s[idx] == '0')
        idx++;
    s = (idx == s.size() ? "" : s.substr(idx, s.size() - idx));
}

// returns {quotient, remainder}
std::pair<std::string, std::string> divide(std::string dividend, std::string divisor)
{
    if (dividend.size() < divisor.size())
        return {"0", dividend};

    std::string quotient = "";
    std::string cur_dividend = dividend.substr(0, divisor.size() - 1);
    int dividend_next_idx = divisor.size() - 1;

    while (dividend_next_idx < dividend.size())
    {
        cur_dividend += dividend[dividend_next_idx++];
        assert(divisor.size() == cur_dividend.size());

        std::string subtrahend; // divisor itself or 0
        if (cur_dividend.front() == '0')
            quotient += '0', subtrahend = std::string(divisor.size(), '0');
        else
            quotient += '1', subtrahend = divisor;
        // std::cerr << "cur_dividend: " << cur_dividend << " subtrahend: " << subtrahend << std::endl;

        assert(subtrahend.size() == cur_dividend.size());
        for (int i = 0; i < cur_dividend.size(); i++)
            cur_dividend[i] = (cur_dividend[i] == subtrahend[i] ? '0' : '1');
        // drop the MSB from cur_dividend
        cur_dividend = cur_dividend.substr(1, cur_dividend.size() - 1);
    }

    return {quotient, cur_dividend};
}

std::string subtract(const std::string &a, const std::string &b)
{
    std::string result(a);
    const int sz_a = a.size(), sz_b = b.size();
    for (int i = 0; i < b.size(); i++)
        result[sz_a - i - 1] = (a[sz_a - i - 1] == b[sz_b - i - 1] ? '0' : '1');
    return result;
}

int main()
{
    // srand(time(NULL));
    srand(1);
    std::cout << "enter data string: ";
    std::getline(std::cin, data_string);
    std::cout << "enter number of data bytes in a row (m): ";
    std::cin >> m;
    std::cout << "enter probability (p): ";
    std::cin >> p;
    std::cout << "enter generator polynomial: ";
    std::cin >> generator_polynomial;

    trim(generator_polynomial);
    if (generator_polynomial.empty())
        std::cout << "Please enter a valid generator polynomial" << std::endl, exit(0);

    std::cout << std::endl
              << std::endl;

    // padding to make data_string.size() a multiple of m
    while (data_string.size() % m != 0)
        data_string += '~';
    std::cout << "data string after padding: " << data_string << std::endl
              << std::endl;

    const int data_string_size = data_string.size();
    assert(data_string_size % m == 0);
    const int n_rows = data_string_size / m; // n_rows is basically the number of data blocks

    // binary search for finding no. of check bits
    int low = 0, high = 60, n_check_bits = 0;
    while (low <= high)
    {
        int mid = (low + high) / 2;
        if (m * 8 + mid + 1 <= (1LL << mid))
            n_check_bits = mid, high = mid - 1;
        else
            low = mid + 1;
    }
    const int n_cols = 8 * m + n_check_bits; // no of bits transmitted per block: m*8 data bits + n_check_bits check bits
    // std::cerr << "no. of check bits: " << n_check_bits << std::endl;

    // printing data block (ascii code of m characters per row)
    std::string ascii_binary_string(8 * data_string_size, '0');

    for (int i = 0; i < data_string_size; i++)
    {
        std::bitset<8> ascii(data_string[i]);
        ascii_binary_string.replace(8 * i, 8, ascii.to_string());
    }

    std::cout << "data block (ascii code of m characters per row):" << std::endl;
    for (int i = 0; i < n_rows; i++)
        std::cout << ascii_binary_string.substr(8 * i * m, 8 * m) << std::endl;
    std::cout << std::endl;

    std::vector<std::string> sent_data_blocks(n_rows);
    int ascii_string_idx = 0;
    for (int i = 0; i < n_rows; i++)
    {
        std::string &current_block = sent_data_blocks[i];
        current_block.assign(n_cols + 1, '0'); // +1 to make it 1-indexed
        int next_idx = 1;
        for (int j = ascii_string_idx; j < ascii_string_idx + 8 * m; j++)
        {
            // next_idx should not be a power of 2, as those positions are reserved for check bits
            while (!(next_idx & (next_idx - 1)))
                next_idx++;
            current_block[next_idx] = ascii_binary_string[j];

            // now check to which bits current_block[idx_here] contributes
            // std::cerr << "Checking for position " << next_idx << std::endl;
            for (int bit = 0; bit < n_check_bits; bit++)
            {
                if (!(next_idx & (1 << bit)))
                    continue;
                // std::cerr << "contributes to " << (1 << bit) << std::endl;
                current_block[1 << bit] = (current_block[1 << bit] - '0') ^ (current_block[next_idx] - '0') + '0';
            }
            next_idx++;
        }
        // std::cerr << std::endl;
        ascii_string_idx += 8 * m; // go to next row/block
    }

    // now show the sent data blocks row by row
    std::cout << "data block after adding check bits:" << std::endl;
    for (int row = 0; row < n_rows; row++)
    {
        for (int col = 1; col <= n_cols; col++)
            print_colored<char>(sent_data_blocks[row][col], (col & (col - 1)) ? "white" : "green"); // col & (col - 1) is false iff col is a power of 2
        std::cout << std::endl;
    }
    std::cout << std::endl;

    std::string sent_frame = "";
    std::cout << "data bits after column-wise serialization:" << std::endl;
    for (int col = 1; col <= n_cols; col++)
    {
        for (int row = 0; row < n_rows; row++)
        {
            sent_frame += sent_data_blocks[row][col];
            std::cout << sent_data_blocks[row][col];
        }
    }
    std::cout << std::endl
              << std::endl;

    sent_frame += std::string(generator_polynomial.size() - 1, '0'); // augment the sent frame with zeroes
    std::string remainder = divide(sent_frame, generator_polynomial).second;

    // subtract the remainder from the sent frame
    sent_frame = subtract(sent_frame, remainder);

    std::cout << "data bits after appending CRC checksum (sent frame):" << std::endl;
    std::cout << sent_frame.substr(0, sent_frame.size() - remainder.size());
    print_colored<std::string>(remainder, "cyan");
    std::cout << std::endl
              << std::endl;

    // Received frame
    std::string received_frame = sent_frame;
    // toggle each bit with probability p
    std::vector<bool> toggled(received_frame.size(), false);
    std::cout << "received frame:" << std::endl;
    for (int i = 0; i < received_frame.size(); i++)
    {
        double r = (double)rand() / RAND_MAX;
        if (r < p)
        {
            received_frame[i] = (received_frame[i] == '0' ? '1' : '0');
            toggled[i] = true;
        }
        print_colored<char>(received_frame[i], toggled[i] ? "red" : "white");
    }
    std::cout << std::endl
              << std::endl;

    // Verifying CRC checksum
    std::string remainder2 = divide(received_frame, generator_polynomial).second;
    std::cout << "result of CRC checksum matching: ";
    trim(remainder2);
    std::cout << (remainder2.empty() ? "no error detected" : "error detected") << std::endl;
    std::cout << std::endl;

    // remove CRC checksum bits
    std::cout << "data block after removing CRC checksum bits:" << std::endl;
    received_frame.erase(received_frame.size() - (generator_polynomial.size() - 1), generator_polynomial.size() - 1);
    std::vector<std::string> received_data_blocks(n_rows);
    // std::cerr << received_frame << std::endl;
    for (int row = 0; row < n_rows; row++)
    {
        std::string &current_block = received_data_blocks[row];
        current_block.assign(n_cols + 1, '0'); // +1 to make it 1-indexed
        for (int received_string_idx = row, k = 1; received_string_idx < received_frame.size(); received_string_idx += n_rows, k++)
        {
            current_block[k] = received_frame[received_string_idx];
            print_colored<char>(received_frame[received_string_idx], toggled[received_string_idx] ? "red" : "white");
        }
        // std::cerr << "cur: " << current_block << " received_frame: " << received_data_blocks[row] << std::endl;
        std::cout << std::endl;
    }
    std::cout << std::endl;

    // remove check bits after correcting error in each row
    std::cout << "data block after removing check bits:" << std::endl;
    for (int row = 0; row < n_rows; row++)
    {
        std::string computed_check_bits(n_check_bits, '0'); // i-th computed check bit is stored at index 2^i at received_data_blocks[row]
        std::string &current_block = received_data_blocks[row];
        for (int col = 1; col <= n_cols; col++)
        {
            if (!(col & (col - 1)))
                continue;
            for (int bit = 0; bit < n_check_bits; bit++)
                if (col & (1 << bit))
                    computed_check_bits[bit] = (computed_check_bits[bit] - '0') ^ (current_block[col] - '0') + '0';
        }

        int total = 0;
        for (int bit = 0; bit < n_check_bits; bit++)
            if (computed_check_bits[bit] != current_block[1 << bit])
                total += (1 << bit);

        if (total <= n_cols) // correct 1-bit error
            current_block[total] = (current_block[total] == '0' ? '1' : '0');

        for (int col = 1; col <= n_cols; col++)
        {
            if (col & (col - 1))
                std::cout << current_block[col];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    // now decode the output frame
    std::string decoded_string = "";
    for (int row = 0; row < n_rows; row++)
    {
        std::string cur_row = "";
        for (int col = 1; col <= n_cols; col++)
            if (col & (col - 1))
                cur_row += received_data_blocks[row][col];
        // std::cerr << "decoded row: " << cur_row << std::endl;

        for (int j = 0; j < cur_row.size(); j += 8)
        {
            std::string cur_byte = cur_row.substr(j, 8);
            std::bitset<8> ascii(cur_byte);
            decoded_string += (char)ascii.to_ulong();
        }
    }
    std::cout << "output frame: " << decoded_string << std::endl
              << std::endl;
}
