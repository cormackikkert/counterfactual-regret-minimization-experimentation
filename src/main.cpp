#include <bits/stdc++.h>

using namespace std;

// range to 10 choose 6
vector<vector<int>> nCr = {
    {1},
    {1,1},
    {1,2,1},
    {1,3,3,1},
    {1,4,6,4,1},
    {1,5,10,10,5,1},
    {1,6,15,20,15,6,1},
    {1,7,21,35,35,21,7,1},
    {1,8,28,56,70,56,28,8,1},
    {1,9,36,84,126,126,84,36,9,1},
    {1,10,45,120,210,252,210,120,45,10,1}
};

class Claim {
    public:
    int num;
    int rank;
    int strength;
    bool dudo;
    bool empty;
    
    Claim(int n, int r, int dice) {
        num = n;
        rank = r;
        dudo = false;
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

    Claim() {
        dudo = true;
        // cout<<"dudduudududo"<<endl;
        num = -1;
        rank = -1;
        strength = -1;
    }

    // check if has strictly greater strength
    bool compare(Claim o) {
        // cout<<strength<<' '<<o.strength<<endl;
        return strength > o.strength;
    }
};

class Node { // Game tree for player
    public:
    bool root;
    bool leaf;
    Claim claim;
    vector<int> dice;
    int totalDice;
    bool maximising;
    bool underRoot;
    int depth;
    
    Node(vector<int> dice, int totalDice, bool maximising, Claim claim, bool underRoot, int depth) {
        this->dice = dice;
        this->totalDice = totalDice;
        this->maximising = maximising;
        this->claim = claim;
        this->underRoot = underRoot;
        this->depth = depth;
    }

    Node(vector<int> _dice, int _totalDice, bool _maximising, Claim _claim, bool _root, int _depth) {
        this->root = _root;
        this->dice = _dice;
        this->totalDice = _totalDice;
        this->maximising = _maximising;
        this->claim = _claim;
        this->depth = _depth;
        // cout<<claim.num<<' '<<claim.rank<<endl;
        if ((root || !claim.dudo)) {
            // cout<<depth<<endl;
            // cout<<claim.dudo<<' '<<maximising<<endl;
            for (int n = 1; n <= totalDice; n++) {
                for (int r = 1; r <= 6; r++) {
                    Claim temp = Claim(n,r,totalDice);
                    if (temp.compare(claim) && depth < 2) { //////////////////////////////////////////////// DEPTH
                        Node(dice, totalDice, !maximising, temp, 0, depth+1);
                        // cout<<n<<' '<<r<<endl;
                    }
                }
            }
            if (!root) {
                Claim dudo = Claim();
                Node(dice, totalDice, !maximising, dudo, 0, depth+1);
                // cout<<"dudo"<<endl;
            }
        }
    }

    
};

class Round { // useless
    public:
    
    vector<int> p1dice;
    vector<int> p2dice;
    // Node *p1node;
    // Node *p2node;

    Round(int dice1, int dice2) {
        srand (time(NULL));
        p1dice.assign(7, 0);
        p2dice.assign(7, 0);
        
        for (int i = 0; i < dice1; i++) {
            p1dice[rand()%6 +1]++;
        }
        for (int i = 0; i < dice2; i++) {
            p2dice[rand()%6 +1]++;
        }
        
        // p1node = new Node(p1dice, dice1+dice2, 1, Claim(3,3,dice1+dice2), 1, 0);
        // p2node = new Node(p2dice, dice1+dice2, 0, Claim(3,3,dice1+dice2), 0, 0);
        
    }
};

const int searchDepth = 5;

double heuristic(vector<int> dice, int totalDice, bool maximising, Claim claim) { // expected outcome from calling Dudo
    int n = totalDice - dice.size();
    int r = claim.rank;
    double expectedDice = dice[r]; // expected real count
    for (int k = 0; k <= n; k++) {
        expectedDice += pow(1.0/6,k) * pow(5.0/6,n-k) * nCr[n][k];
    }
    if (r>1) {
        expectedDice += dice[1];
        for (int k = 0; k <= n; k++) {
            expectedDice += pow(1.0/6,k) * pow(5.0/6,n-k) * nCr[n][k];
        }
    }
    double claimDice = claim.num;
    double h = n - expectedDice; // real < opponent claim = opponent lose points so good for me
    if (!maximising) { // i.e. opponent calling dudo
        h *= -1;
    }
    if (h==0) {
        h = maximising ? -1 : 1;
    }
    return h;
}

double minimax(vector<int> dice, int totalDice, bool maximising, Claim claim, int depth, bool dudo) {
    if (dudo) {
        return heuristic(dice, totalDice, maximising, claim);
    }
    double out;
    if (maximising) {
        out = DBL_MIN;
        for (int n = 1; n <= totalDice; n++) {
            for (int r = 1; r <= 6; r++) {
                Claim clm = Claim(n,r,totalDice);
                if (clm.compare(claim) && depth < searchDepth) { 
                    out = max(out, minimax(dice, totalDice, !maximising, clm, depth+1, 0));
                }
            }
        }
        out = max(out, minimax(dice, totalDice, !maximising, claim, depth+1, 1));
    } else {
        out = DBL_MAX;
        for (int n = 1; n <= totalDice; n++) {
            for (int r = 1; r <= 6; r++) {
                Claim clm = Claim(n,r,totalDice);
                if (clm.compare(claim) && depth < searchDepth) { 
                    out = min(out, minimax(dice, totalDice, !maximising, clm, depth+1, 0));
                }
            }
        }
        out = min(out, minimax(dice, totalDice, !maximising, claim, depth+1, 1));
    }
    return out;
}

Claim play(vector<int> dice, int totalDice, bool claimed, Claim claim) { // claim always true except p1 turn 1, claim is previous claim
    Claim out;
    double h = DBL_MIN;
    for (int n = 1; n <= totalDice; n++) {
        for (int r = 1; r <= 6; r++) {
            Claim clm = Claim(n,r,totalDice);
            double mm = minimax(dice, totalDice, 1, clm, 0, 0);
            if (mm > h) {
                h = mm;
                out = clm;
            }
        }
    }
    if (claimed) {
        double mm = heuristic(dice, totalDice, 1, claim);
        if (mm > h) {
            h = mm;
            out = Claim(); // call Dudo
        }
    }
    return out;
}

int main() {
    // Round dong = Round(6,6);
    // for (int i = 0; i < 6; i++) {
    //     cout<<dong.p1dice[i]<<" ";
    // }
    // cout<<'\n';
    // vector<int> p1dice(7,0);
    // for (int i = 0; i < 6; i++) {
    //     p1dice[rand()%6 +1]++;
    //     p1dice[3]=3;
    // }
    // Node dongNode(p1dice, 12, 1, Claim(3,3,12), 1, 0);
    // cout<<dongNode.heuristic()<<endl;
}