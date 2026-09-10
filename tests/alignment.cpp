#include <iostream>
#include <type_traits>

// 00 -> long long, char, int can live here
// 01 -> char
// 02 -> char
// 03 -> char
// 04 -> char, int can live here
// 05 -> char
// 06 -> char
// 07 -> char
// 08 -> long long, char, int can live here

struct S {
    char a;       // 1 byte  --> offset 0
    ;             // padding --> offsets 1-3 (3 bytes)
    int i;        // 4 bytes --> offsets 4-7
    char b;       // 1 byte  --> offset 8
    ;             // padding --> offsets 9-11 (3 bytes)
    int j;        // 4 bytes --> offsets 12-15
    long long l;  // 8 bytes --> offsets 16-23
    char c;       // 1 byte  --> offset 24
    ;             // padding --> offsets 25-31 (7 bytes)
};
// Actual data: 1 + 4 + 1 + 4 + 8 + 1 = 19 bytes
// Padding:     3 + 3 + 7             = 13 bytes
// sizeof(S) = 19 + 13 = 32 bytes

// Now, by just reordering:
struct T {
    char a;       // 1 byte  --> offset 0
    char b;       // 1 byte  --> offset 1
    char c;       // 1 byte  --> offset 2
    ;             // padding --> offset 3 (1 byte)
    int i;        // 4 bytes --> offsets 4-7
    int j;        // 4 bytes --> offsets 8-11
    ;             // padding --> offset 12-15 (4 bytes)
    long long l;  // 8 bytes --> offsets 16-23
};
// Actual data: 1 + 4 + 1 + 4 + 8 + 1 = 19 bytes
// Padding:     1 + 4                 = 5 bytes
// sizeof(T) = 19 + 5 = 24 bytes

// Use the -Wpadded flag for g++ to view warnings about padding

int main() {

    // byte boundaries (limits)
    std::cout << std::alignment_of_v<int> << "\n";  // int (4)
    std::cout << std::alignment_of_v<char> << "\n";  // char (1)
    std::cout << std::alignment_of_v<long long> << "\n";  // long long (8)

    // padding issues
    std::cout << sizeof(S) << "\n";  // 32 bytes after padding
    std::cout << sizeof(T) << "\n";  // 24 bytes after padding

    return 0;
}