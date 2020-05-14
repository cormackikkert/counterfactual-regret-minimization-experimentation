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

    // check if has greater strength
    bool compare(Claim o) {
        return strength >= o.strength;
    }
};

int main() {
    int dice =4;
    for (int i = 1; i <= 6; i++) {
        for(int j = 2; j <= 6; j++) {
            Claim dong = Claim(i,j,dice);
            cout<<dong.num<<' '<<dong.rank<<' '<<dong.strength<<endl;
        }
        int j=1;
        Claim dong = Claim(i,j,dice);
        cout<<dong.num<<' '<<dong.rank<<' '<<dong.strength<<endl;
    }

    cout<<'\n';
    Claim dong =Claim(1,2,2);
    Claim dong2 = Claim(1,1,2);
    cout<<dong.compare(dong2)<<'\n';
}