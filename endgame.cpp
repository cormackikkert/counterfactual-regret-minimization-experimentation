#pragma GCC target ("avx2")
#pragma GCC optimization ("O3")
#pragma GCC optimization ("unroll-loops")

#include <bits/stdc++.h>
using namespace std;
using namespace std::chrono; 

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
		for (int i = 0; i < num_actions; ++i) if (strategySum[i] > 0.01) posRegrets[i] = strategySum[i];

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

	pair<int, int> sampleActionCurrent() {
		vector<pair<int, double>> actions = getMoveProb(getStrategy(0.0));
		double r = static_cast <double> (rand()) / static_cast <double> (RAND_MAX);
		double cur = 0.0;
		for (int i = 0; i < actions.size(); ++i) {
			if (cur + actions[i].second >= r) return {i, actions[i].first};
			cur += actions[i].second;
		}
		cout << "FAILED: " << r << " " << cur << endl;
		for (pair<int, double> t : actions) {
			cout << t.first << " " << t.second << endl;
		}
		cout << infoSet << endl;
		return {0, actions[0].first};
	}

	int sampleAction() {
		vector<pair<int, double>> actions = getMoveProb(getAverageStrategy());
		double r = static_cast <double> (rand()) / static_cast <double> (RAND_MAX);
		double cur = 0.0;
		for (int i = 0; i < actions.size(); ++i) {
			if (cur + actions[i].second >= r) return actions[i].first;
			cur += actions[i].second;
		}
		cout << "FAILED: " << r << " " << cur << endl;
		for (pair<int, double> t : actions) {
			cout << t.first << " " << t.second << endl;
		}
		cout << infoSet << endl;
		return actions[0].first;
	}
};

/*
	Variants
		- Normal
		- Weighted strategy sampling
		- Positive regret matching
		- Alternating updates
*/
class DefaultCFRTrainer {
	public:
	map<int, EndgameNode> informationStates;
	DefaultCFRTrainer () {}

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

	inline double train(int iterations) {
		double res;
		for (int i = 0; i < iterations; ++i) {
			for (int r1 = 1; r1 <= 6; ++r1) {
				for (int r2 = 1; r2 <= 6; ++r2) {
					EndgameGame g;
					g.p1Roll = r1;
					g.p2Roll = r2;
					res += cfr(g, 1.0, 1.0);
				}
			}
			
		}
		return res / (36.0 * (double) iterations);
	}

	void train_time(int time) {
		auto s1 = high_resolution_clock::now(); 
		while (true) {
			train(1);
			auto e1 = high_resolution_clock::now(); 
			auto d1 = duration_cast<microseconds>(e1 - s1); 
			if (d1.count() > time) break;
		}
	}
};

class DefaultCFRWeightedTrainer {
	public:
	map<int, EndgameNode> informationStates;
	DefaultCFRWeightedTrainer () {}
	double global_its = 1.0;

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
		vector<double> strategy = node.getStrategy(global_its * (game_state.player ? p1 : p0));

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

	inline double train(int iterations) {
		double res;
		for (int i = 0; i < iterations; ++i) {
			global_its += 1.0;
			for (int r1 = 1; r1 <= 6; ++r1) {
				for (int r2 = 1; r2 <= 6; ++r2) {
					EndgameGame g;
					g.p1Roll = r1;
					g.p2Roll = r2;
					res += cfr(g, 1.0, 1.0);
				}
			}
			
		}
		return res / (36.0 * (double) iterations);
	}

	void train_time(int time) {
		auto s1 = high_resolution_clock::now(); 
		while (true) {
			train(1);
			auto e1 = high_resolution_clock::now(); 
			auto d1 = duration_cast<microseconds>(e1 - s1); 
			if (d1.count() > time) break;
		}
	}
};

class DefaultCFRPosRegretTrainer {
	public:
	map<int, EndgameNode> informationStates;
	DefaultCFRPosRegretTrainer () {}

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
			node.regretSum[i] = max(0.0, node.regretSum[i]);
		}

		informationStates[info_set] = node;
		return node_util;
	}

	inline double train(int iterations) {
		double res;
		for (int i = 0; i < iterations; ++i) {
			for (int r1 = 1; r1 <= 6; ++r1) {
				for (int r2 = 1; r2 <= 6; ++r2) {
					EndgameGame g;
					g.p1Roll = r1;
					g.p2Roll = r2;
					res += cfr(g, 1.0, 1.0);
				}
			}
			
		}
		return res / (36.0 * (double) iterations);
	}

	void train_time(int time) {
		auto s1 = high_resolution_clock::now(); 
		while (true) {
			train(1);
			auto e1 = high_resolution_clock::now(); 
			auto d1 = duration_cast<microseconds>(e1 - s1); 
			if (d1.count() > time) break;
		}
	}
};

class DefaultESCFRTrainer {
	public:
	map<int, EndgameNode> informationStates;
	DefaultESCFRTrainer () {}

	double cfr(EndgameGame game_state, bool currentPlayer) {
		if (game_state.isTerminal()) {
			double u = game_state.getUtility();
			return (game_state.player == currentPlayer) ? u : -u;
		}

		// get information node (or create it if it does not exist)
		int info_set = game_state.getInfoSet();

		EndgameNode node;
		if (informationStates.find(info_set) == informationStates.end()) {
			node = EndgameNode(game_state);
		} else {
			node = informationStates[info_set];
		}
		
		int num_moves = game_state.getNumLegalMoves();

		if (game_state.player == currentPlayer) {
			double node_util = 0;
			vector<double> util (num_moves);
			vector<double> strategy = node.getStrategy(0.0); // hacky

			vector<pair<int, double>> moveProbs = node.getMoveProb(strategy);
			for (int i = 0; i < num_moves; ++i) {
				int action = moveProbs[i].first;
				double prob = moveProbs[i].second;
				util[i] = cfr(game_state.makeMove(action), currentPlayer);  
				node_util += prob * util[i];
			}

			// compute and accumulate counterfactual regret
			for (int i = 0; i < num_moves; ++i) {
				int action = moveProbs[i].first;
				double prob = moveProbs[i].second;
				double regret = util[i] - node_util;
				node.regretSum[i] += regret;
			}

			informationStates[info_set] = node;
			return node_util;
		} else {
			pair<int, int> move = node.sampleActionCurrent();
			node.strategySum[move.first]+=1.0; // hacky 
			informationStates[info_set] = node;
			return cfr(game_state.makeMove(move.second), currentPlayer);
		}
	}

	inline double train(int iterations) {
		double res;
		for (int i = 0; i < iterations; ++i) {
			for (int r1 = 1; r1 <= 6; ++r1) {
				for (int r2 = 1; r2 <= 6; ++r2) {
					EndgameGame g;
					g.p1Roll = r1;
					g.p1Roll = r2;
					res += cfr(g, false);
					g = EndgameGame();
					g.p1Roll = r1;
					g.p1Roll = r2;
					res -= cfr(g, true);
				}
			}
			
		}
		return res / (2.0 * 36.0 * (double) iterations);
	}

	void train_time(int time) {
		auto s1 = high_resolution_clock::now(); 
		while (true) {
			train(1);
			auto e1 = high_resolution_clock::now(); 
			auto d1 = duration_cast<microseconds>(e1 - s1); 
			if (d1.count() > time) break;
		}
	}
};

double compareOnce(map<int, EndgameNode> & bot1, map<int, EndgameNode> & bot2, int r1, int r2) {
	EndgameGame g;
	g.p1Roll = r1;
	g.p2Roll = r2;
	while (!g.isTerminal()) {

		int infoSet = g.getInfoSet();

		EndgameNode node;
		if (g.player) {
			if (bot2.find(infoSet) == bot2.end()) bot2[infoSet] = EndgameNode(g);
			node = bot2[g.getInfoSet()];
		} else {
			if (bot1.find(infoSet) == bot1.end()) bot1[infoSet] = EndgameNode(g);
			node = bot1[g.getInfoSet()];
		}

		int action = node.sampleAction();
		g = g.makeMove(action);
	}
	// return score for player 1
	return (g.player) ? -g.getUtility() : g.getUtility();
}

double compare(map<int, EndgameNode> & bot1, map<int, EndgameNode> & bot2) {
	// play 50 games, swapping players each one
	double bot1util = 0.0;
	double bot2util = 0.0;
	for (int i = 0; i < 1000; ++i) {
		for (int r1 = 1; r1 <= 6; ++r1) {
			for (int r2 = 1; r2 <= 6; ++r2) {
				bot1util += compareOnce(bot1, bot2, r1, r2);
				bot1util -= compareOnce(bot2, bot1, r1, r2);
			}
		}
	}
	return bot1util / 36.0 / 2000.0;
}

void printStates(map<int, EndgameNode> & bot1) {
	cout << bot1.size() << endl;
	for (pair<int, EndgameNode> data : bot1) {
		int infoSet = data.first;
		EndgameNode node = data.second;
		cout << infoSet << endl;
		cout << node.num_actions << endl;
		for (double x : node.strategySum) cout << x << " ";
		cout << endl;
	}
}

map<int, EndgameNode> readStates() {
	map<int, EndgameNode> data;
	int N; cin >> N;
	for (int i = 0; i < N; ++i) {		
		EndgameNode node;
		cin >> node.infoSet;
		cin >> node.num_actions;
		node.strategySum.resize(node.num_actions);
		for (int j = 0; j < node.num_actions; ++j) {
			cin >> node.strategySum[j];
		}
		data[node.infoSet] = node;
	}
	return data;
}

int main() {
    // ios::sync_with_stdio(0), cin.tie(0), cout.tie(0);
	srand (time(NULL));
	map<int, EndgameNode> standard = readStates();
	// cout << "READ" << endl;

	/* 
		- Training code

	DefaultCFRTrainer trainer1;
	for (int i = 0; true; ++i) {
		trainer1.train(1000);
		ofstream file;
		file.open("saved" + to_string(i) + ".txt");
		
		map<int, EndgameNode> & bot1 = trainer1.informationStates;

		file << bot1.size() << endl;
		for (pair<int, EndgameNode> data : bot1) {
			int infoSet = data.first;
			EndgameNode node = data.second;
			file << infoSet << endl;
			file << node.num_actions << endl;
			for (double x : node.strategySum) file << x << " ";
			file << endl;
		}

		file.close();
		// trainer2.train(50);
		// cout << compare(trainer1.informationStates, standard) << endl;
	}
	*/
	DefaultCFRTrainer t1 = DefaultCFRTrainer();
	DefaultCFRWeightedTrainer t2 = DefaultCFRWeightedTrainer();
	DefaultCFRPosRegretTrainer t3 = DefaultCFRPosRegretTrainer();
	DefaultESCFRTrainer t4 = DefaultESCFRTrainer ();

	cout << "---" << endl;
	cout << compare(t1.informationStates, standard) << endl;
	cout << compare(t2.informationStates, standard) << endl;
	cout << compare(t3.informationStates, standard) << endl;
	cout << compare(t4.informationStates, standard) << endl;

	for (int i = 0; ; ++i) {
		t1.train_time(10000000);
		t2.train_time(10000000);
		t3.train_time(10000000);
		t4.train_time(10000000);
		cout << "---" << endl;
		cout << compare(t1.informationStates, standard) << endl;
		cout << compare(t2.informationStates, standard) << endl;
		cout << compare(t3.informationStates, standard) << endl;
		cout << compare(t4.informationStates, standard) << endl;
	}
	
	/*
	auto s2 = high_resolution_clock::now(); 
	while (true) {
		t1.train(1);
		auto e2 = high_resolution_clock::now(); 
		auto d2 = duration_cast<microseconds>(e2 - s2); 
		if (d2.count() > 10000000) break;
	}

	cout << compare(t1.informationStates, t2.informationStates) << endl;

	// -0.78
	*/
}