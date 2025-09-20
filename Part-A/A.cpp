#include <bits/stdc++.h>
using namespace std;

class Node {
public:
    string name;
    int anc_locked, dec_locked, id;
    Node* parent;
    vector<Node*> children;
    bool isLocked;

    Node(string x, Node* p) {
        name = x;
        parent = p;
        anc_locked = 0;
        dec_locked = 0;
        id = 0;
        isLocked = false;
    }
};

Node* buildTree(Node* root, int& m, vector<string>& s) {
    queue<Node*> q;
    q.push(root);

    int indx = 1;
    while (!q.empty() && indx < s.size()) {
        Node* subroot = q.front(); q.pop();

        for (int i = 0; i < m && indx < s.size(); i++) {
            Node* childNode = new Node(s[indx], subroot);
            subroot->children.push_back(childNode);
            q.push(childNode);
            indx++;
        }
    }
    return root;
}

class Tree {
private:
    Node* root;
    unordered_map<string, Node*> mpp;

public:
    Tree(Node* r) { root = r; }
    Node* fetchRoot() { return root; }

    void buildMap(Node* node) {
        if (!node) return;

        mpp[node->name] = node;
        for (auto child : node->children) {
            buildMap(child);
        }
    }
    void informAncestors(Node* root, int val){
        Node* par= root->parent;

        while(par){
            par->dec_locked+=val;
            par=par->parent;
        }
    }
    void informDescendants(Node* root, int val) {
        for (auto child : root->children) {
            child->anc_locked += val;
            informDescendants(child, val);
        }
    }

    bool verifyDescendants(Node* node, int uid, vector<Node*>& lockedDec) {
        if (node->isLocked) {
            if (node->id != uid) return false;
            lockedDec.push_back(node);
        }

        if (node->dec_locked == 0) return true;

        bool ans = true;
        for (auto child : node->children) {
            ans &= verifyDescendants(child, uid, lockedDec);
            if (ans == false) return false;
        }
        return ans;
    }

    bool lock(string name, int uid) {
        Node* node = mpp[name];

        if (node->isLocked == true) return false;
        if (node->anc_locked != 0) return false;
        if (node->dec_locked != 0) return false;

        informAncestors(node, 1);
        informDescendants(node, 1);

        node->isLocked = true;
        node->id = uid;

        return true;
    }

    bool unlock(string name, int uid) {
        Node* node = mpp[name];

        if (node->isLocked == false) {
            return false;
        }
        if (node->id != uid) return false;

        informAncestors(node, -1);
        informDescendants(node, -1);

        node->isLocked = false;
        node->id = 0;
        return true;
    }

    bool upgrade(string name, int uid) {
        Node* node = mpp[name];

        if (node->isLocked == true) return false;
        if (node->anc_locked != 0) return false;
        if (node->dec_locked == 0) return false;

        vector<Node*> lockedDec;
        if (!verifyDescendants(node, uid, lockedDec)) return false;

        for (auto dec : lockedDec) {
            unlock(dec->name, uid);
        }

        lock(node->name, uid);
        return true;
    }
};

int main() {
    int n, m;
    cin >> n >> m;

    vector<string> s(n);
    for (int i = 0; i < n; i++) cin >> s[i];

    Node* root = new Node(s[0], nullptr);
    root = buildTree(root, m, s);

    Tree t(root);
    t.buildMap(t.fetchRoot());

    int op, uid;
    string name;

    while (cin >> op >> name >> uid) {
        if (op == 1) {
            if (t.lock(name, uid)) cout << "true ";
            else cout << "false ";
        } else if (op == 2) {
            if (t.unlock(name, uid)) cout << "true ";
            else cout << "false ";
        } else if (op == 3) {
            if (t.upgrade(name, uid)) cout << "true ";
            else cout << "false ";
        }
    }
}
