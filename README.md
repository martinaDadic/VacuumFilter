# VacuumFilter

Testing the program

To run benchmark.cpp we first need to run the subsequence_search.cpp program to generate the 
data.

In the project folder, run: 
g++ subsequence_search.cpp -o subsequence_search 
./subsequence_search.exe      


After that, we can run the benchmark by running:
g++ benchmark.cpp vacuum_filter.cpp -o test  
./test.exe      