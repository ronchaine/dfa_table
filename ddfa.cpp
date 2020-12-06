#include "dfa_table.hpp"

int main()
{
    unorthodox::dfa_table<char8_t, int> dfa;

    dfa.insert({'a', 'b', 'c', 'd'}, 23);
    dfa.insert({'a', 'b', 'x'}, 53);
    dfa.insert("ölinää", 43);

    dfa.print();

    std::cout << dfa.entry({'a', 'b', 'c', 'd'}) << "\n";
    std::cout << dfa.entry("abx") << "\n";
}

