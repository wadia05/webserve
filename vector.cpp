#include <bits/stdc++.h>
#include <iostream>

void print_vector(std::vector<char> &v)
{
    int i  = 0;
    while (i < v.size())
    {
        std::cout << v[i] << " - ";
        i++;
    }
    std::cout << std::endl;
}


int main ()
{
    std::vector<int> v1;
    std::vector<char> v2 = {'s','s','s','d','l'};
    std::vector<int> v3 (5, 9);
    std::string str = "hello";
    std::vector<char> v4(str.begin(), str.end());

    // v2.pop_back();
    // v2.insert(v2.begin() , 'c');
    v2.erase (std::find (v2.begin(),  v2.end(), 'd'));
    // std::cout << v2.at(33);
    // print_vector(v1);
    print_vector(v4);
    // print_vector(v3);

    return 0;
}