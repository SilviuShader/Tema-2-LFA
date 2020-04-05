#include <fstream>
#include <iostream>
#include "NFA.h"

#define LAMBDA '*'

using namespace std;

int main()
{
    NFA nfa = NFA(LAMBDA);

    ifstream fin("data.txt");
    fin >> nfa;
    fin.close();

    cout << "LAMBDA-NFA:" << endl;
    cout << nfa << endl;

    nfa.RemoveLambda();
    cout << "NFA:" << endl;
    cout << nfa << endl;

    nfa.ToDFA();
    cout << "DFA: " << endl;
    cout << nfa << endl;

    return 0;
}