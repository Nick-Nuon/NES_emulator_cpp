#include <iostream>

int main() {
uint8_t status = 0b0000'0001; // Example status value
int a = 5;
int b = 10;
int c = 3;
uint8_t carry = 0b0000'0001;

int sum1 = a + b + ( status & carry ? 1 : 0);
/*    ([&]() -> int {
    if (status & static_cast<uint8_t>(carry)) {
        return 1;
    } else {
        return 0;
    }
})();*/

std::cout << sum1;

}
