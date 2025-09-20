#include<mutex>
#include<bits/stdc++.h>

using namespace std;

class Node{
	public:
	string name;
	int id,anc_locked;
	Node* parent;
	vector<Node*>children;
	bool isLocked;
	std::mutex mtx;
	unordered_set<Node*>lockedDec;

	Node(string x, Node* p){
		name = x;
		parent = p;
		anc_locked=0;
		id = 0;
		isLocked= false;
	}
};

Node* buildTree(Node* root, vector<string>s, int m){
	queue<Node*>q;
	q.push(root);

	int indx=1;
	while(!q.empty() && indx<s.size()){
		Node* subroot= q.front(); q.pop();

		for(int i=0; i< m && indx<s.size(); i++){
			Node* childnode= new Node(s[indx++], subroot);
			subroot->children.push_back(childnode);
			q.push(childnode);
		}
	}
	return root;
}

class Tree{
	Node* root;
	unordered_map<string, Node*>mpp;
	
public:
	Tree(Node* r){ root=r;}
	Node* fetchRoot(){return root;}

	void buildMap(Node* root){
		if(!root) return;

		mpp[root->name]= root;
		for(auto child:root->children){
			buildMap(child);
		}
	}

	bool lock(string name, int uid) {
		Node* node = mpp[name];

		vector<Node*> path;
		Node* temp = node;
		while (temp) {
			path.push_back(temp);
			temp = temp->parent;
		}

		for (int i=path.size()-1; i>=0; i--) {
			path[i]->mtx.lock();

			if (i!=0 && path[i]->isLocked) {
				for (int j = path.size()-1; j>=i; j--) path[j]->mtx.unlock();
				return false;
			}
		}

		if (node->isLocked || !node->lockedDec.empty()) {
			for (auto* p:path) p->mtx.unlock();
			return false;
		}

		node->isLocked = true;
		node->id = uid;

		for (int i=1; i<path.size(); i++) {
			path[i]->lockedDec.insert(node);
		}

		for (auto* p:path) p->mtx.unlock();

		return true;
	}


	bool unlock(string name, int id){
		Node* node=mpp[name];

		if(!node->isLocked || node->id!=id ) return false;
		
		node->isLocked=false;
		node->id=0;

		//inform ancestors
		Node* par= node->parent;
		while(par){
			par->lockedDec.erase(node);
			par=par->parent;
		}
		return true;
	}

	bool upgrade(string name, int id){
		Node* node= mpp[name];

		if(node->isLocked || node->lockedDec.empty()) return false;

		for(auto dec: node->lockedDec) if(dec->id !=id) return false;

		vector<Node*>locked(node->lockedDec.begin(), node->lockedDec.end());
		for(auto dec : locked) unlock(dec->name, id);
		

		//ancestor will be checked here only
		return lock(node->name, id);
	}
};

int main(){
	int n,m;
	cin>>n>>m;

	vector<string>names(n);
	for(int i=0; i<n; i++) cin>>names[i];

	Node* root= new Node(names[0], nullptr);
	root= buildTree(root, names, m);

	Tree t(root);
	t.buildMap(t.fetchRoot());

	int op, id;
	string nodename;

	while(cin>>op>>nodename>>id){
		if(op == 1){
			 if(t.lock(nodename, id)) cout<<"true ";
			 else cout<<"false ";
		}

		else if(op==2){
			if(t.unlock(nodename, id))cout<<"true ";
			else cout<<"false ";
		}

		else{
			if(t.upgrade(nodename, id))cout<<"true ";
			else cout<<"false ";
		}
	}
}