#include <iostream>
#include <iomanip>

using namespace std;

enum RecordType{BTREENODE,FLISTNODE};

class FreeListNode {
	RecordType type;
	FreeListNode *next;
	public:
	void init(FreeListNode *newNext=NULL) {
		next=newNext;
		type=FLISTNODE;
	}
	void push(FreeListNode *item) {  
		item->next=next;  // Record is now part of FreeList
		next=item;
	}
	FreeListNode* pop() {
		if (isEmpty()) return NULL;   // Pop an empty list
		FreeListNode *retVal=next;
		next=next->next;
		free(this);  // Record can now be used for BTreeNode
		return retVal;
	}
	bool isEmpty() {
		return next==NULL;
	}
};

class BTreeNode {
	RecordType type;
	BTreeNode *left,*right; // Left and Right pointers for the tree
	int key;  // The integer index (like autoincrement id in mysql)
			  // could be a couple of string, any combination really
	int location; // The location in the file of the data
	public:
	BTreeNode(int newKey=-1,int newLocation=-1,
		BTreeNode *newLeft=NULL,BTreeNode *newRight=NULL) {
		type=BTREENODE;
		left=newLeft;
		right=newRight;
		key=newKey;
		location=newLocation;
	}
	void invalidate() {
		location=-1;
		key=-1;
		left=NULL;
		right=NULL;
	}
	bool isValid() {
		return !((location==-1) && (key==-1));
	}
	void copy(const BTreeNode *other) {
		type=other->type;
		left=other->left;
		right=other->right;
		key=other->key;
		location=other->location;
	}
	bool operator <(const BTreeNode *other) const {
		return key<other->key;
	}
	friend ostream & operator <<(ostream &out,const BTreeNode *node) {
		long tadd=((long)node)&0xFFFF;
		int ladd=((long)node->left)&0xFFFF;
		int radd=((long)node->right)&0xFFFF;
		out  << hex << setfill('0') <<setw(4) << tadd
		    << dec<<'['<< node->key << ',' << node->location<<']'<<endl;  
		out << hex << setfill('0') << setw(4)<<setfill('0') << ladd 
		  << "<-   ->" 
		  << setw(4) << radd << dec << setfill(' ') << endl;
		if (node->left!=NULL)  out << "L->" << node->left;
		if (node->right!=NULL) out << "R->" << node->right;
		return out;
	}
	BTreeNode *find(int searchKey) {
		if (searchKey==key) return this;
		else if (key<searchKey && left!=NULL) return left->find(searchKey);
		else if (key>searchKey && right!=NULL) return right->find(searchKey);
		else return NULL;  // Could not find it
	}
	void add(BTreeNode *item) {  // Takes ownership of freeing node
		if (!isValid()) { // Special case of grounded empty root
			copy(item);
			delete item; // or use shared_ptr
		} else if (item->key==key){
			location=item->location;  // update location
			delete item; // or use shared_ptr
		}else if (key>item->key) {
			if (left!=NULL) left->add(item);
			else left=item;
		} else if (key<item->key) {
			if (right!=NULL) right->add(item);
			else right=item;
	    } 
	}
	void del(BTreeNode *item) {
		if (key==item->key) {  // this node needs to be deleted
			if (left!=NULL) { 
				BTreeNode *delTarget=left;
				if (right!=NULL) left->add(right);
				copy(left);
				delete delTarget; // or use shared_ptr
			} else if (right!=NULL) {
				BTreeNode *delTarget=right;
				copy(right);
				delete delTarget; // or use shared_ptr
			} else {
				invalidate();
			}
		} else if (key>item->key) {  // more to search left
			if (left!=NULL) left->del(item);
		} else if (key<item->key) {  // more to search right
			if (right!=NULL) right->del(item);
		}
	}
	void destroy() {  // complete memory cleanup of BinaryTreeNode
		if (left!=NULL) left->destroy();		
		if (right!=NULL) right->destroy();
		delete this;
	}
};

void BTreeNodeTest(){
	BTreeNode *root=new BTreeNode(3,1);
	root->add(new BTreeNode(1,2));
	root->add(new BTreeNode(6,7));
	root->add(new BTreeNode(8,9));
	root->add(new BTreeNode(12,14));
	cout << "Before " << endl << root;
	BTreeNode search=BTreeNode(8);
	root->del(&search);
	cout << "After " << endl << root;
    root->destroy();
}


union IndexRecord {
	BTreeNode btn;
	FreeListNode fln;
};

int main() {
	BTreeNodeTest();
	return 0;
}
