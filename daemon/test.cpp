#include <iostream>
#include <vector>
#include <thread>
#include <unordered_map>

using namespace std;

vector<int>* make_vec() {


}

int main() {
    unordered_map<string, int> h;
    h["a"] = 1;
    h["b"] = 2;

    for (auto x:h) {
        cout << x.first << ", " << x.second << endl;
    }

    return 0;
}
