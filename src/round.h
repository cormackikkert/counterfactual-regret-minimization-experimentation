#include "tree.h"

class Round {
    private:
    std::vector<int> p1dice;
    std::vector<int> p2dice;
    Node p1node;
    Node p2node;

    public:
    Round(int dice1, int dice2);
    std::vector<int> getP1dice();
    std::vector<int> getP2dice();
};