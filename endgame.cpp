#include <bits/stdc++.h>
using namespace std;

class EndgameGame {
	public:
	// static
	static const int NUM_MOVES = 13; 
	static constexpr int claimNum  [12] = {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2};
	static constexpr int claimRank [12] = {2, 3, 4, 5, 6, 1, 2, 3, 4, 5, 6, 1};

	// non-static
	bool calledDudo = false;
	bool player = false; // (P1, P2) = (false, true)
	int p1Roll = 0;
	int p2Roll = 0;
	bool claimed [12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	EndgameGame() {
		p1Roll = rand()%6 + 1;
		p2Roll = rand()%6 + 1;
	}

	bool isTerminal() {return calledDudo;}

	int getInfoSet() {
		int state = 0;

		// move information
		for (int i = 0; i < NUM_MOVES; ++i) {
			bool played = (i != NUM_MOVES - 1) ? claimed[i] : calledDudo;
			state |= (played << i);
		}

		// roll information
		state |= (player ? p2Roll : p1Roll) << NUM_MOVES; 

		return state;
	}

	int getUtility() {
		int lastClaim = NUM_MOVES - 2;
		while (!claimed[lastClaim]) --lastClaim;

		int ccount = claimNum[lastClaim];
		int crank = claimRank[lastClaim];

		int acount = (p1Roll == crank) + (p2Roll == crank);

		return (acount >= ccount) ? 1 : -1;
	}

	// TODO: play/unplay moves
};

int main() {
    ios::sync_with_stdio(0), cin.tie(0), cout.tie(0);

	EndgameGame g;
	cout << g.p1Roll << " " << g.p2Roll;
}