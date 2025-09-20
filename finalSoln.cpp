#include <bits/stdc++.h>
using namespace std;

struct TreeNode {
    string name;
    int lockedBy = -1;          // -1 means unlocked
    int lockedCount = 0;        // 1 if locked, else 0
    vector<TreeNode*> children;
    TreeNode* parent = nullptr;
    unordered_set<TreeNode*> lockedDescendants;  // track locked descendants
};

class LockingTree {
    unordered_map<string, TreeNode*> nameToNode;

public:
    LockingTree(const vector<string>& names, int m) {
        if (names.empty()) return;
        TreeNode* root = new TreeNode();
        root->name = names[0];
        nameToNode[names[0]] = root;

        queue<TreeNode*> q;
        q.push(root);
        int i = 1;
        while (!q.empty() && i < (int)names.size()) {
            TreeNode* curr = q.front();
            q.pop();
            for (int cnt = 0; cnt < m && i < (int)names.size(); ++cnt, ++i) {
                TreeNode* child = new TreeNode();
                child->name = names[i];
                child->parent = curr;
                curr->children.push_back(child);
                nameToNode[names[i]] = child;
                q.push(child);
            }
        }
    }

    bool lock(const string& name, int id) {
        TreeNode* r = nameToNode[name];
        if (!r) return false;

        // If current node locked or has locked descendants, cannot lock
        if (r->lockedCount > 0 || !r->lockedDescendants.empty()) return false;

        // Increment lockedCount optimistically
        r->lockedCount++;
        if (r->lockedCount > 1) {  // Already locked by someone else
            r->lockedCount--;
            return false;
        }

        // Now update ancestors, add r to their lockedDescendants
        TreeNode* par = r->parent;
        while (par) {
        // Check if ancestor locked or if current node has new locked descendants (conflict)
            if (par->lockedCount > 0 || !r->lockedDescendants.empty()) {
                // Rollback lockedCount
                r->lockedCount--;

                // Rollback lockedDescendants insertions on path from r->parent to par (exclusive)
                TreeNode* par1 = r->parent;
                while (par1 != par) {
                    par1->lockedDescendants.erase(r);
                    par1 = par1->parent;
                }
                return false;
            }
            par->lockedDescendants.insert(r);
            par = par->parent;
        }

        r->lockedBy = id;
        return true;
    }

    bool unlock(const string& name, int id) {
        TreeNode* r = nameToNode[name];
        if (!r) return false;

        if (r->lockedCount == 0 || r->lockedBy != id) return false;

        r->lockedCount--;
        r->lockedBy = -1;

        TreeNode* par = r->parent;
        while (par) {
            par->lockedDescendants.erase(r);
            par = par->parent;
        }
        return true;
    }

    bool upgrade(const string& name, int id) {
        TreeNode* r = nameToNode[name];
        if (!r) return false;

        // Cannot upgrade if node locked or no locked descendants
        if (r->lockedCount > 0 || r->lockedDescendants.empty()) return false;

        // Check ancestors not locked
        TreeNode* par = r->parent;
        while (par) {
            if (par->lockedCount > 0) return false;
            par = par->parent;
        }

        // Check all locked descendants are locked by same id
        for (TreeNode* desc : r->lockedDescendants) {
            if (desc->lockedBy != id) return false;
        }

        // Unlock all locked descendants
        vector<TreeNode*> lockedNodes(r->lockedDescendants.begin(), r->lockedDescendants.end());
        for (TreeNode* desc : lockedNodes) {
            unlock(desc->name, id);
        }

        // Lock current node
        return lock(name, id);
    }
};

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;
    vector<string> names(n);
    for (int i = 0; i < n; ++i)
        cin >> names[i];

    LockingTree tree(names, m);

    int type, id;
    string name;
    while (cin >> type >> name >> id) {
        bool ans = false;
        if (type == 1)
            ans = tree.lock(name, id);
        else if (type == 2)
            ans = tree.unlock(name, id);
        else if (type == 3)
            ans = tree.upgrade(name, id);

        cout << (ans ? "true " : "false ");
    }
    cout << "\n";
    return 0;