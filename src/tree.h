class Node { // Game tree for player
    private:
    Node *prev;
    bool root;
    bool leaf;

    public:
    Node(Node *p);
    Node();
};