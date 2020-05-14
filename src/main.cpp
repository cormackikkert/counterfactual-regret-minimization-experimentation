#include <bits/stdc++.h>

using namespace std;

class Claim {
    public:
    int num;
    int rank;
    int strength;
    
    Claim(int n, int r, int dice) {
        num = n;
        rank = r;
        if (r!=1) {
            strength = 5*n + (n/2) + r - 7;
        } else {
            if (r <= dice/2) {
                strength = 11*n - 6;
            } else {
                strength = 5*dice + n - 1;
            }
        }
    }

    Claim() {}

    // check if has greater strength
    bool compare(Claim o) {
        return strength >= o.strength;
    }
};

class Node { // Game tree for player
    public:
    Node *prev;
    bool root;
    bool leaf;
    Claim claim;

    Node(Node *p) {
        prev = p;
        root = false;
        leaf = false;
    }

    Node() {
        root = true;
    }
};

class Round {
    public:
    vector<int> p1dice;
    vector<int> p2dice;
    Node p1node;
    Node p2node;

    Round(int dice1, int dice2) {
        srand (time(NULL));
        p1dice.assign(6, 0);
        p2dice.assign(6, 0);
        for (int i = 0; i < dice1; i++) {
            p1dice[rand()%6]++;
        }
        for (int i = 0; i < dice2; i++) {
            p2dice[rand()%6]++;
        }
    }
};
    

int main() {
    Round dong = Round(6,6);
    for (int i = 0; i < 6; i++) {
        cout<<dong.p1dice[i]<<" ";
    }
    cout<<'\n';
}