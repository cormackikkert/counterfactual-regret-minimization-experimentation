#include <bits/stdc++.h>
using namespace std;

class EndgameGame {
	public:
	// static
	static const int NUM_MOVES = 13; 
	int claimNum  [12] = {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2}; // should be static
	int claimRank [12] = {2, 3, 4, 5, 6, 1, 2, 3, 4, 5, 6, 1};

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
		while (lastClaim >= 0 && !claimed[lastClaim]) --lastClaim;
		if (lastClaim < 0) return 1; // player called dudo with any claim. PUNISH THEM

		int req_count = claimNum[lastClaim];
		int req_rank = claimRank[lastClaim];

		int actual_count = ((p1Roll == req_rank) || (p1Roll == 1)) + ((p2Roll == req_rank) || (p2Roll == 1));

		return (actual_count >= req_count) ? 1 : -1;
	}

	int getNumLegalMoves() {
		if (calledDudo) return 0;

		int i = NUM_MOVES - 2;
		while (i >= 0 && !claimed[i]) --i;
		return NUM_MOVES - 2 - i + !calledDudo;
	}

	EndgameGame makeMove(int move) {
		// assumes the move is valid
		EndgameGame ng;
		ng.p1Roll = p1Roll;
		ng.p2Roll = p2Roll;
		ng.player = !player;
		copy(claimed, claimed+NUM_MOVES-1, ng.claimed);
		if (move == NUM_MOVES - 1)
			ng.calledDudo = true;
		else
			ng.claimed[move] = true;
		return ng;
	}
};

ostream& operator<<(ostream& os, const EndgameGame& g)
{
    os << "EndgameGame: " << g.p1Roll << " | " << g.p2Roll << endl;
	os << (g.player ? "Player 2" : "Player 1") << endl;
	for (int i = 0; i < g.NUM_MOVES - 1; ++i) cout << (g.claimed[i] ? "1" : "0");
	cout << (g.calledDudo ? "1" : "0") << endl;
    return os;
}

class EndgameNode {
	public:
	int num_actions;
	vector<double> regretSum;
	vector<double> strategySum;
	int infoSet;
	EndgameGame gameState;

	EndgameNode () {}

	EndgameNode(EndgameGame game) {
		gameState = game;

		num_actions = gameState.getNumLegalMoves();
		regretSum.resize(num_actions);
		strategySum.resize(num_actions);
		infoSet = gameState.getInfoSet();
	}

	vector<double> getStrategy(double realizationWeight) {
		vector<double> posRegrets (num_actions);
		for (int i = 0; i < num_actions; ++i) if (regretSum[i] > 0.0) posRegrets[i] = regretSum[i];

		double normalizing_sum = 0;
		for (int i = 0; i < num_actions; ++i) normalizing_sum += posRegrets[i];

		vector<double> strategy (num_actions);
		if (normalizing_sum > 0) {
			for (int i = 0; i < num_actions; ++i) 
				strategy[i] = posRegrets[i] / normalizing_sum;
		} else {
			for (int i = 0; i < num_actions; ++i) {
				strategy[i] = 1.0 / (double) num_actions;
			}
		}

		for (int i = 0; i < num_actions; ++i) {
			strategySum[i] += realizationWeight * strategy[i];
		}

		return strategy;
	}

	vector<double> getAverageStrategy() {
		vector<double> posRegrets (num_actions);
		for (int i = 0; i < num_actions; ++i) if (regretSum[i] > 0.01) posRegrets[i] = regretSum[i];

		double normalizing_sum = 0;
		for (int i = 0; i < num_actions; ++i) normalizing_sum += posRegrets[i];

		vector<double> strategy (num_actions);
		if (normalizing_sum > 0) {
			for (int i = 0; i < num_actions; ++i) 
				strategy[i] = (double) posRegrets[i] / normalizing_sum;
		} else {
			for (int i = 0; i < num_actions; ++i) {
				strategy[i] = (double) 1 / (double) num_actions;
			}
		}

		return strategy;
	}

	vector<pair<int, double>> getMoveProb(vector<double> strat) {
		vector<pair<int, double>> res (num_actions);
		for (int i = 0; i < num_actions; ++i) {
			res[i] = pair<int, double> {i + 13 - num_actions, strat[i]};
		}
		return res;
	}
};

map<int, EndgameNode> informationStates;

double cfr(EndgameGame game_state, double p0, double p1) {
	if (game_state.isTerminal()) return game_state.getUtility();

	// get information node (or create it if it does not exist)
	int info_set = game_state.getInfoSet();

	EndgameNode node;
	if (informationStates.find(info_set) == informationStates.end()) {
		node = EndgameNode(game_state);
	} else {
		node = informationStates[info_set];
	}
	
	int num_moves = game_state.getNumLegalMoves();

	
	double node_util = 0;
	vector<double> util (num_moves);
	vector<double> strategy = node.getStrategy(game_state.player ? p1 : p0);

	vector<pair<int, double>> moveProbs = node.getMoveProb(strategy);
	for (int i = 0; i < num_moves; ++i) {
		int action = moveProbs[i].first;
		double prob = moveProbs[i].second;
		if (game_state.player) {
			util[i] = -cfr(game_state.makeMove(action), p0, p1 * prob);  
		} else {
			util[i] = -cfr(game_state.makeMove(action), p0 * prob, p1);
		}
		node_util += prob * util[i];
	}

	// compute and accumulate counterfactual regret
	for (int i = 0; i < num_moves; ++i) {
		int action = moveProbs[i].first;
		double prob = moveProbs[i].second;
		double regret = util[i] - node_util;
		node.regretSum[i] += (game_state.player ? p0 : p1) * regret;
	}

	informationStates[info_set] = node;
	return node_util;
}
int main() {
    ios::sync_with_stdio(0), cin.tie(0), cout.tie(0);
	srand (time(NULL));
	
	/*
	cout << g.getNumLegalMoves() << endl;
	g = g.makeMove(0);
	cout << g.getNumLegalMoves() << endl;
	g = g.makeMove(2);
	cout << g.getNumLegalMoves() << endl;
	g = g.makeMove(11);
	cout << g << endl;
	cout << g.getNumLegalMoves() << endl;
	*/

	double res = 0;
	for (int i = 0; i < 10000; ++i) {
		EndgameGame g;
		res += cfr(g, 1.0, 1.0);
		if ((i + 1) % 1000 == 0) {
			cout << res / (double) i << endl;
		}
	}
	EndgameGame g;
	EndgameNode n = informationStates[g.getInfoSet()];
	for (double x : n.getAverageStrategy()) {
		cout << x << " ";
	}
	cout << endl;
	cout << (res / 10000.0) << endl;
}