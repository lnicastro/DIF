#include <cstdlib>
#include <vector>
#include <iostream>
#include <iterator>
#include <fstream>


int main() {
    std::ifstream myfile("textexample.txt");

    myfile.unsetf(std::ios_base::skipws);
    
   unsigned line_count = std::count(
   std::istream_iterator<char>(myfile),
   std::istream_iterator<char>(), 
   '\n');
   
  std::cout << "Lines: " << line_count << "\n";
  return 0;
  }
