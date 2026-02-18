#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <cmath>
#include <pthread.h>

using namespace std;

struct Args {
    char ch;
    int bits;
    vector<int> positions;
    int frequency;
    vector<char>* decodedMsg;
    vector<int>* allPositions;
    int start;
};

void* Thread(void* voidPtr) {
    Args* ptr = (Args*) voidPtr;

    ptr->bits = 0;
    ptr->positions.clear();

    for (int i = ptr->start; i < ptr->start + ptr->frequency; i++) {
        int pos = ptr->allPositions->at(i);
        ptr->positions.push_back(pos - 1);
        ptr->bits += 2 * (int)log2(pos) + 1;
        ptr->decodedMsg->at(pos - 1) = ptr->ch;
    }

    return nullptr;
}

int main() {
    int m = 0;
    string binary = "";
    vector<pair<char, int>> symbols;
    vector<char> decodedMsg;

    cin >> m;

    for (int i = 0; i < m; i++) {
        char symbol = ' ';
        int frequency = 0;
        cin >> symbol >> frequency;
        symbols.push_back({symbol, frequency});
    }

    cin >> binary;

    sort(symbols.begin(), symbols.end(),
    [](const pair<char,int>& a, const pair<char,int>& b) {
        if (a.second != b.second)
            return a.second > b.second;
        return a.first < b.first;
    });

    vector<int> positions;

    int i = 0;
    while (i < binary.length()) {
        int n = 0;

        while (i < binary.length() && binary.at(i) == '0') {
            n++;
            i++;
        }

        string temp = binary.substr(i + 1, n);
        int value = 0;
        for (char c : temp)
            value = value * 2 + (c - '0');
        positions.push_back((int)pow(2, n) + value);

        i += 1 + n;
    }

    int totalPositions = 0;
    for (auto& symbol : symbols)
        totalPositions += symbol.second;
    decodedMsg.resize(totalPositions);

    vector<pthread_t> tid(m);
    vector<Args> result(m);

    int start = 0;

    for (int i = 0; i < m; i++) {
        result.at(i).ch = symbols[i].first;
        result.at(i).frequency = symbols[i].second;
        result.at(i).allPositions = &positions;
        result.at(i).start = start;
        result.at(i).decodedMsg = &decodedMsg;

        start += symbols[i].second;

        if (pthread_create(&tid.at(i), nullptr, Thread, (void*)&result.at(i)) != 0) {
            cerr << "Error creating the thread" << endl;
            exit(0);
        }
    }

    for (int i = 0; i < m; i++)
        pthread_join(tid.at(i), nullptr);

    for (int i = 0; i < m; i++) {
        cout << "Symbol: " << result.at(i).ch << ", Frequency: " << result.at(i).frequency << endl;
        cout << "Positions: ";
        for (int pos : result.at(i).positions)
            cout << pos << " ";
        cout << endl;
        cout << "Bits to represent the position(s): " << result.at(i).bits << endl;
        cout << endl;
    }

    cout << "Decoded message: ";
    for (char c : decodedMsg)
        cout << c;
    cout << endl;
}