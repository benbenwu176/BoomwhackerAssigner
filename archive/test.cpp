#include <iostream>
#include <vector>
#include <algorithm>  // for upper_bound

// Returns the value of the closest element in `arr` that is <= target.
// If no such element exists, throws an exception.
int findClosestLE(const std::vector<int>& arr, int target) {
    // upper_bound returns iterator to first element > target
    auto it = std::upper_bound(arr.begin(), arr.end(), target);
    if (it == arr.begin()) {
        // All elements in arr are > target
        throw std::out_of_range("No element ≤ target");
    }
    // step back to get the largest element ≤ target
    --it;
    return *it;
}

int main() {
    int n;
    std::cout << "Enter number of elements: ";
    std::cin >> n;

    std::vector<int> arr(n);
    std::cout << "Enter " << n << " sorted elements:\n";
    for (int i = 0; i < n; ++i) {
        std::cin >> arr[i];
    }

    int target;
    std::cout << "Enter target value: ";
    std::cin >> target;

    try {
        int closest = findClosestLE(arr, target);
        std::cout << "Closest element <= " << target << " is " << closest << "\n";
    } catch (const std::out_of_range& e) {
        std::cout << "Error: " << e.what() << "\n";
    }

    return 0;
}
