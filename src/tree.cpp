#include <bits/stdc++.h>
#include "tree.h"

using namespace std;   

Node::Node(Node *p) {
    root = false;
    leaf = false;
}

Node::Node() {
    root = true;
}
 