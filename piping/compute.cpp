// TODO: Check argv for NULL pointers

#include <iostream>
#include "compute.h"


void output() {
    int numbers[5] = {1, 2, 3, 4, 5};
    std::string text = "Hello World";

    for (int i = 0; i < 2; i++) {
        for (int num : numbers) {
            std::cout << num + i;
        }
        std::cout << text;
    }
}

int main(int argc, char* argv[]) {
    
    return 0;
}