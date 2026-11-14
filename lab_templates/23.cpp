#include <iostream>
#include <map>
#include <string>
#include <sstream>
int main(){std::map<std::string,int>counts;std::istringstream input("cat dog cat bird dog cat");std::string word;
    // TODO: loop over words from input; increment counts[word]
    for(const auto &p:counts)std::cout<<p.first<<":"<<p.second<<"\n";
}
