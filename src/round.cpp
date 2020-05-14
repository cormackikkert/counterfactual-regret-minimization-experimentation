#include <bits/stdc++.h>
#include "round.h"

using namespace std;

Round::Round(int dice1, int dice2) {
    srand (time(NULL));
    p1dice.assign(6,0);
    p2dice.assign(6,0);
    for (int i = 0; i < dice1; i++) {
        p1dice[rand()%6]++;
    }
    for (int i = 0; i < dice2; i++) {
        p2dice[rand()%6]++;
    }
}

vector<int> Round::getP1dice() {
    return p1dice;
}

vector<int> Round::getP2dice() {
    return p1dice;
}