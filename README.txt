Library Books Returns Register - BCS 2204 Group Coursework 1 (Scenario 5)
Student: Muhumuza Aryatujuna   Reg no: VU-BSF-2503-1652-DAY
Repository: https://github.com/muhumuzaaryatujuna-cpu/library

FILES
  DoublyLinkedList.h  Task A1: generic doubly linked list (class template DoublyLinkedList<T>)
  ReturnRecord.h      the ReturnRecord struct, date helpers, validation, file line format
  ReturnRegister.h    glue: unique-id rules, find/remove by id, printing, load/save, sample data
  Sorting.h           Task A2: insertion sort, merge sort, quicksort (random pivot), binary search
  main.cpp            interactive menu and program entry point
  tests.cpp           the 54 tests of the A3 test table
  bench.cpp           the A4 timing experiment
  returns.txt         sample data (12 records)

COMPILE (g++ or clang++, C++17)
  g++ -std=c++17 -O2 -Wall -Wextra main.cpp tests.cpp bench.cpp -o library_register

RUN
  ./library_register          interactive menu (loads returns.txt, creates it if missing)
  ./library_register test     run the 54 tests, prints the test table (expects TOTAL: 54 passed, 0 failed)
  ./library_register bench    run the timing experiment (n = 1000 ... 64000), CSV output; insertion sort is
                              slow at large n, expect about a minute
  ./library_register my.txt   menu using another data file

MEMORY CHECK
  g++ -std=c++17 -g -fsanitize=address -O1 main.cpp tests.cpp bench.cpp -o library_register_asan
  ./library_register_asan test

NOTES
  - Data file format, one record per line:  id|member|title|due|returned   ('#' lines are comments)
  - The O(n log n) sort used by the menu is merge sort (last digit of registration number is even).
    Compile with -DUSE_MERGE_SORT=0 to use quicksort instead.
