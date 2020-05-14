#include <bits/stdc++.h>
#include "tree.h"
#include "round.h"

using namespace std;

int main() {
    Round dong = Round(6,6);
    for (int i = 0; i < 6; i++) {
        cout<<dong.getP1dice()[i]<<" ";
    }
    cout<<'\n';
}