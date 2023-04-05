#include <algorithm>
#include <iostream>
#include <vector>
#include <ranges>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    auto even_numbers = numbers
                        | std::views::filter([](int n) { return n % 2 == 0; })
                        | std::views::transform([](int n) { return n * 2; });

    for (int n : even_numbers) {
        std::cout << n << " ";
    }

    return 0;
}
