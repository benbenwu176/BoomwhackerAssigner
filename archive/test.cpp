#include <iostream>
#include <vector>
#include <unordered_set>
#include <iterator>
#include <string>
#include <type_traits>

struct Person {
    std::string name;
    int         age;
};

template<typename Iter, typename Proj>
std::vector<typename std::iterator_traits<Iter>::value_type>
last_n_unique_by(Iter first,
                 Iter last,
                 std::size_t n,
                 Proj proj,
                 typename std::iterator_traits<Iter>::value_type initial)
{
    using T   = typename std::iterator_traits<Iter>::value_type;
    using Key = std::invoke_result_t<Proj, T>;

    std::vector<T>          result;
    std::unordered_set<Key> seen;

    // 1) Insert the initial element first:
    Key init_key = proj(initial);
    seen.insert(init_key);
    result.push_back(initial);

    // 2) Now walk backwards through [first, last),
    //    stopping when we've collected n total elements:
    auto rbegin = std::make_reverse_iterator(last);
    auto rend   = std::make_reverse_iterator(first);

    for (auto rit = rbegin;
         rit != rend && result.size() < n;
         ++rit)
    {
        T elem = *rit;
        Key key = proj(elem);
        if (seen.insert(key).second) { 
            result.push_back(elem);
        }
    }

    return result;
}


int main() {
    Person a{"Alice", 30},
           b{"Bob",   25},
           c{"Carol", 27},
           d{"Bob",   26},
           e{"Dave",  35},
           f{"Alice", 31};

    // vector of pointers:
    std::vector<Person*> everyone = { &a, &b, &c, &d, &e, &f };

    // Let's say we always want "Zoe" first, then the last 3 unique names from the back:
    Person zoe{"Bob", 99};
    auto last4 = last_n_unique_by(
        everyone.begin(),
        everyone.end(),
        4,                          // total count (including Zoe)
        [](Person* p){ return p->name; },
        &zoe                       // initial Person*
    );

    // prints: Zoe, Alice(31), Dave(35), Bob(26)
    for (auto *p : last4) {
        std::cout << p->name
                  << " (age " << p->age << ")\n";
    }
}