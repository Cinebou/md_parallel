#include <iostream>
#include <fstream>

using std::cout; using std::ofstream;
using std::endl; using std::string;

int main()
{
    //string filename("tmp.txt");
    ofstream file_out;
    file_out.open("test.txt", std::ios_base::app);


    int line = 12;
    double distance = 3;
    double offset = 0;

    for (int k = 0; k <line; ++k){
      for (int i = 0; i < line; ++i){
        for (int j = 0; j<line;++j){
          file_out << "He " << offset + distance*k << " " << offset + distance*i << " " << offset + distance*j << " 0 0 0"<< endl;
          //std::cout << " minimum box length in case1.txt = " << offset + (line+1) * distance << std::endl;
        }
      }
    }
    return 0;
}

//  icpc -axCORE-AVX512 initial_data.cpp
//  ./a.out
