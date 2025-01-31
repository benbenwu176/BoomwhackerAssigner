#include <iostream>
#include <vector>
#include <array>
#include <random>
#include <algorithm>

struct Node {
    int data;
    std::string name;
};

void print_node(Node* node) {
    std::cout << node->data << " " << node->name << std::endl;
}

void print_vector(std::vector<int> v) {
    for (int i = 0; i < v.size(); i++) {
        std::cout << v[i] << " ";
    }
    std::cout << "\n";
}

void modify_vector(std::vector<int>& v) {
    for (int i = 0; i < v.size(); i++) {
        v[i] = i * 2;
    }
}

int main(int argc, char* argv[]) {
    int n = 5; 
    std::vector<int> arr(n);

    print_vector(arr);
    modify_vector(arr);
    print_vector(arr);

    return 0;
}

