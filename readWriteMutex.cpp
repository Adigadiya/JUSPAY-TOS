#include <bits/stdc++.h>
using namespace std;

class Node {
public:
    string name;
    int id;
    Node* parent;
    vector<Node*> children;
    bool isLocked;
    unordered_set<Node*> lockedDec;

    mutable shared_mutex rw_mutex; // for shared/exclusive locking

    Node(string x, Node* p) {
        name = x;
        parent = p;
        id = 0;
        isLocked = false;
    }
};

Node* buildTree(Node* root, vector<string>& s, int m) {
    queue<Node*> q;
    q.push(root);

    int idx = 1;
    while (!q.empty() && idx < s.size()) {
        Node* curr = q.front(); q.pop();
        for (int i = 0; i < m && idx < s.size(); i++) {
            Node* child = new Node(s[idx++], curr);
            curr->children.push_back(child);
            q.push(child);
        }
    }
    return root;
}

class Tree {
    Node* root;
    unordered_map<string, Node*> mpp;

public:
    Tree(Node* r) { root = r; }

    void buildMap(Node* node) {
        if (!node) return;
        mpp[node->name] = node;
        for (auto child : node->children) {
            buildMap(child);
        }
    }

    Node* fetchRoot() { return root; }

    bool lock(string name, int uid) {
        Node* node = mpp[name];

        // Build path to root
        vector<Node*> path;
        Node* temp = node;
        while (temp) {
            path.push_back(temp);
            temp = temp->parent;
        }

        // Acquire shared locks on ancestors (excluding self)
        vector<shared_lock<shared_mutex>> readLocks;
        for (int i = path.size() - 1; i > 0; --i) {
            readLocks.emplace_back(path[i]->rw_mutex);
        }

        // Acquire unique lock on current node
        unique_lock<shared_mutex> writeLock(node->rw_mutex);

        // Check if locked or descendants locked
        if (node->isLocked || !node->lockedDec.empty()) return false;

        // Check ancestors are not locked
        for (int i = 1; i < path.size(); ++i) {
            if (path[i]->isLocked || !path[i]->lockedDec.empty()) return false;
        }

        // All checks passed — proceed to lock
        node->isLocked = true;
        node->id = uid;

        // Update lockedDec for ancestors
        for (int i = 1; i < path.size(); ++i) {
            path[i]->lockedDec.insert(node);
        }

        return true;
    }

    // unlock() and upgrade() left as-is (not thread-safe)
    bool unlock(string name, int id) {
        Node* node = mpp[name];

        if (!node->isLocked || node->id != id) return false;

        node->isLocked = false;
        node->id = 0;

        Node* par = node->parent;
        while (par) {
            par->lockedDec.erase(node);
            par = par->parent;
        }

        return true;
    }

    bool upgrade(string name, int id) {
        Node* node = mpp[name];

        if (node->isLocked || node->lockedDec.empty()) return false;

        for (auto dec : node->lockedDec) {
            if (dec->id != id) return false;
        }

        vector<Node*> locked(node->lockedDec.begin(), node->lockedDec.end());
        for (auto dec : locked) unlock(dec->name, id);

        return lock(node->name, id);
    }
};

int main() {
    int n, m;
    cin >> n >> m;

    vector<string> names(n);
    for (int i = 0; i < n; i++) cin >> names[i];

    Node* root = new Node(names[0], nullptr);
    root = buildTree(root, names, m);

    Tree t(root);
    t.buildMap(t.fetchRoot());

    int op, id;
    string name;

    while (cin >> op >> name >> id) {
        if (op == 1) {
            cout << (t.lock(name, id) ? "true " : "false ");
        } else if (op == 2) {
            cout << (t.unlock(name, id) ? "true " : "false ");
        } else {
            cout << (t.upgrade(name, id) ? "true " : "false ");
        }
    }
}
