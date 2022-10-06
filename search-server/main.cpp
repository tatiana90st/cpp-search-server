#include "process_queries.h"
#include "search_server.h"
#include "log_duration.h"
#include "test_example.h"
#include "unit_tests.h"
#include <iostream>
#include <string>
#include <vector>
#include <random>

using namespace std;

int main() {

    TestSearchServer();

    TestExampleFunctions();
    cout << "tests finished"s << endl;
}