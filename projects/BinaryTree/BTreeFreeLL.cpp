/* Modify the code below as needed to make is so no data of FreeListNode
 * or BTreeNode allocates from the heap (from memory), instead make
 * the two classes work with a file on disk that is opened in 
 * IntIndex constructor.  When BTreeNodes are freed they should be
 * added to FreeListNode.
 * 
 * Changes to FreeListNode and BTreeNode should really focus on removing
 * new and deletes and instead use the FreeListNode linked list or
 * allocate new bytes on the file on disk.
 * 
 * IntIndex currently does not new or delete anything from memory
 * and that should remain.
 * 
 * I encourage you to make use of the two test routines to verify
 * your modifications to FreeListNode and BTreeNode before working
 * on modifications to IntIndex.
 * 
 * But you can tackle the update how ever you like as long as the 
 * code does not grow in memory size after the constructor of
 * IntIndex is called.
 */ 

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sys/stat.h>
#include <vector>
#include <string>

using namespace std;

void BTreeNodeTest();
void FreeListNodeTest();
void IntIndexTest();


enum RecordType{
	BTREENODE, 
	FLISTNODE
};


class DBException {
	private:
		string message;

	public:
		DBException(string msg) {
			message = msg;
		}

		string getMessage() {
			return message;
		}
};


/**
 * @brief This class will control access to the database file
 * 
 *   	We call this to create a new record
 * 
 */
class Memory {
	private:
		fstream *indexFile;
		int recordSize;

		void checkFile() {
			if (!indexFile->is_open()) {
				throw DBException("File not open");
			}
		}

	
	public:
		Memory(string fileName) {
			struct stat s;
			ofstream t;

			cout << "Create Database Memory Allocation" << endl;

			if (stat(fileName.c_str(), &s) != 0) {
				cout << "Creating file: " << fileName << endl;
				t.open(fileName.c_str());
				t.close();

				indexFile = new fstream(fileName.c_str());
				indexFile->open(fileName);
				checkFile();

				// root.btn.init();
				// head.fln.init();

				// writeAt(0, root);
				// writeAt(1, head);

			} else {

				cout << "Opening file: " << fileName << endl;
				indexFile = new fstream(fileName.c_str());
				indexFile->open(fileName);
				checkFile();

				// readAt(0, root);
				// readAt(1, head);
			}
			
		
			// root.btn.init(); // <-- temp
			// head.fln.init(); // <-- temp
		}

		~Memory() {
			indexFile->close();
			delete indexFile;
		}
};

// 
class FreeListNode {
	public:
		RecordType type;
		void init(FreeListNode *newNext = NULL) {
			next = newNext;
			type = FLISTNODE;
			memIndex = -1;
		}

	private: 
		FreeListNode *next;
		int memIndex;

	public:

		void push(FreeListNode *item) {  
			item->next = next;  // Record is now part of FreeList
			next = item;
		}

		FreeListNode* pop() {
			FreeListNode *retVal;
			
			if (isEmpty()){
				return NULL;   // Pop an empty list
			}
			
			retVal = next;
			next = next->next;
			retVal->next = NULL;

			//free(this);  // Record can now be used for BTreeNode
			return retVal;
		}

		bool isEmpty() {
			return next == NULL;
		}

		friend ostream & operator << (ostream &out, const FreeListNode *node) {
			long add = ((long)node) & 0xFFFF;

			out 
				// << hex << setfill('0') << setw(4) 
				<< '(' << add << ')' 
				// << dec << setfill(' ') 
				<< "->";  
			
			if (node->next!=NULL) {
				out << node->next;
			} else {
				out << "Tail(NULL)" << endl;
			} 

			return out;
		}
};



// Nodes for the binary tree
class BTreeNode {
	public:
		RecordType type;
		void init(int newKey = -1, int newLocation = -1, BTreeNode *newLeft = NULL, BTreeNode *newRight = NULL) {
			type     = BTREENODE;
			left     = newLeft;
			right    = newRight;
			key      = newKey;
			location = newLocation;
		}

	private:
		BTreeNode *left, *right; // Left and Right pointers for the tree
		int key;  				 // The integer index (like autoincrement id in mysql)
								 // could be a couple of string, any combination really
		int location; 			 // The location in the file of the data

	public:

		void invalidate() {
			location = -1;
			key      = -1;
			left     = NULL;
			right    = NULL;
		}

		bool isValid() {
			return !((location == -1) && (key == -1));
		}

		int getLocation() const { 
			return location;
		}

		void copy(const BTreeNode *other) {
			type 		= other->type;
			left 		= other->left;
			right 		= other->right;
			key 		= other->key;
			location 	= other->location;
		}

		BTreeNode *findByKey(int searchKey) {
			BTreeNode *retNode;

			if (searchKey == key) {
				retNode = this;
			} else if (key < searchKey && left != NULL) {
				retNode = left->findByKey(searchKey);
			} else if (key > searchKey && right != NULL) {
				retNode = right->findByKey(searchKey);
			} else {
				retNode = NULL;
			} 

			return retNode;
		}

		void add(const BTreeNode *item) {  // Allocates and copies
			if (!isValid()) { // Special case of grounded empty root
				copy(item);
			} else if (item->key == key){
				location = item->location;  // update location
			}else if (key > item->key) {
				
				if (left != NULL) {
					left->add(item);
				} else {
					left = new BTreeNode();
					left->copy(item);
				}

			} else if (key < item->key) {
				
				if (right != NULL) {
					right->add(item);
				} else {
					right = new BTreeNode();
					right->copy(item);
				}
			} 
		}

		void del(const BTreeNode *item) {
			if (key == item->key) {  // this node needs to be deleted

				if (left != NULL) { 
					BTreeNode *delTarget=left;
					
					if (right != NULL) {
						left->add(right);
					}

					copy(left);
					delete delTarget; // or use shared_ptr

				} else if (right != NULL) {
					BTreeNode *delTarget=right;
					copy(right);
					delete delTarget; // or use shared_ptr

				} else {
					invalidate();
				}

			} else if (key > item->key) {  // more to search left

				if (left != NULL) {
					left->del(item);
				} 

			} else if (key < item->key) {  // more to search right

				if (right != NULL) {
					right->del(item);
				}
			}
		}

		void destroy() {  // complete memory cleanup of BinaryTreeNode
			
			if (left != NULL) {
				left->destroy();		
			}

			if (right != NULL) {
				right->destroy();
			}

			delete this;
		}

		bool operator < (const BTreeNode *other) const {
			return key < other->key;
		}

		friend ostream & operator << (ostream &out, const BTreeNode *node) {
			// long tadd = ((long)node) & 0xFFFF;
			// int ladd  = ((long)node->left) & 0xFFFF;
			// int radd  = ((long)node->right) & 0xFFFF;

			out 
				// << hex << setfill('0') << setw(4) << tadd << dec 
				<< "\t[" << node->key << ',' << node->location << "]  ";  


			if (node->left != NULL) {
				out << "L[" << node->left->key << ',' << node->left->location << "] ";
			}  

			if (node->right != NULL) {
				out << "R[" << node->right->key << ',' << node->right->location << ']';
			}

			if (node->left == NULL && node->right == NULL) {
				out << "Leaf";
			}
			
			out << endl;

			// out 
			// 	// << hex << setfill('0') << setw(4) << setfill('0') << ladd 
			// 	<< "<-   ->" << setw(4) 
			// 	// << radd << dec << setfill(' ') 
			// 	<< endl;

			if (node->left != NULL) {
				out << node->left;
				// out << "L->" << node->left;
			}  

			if (node->right != NULL) {
				out << node->right;
				// out << "R->" << node->right;
			}

			return out;
		}
};


union IndexRecord {
	BTreeNode btn;
	FreeListNode fln;
};


class IntIndex {
	private:
		IndexRecord root;
		IndexRecord head;
		fstream *indexFile;

		void readAt(int i, IndexRecord &ir) {
			indexFile->seekg(i * sizeof(IndexRecord));
			indexFile->read((char *)(&ir), sizeof(IndexRecord));
		}

		void writeAt(int i, IndexRecord &ir) {
			indexFile->seekp(i * sizeof(IndexRecord));
			indexFile->write((char *) (&ir), sizeof(IndexRecord));
		}

		void checkFile() {
			if (!indexFile->is_open()) {
				throw DBException("File not open");
			}
		}


	public:
		IntIndex(string fileName){
			// hint: Open file and read the root of btree and head of freelist
			//       The file is nothing but IndexRecords but they can either
			//       be BTreeNodes or FreeListNodes

			struct stat s;
			ofstream t;

			cout << "Create int index" << endl;

			if (stat(fileName.c_str(), &s) != 0) {
				cout << "Creating file: " << fileName << endl;
				t.open(fileName.c_str());
				t.close();

				indexFile = new fstream(fileName.c_str());
				indexFile->open(fileName);
				checkFile();

				root.btn.init();
				head.fln.init();

				writeAt(0, root);
				writeAt(1, head);

			} else {

				cout << "Opening file: " << fileName << endl;
				indexFile = new fstream(fileName.c_str());
				indexFile->open(fileName);
				checkFile();

				// readAt(0, root);
				// readAt(1, head);
			}
			
		
			root.btn.init(); // <-- temp
			head.fln.init(); // <-- temp
		}

		int getSize() {
			indexFile->seekg(0, ios::end);
			return indexFile->tellg();
		}

		int getNumRecords() {
			return getSize() / sizeof(IndexRecord);
		}

		int findByKey(int key){
			BTreeNode *element = root.btn.findByKey(key);

			if (element != NULL) {
				return element->getLocation();
			} else {
				return -1;
			}
		}

		void set(int key, int location) {  // another name for add
			BTreeNode element;
			element.init(key, location);
			root.btn.add(&element);  // just update to data
		}

		void add(int key, int location) { 
			// hint: line below needs to check that a element is truely
			//       added or is it just updating an existing record

			BTreeNode element;
			element.init(key, location);
			root.btn.add(&element);
		}

		void del(int key) {
			BTreeNode search;
			search.init(key);
			// BTreeNode *element=root.btn.findByKey(key);

			root.btn.del(&search);
			//hint: add element BTreeNode you are delete to freeList
		}

		~IntIndex() {
			// hint: Write the root and head to first two records and 
			//       then close the file

			cout << "Destroying int index" << endl;

			writeAt(0, root);
			writeAt(1, head);
			indexFile->close();

			delete indexFile;

			cout << "Int index destroyed" << endl;
		}

		friend ostream & operator <<(ostream &out,IntIndex &info) {
			return out << "IntIndex information" << endl 
					   << "BTreeNodes:"          << endl 
					   << "ROOT"
					   << &(info.root.btn) 
					   << "FreeListNodes:"       << endl 
					   << "HEAD"
					   << &(info.head.fln)       << endl;
		}
};




int main(int argc, char *argv[]) {
	if (argc > 1) {
		if (strcmp(argv[1], "BT") == 0) {
			BTreeNodeTest();
		} else if (strcmp(argv[1],"FL") == 0) {
			FreeListNodeTest();
		} else if (strcmp(argv[1],"INT") == 0) {
			IntIndexTest();
		} else {
			cout << "Usage: " << argv[0] << " [BT|FL|INT|<Nothing>]" << endl;
		}

		return 0;
	}

	IntIndex index = IntIndex("intIndex.idx");
	
	return 0;
}


void IntIndexTest() {
	IntIndex index = IntIndex("intIndex.idx");

	index.add(3, 1);
	index.add(1, 2);
	index.add(6, 7);
	index.add(8, 9);
	index.add(12, 14);

	cout << "\nBefore:" << endl;
	cout << index;

	// should have 1 item on free list 
	index.del(8);
	cout << "After 1 Free List Item:" << endl << index;

	// should have 2 item on free list 
	index.del(6);
	cout << "After 2 Free List Items" << endl << index;
	
	// should have 1 item on free list 
	index.add(75, 12);
	cout << "After Back to 1 Free List Item" << endl << index;
}


void BTreeNodeTest(){

	BTreeNode *root=new BTreeNode();
	root->init(3,1);
	//BTreeNode *root=new BTreeNode(3,1);

	BTreeNode *a,*b,*c,*d;
	a=new BTreeNode();
	a->init(1,2);
	b=new BTreeNode();
	b->init(6,7);
	c=new BTreeNode();
	c->init(8,9);
	d=new BTreeNode();
	d->init(12,14);
	root->add(a);
	root->add(b);
	root->add(c);
	root->add(d);
	cout << "Before BTree" << endl << root;
	BTreeNode search;
	search.init(8);
	root->del(&search);
	cout << "After BTree" << endl << root;
    root->destroy();
}


void FreeListNodeTest() {
    FreeListNode *froot=new FreeListNode();
    froot->init();
    froot->push(new FreeListNode());
    froot->push(new FreeListNode());
    froot->push(new FreeListNode());
    cout << "Before FreeList" << endl;
    cout << froot;
    FreeListNode *used=froot->pop();
    cout << used;
    free(used);
    
    used=froot->pop();
    cout << used;
    free(used);
    
    used = froot->pop();
    cout << used;
    free(used);
    
	cout << "After FreeList" << endl << froot;
    cout << froot;
    delete froot;
}
