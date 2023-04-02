/**
 * @file main.h
 * @author James Halladay
 * 
 * Class: Database Design
 * Professory: Karl Castleton
 * 
 * @brief This is the header file for managing the index file for our database
 * @version 0.1
 * @date 2023-03-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>
#include <sys/stat.h>

using namespace std;

int basicTest();

enum RecordType {
    FREE, 
    OCCUPIED,
    INVALID
};

class DBException {
    private:
        string errorMessage;
    public:
        DBException(string msg);
        string message();
};

class FreeListNode {
    public:
        RecordType type;
        long location;
        void init(long memLocation, long nextLocation);
        long getLocation();
    private:
        long nextLocation;
        // FreeListNode();
    public:
        long getNextLocation();
        // void setLocation(int newLocation);

        void push(int newLocation);
        int pop();
        bool isEmpty();

        friend ostream & operator << (ostream& out, const FreeListNode& node);
};

class TreeNode {
    public: // Both the tree and free list need to have theese properties at the top of their memory block

        // properties
        RecordType type;
        long location;

        // constructors
        // void init(long memLocation);
        void init(long memLocation, bool root);
        void init(long key, long value, long memLocation, long left, long right);
        void from(long memLocation);

    private:

        // properties
        bool root;
        // bool leaf;
        long leftLocation;
        long rightLocation;
        long key;
        long value;

        // setters
        void copy(TreeNode &node);

        // manipulation
        void addNode(TreeNode &node); // used internally by del

    public:

        // getters 
        bool isRoot();
        bool isLeaf();
        bool isValid();
        long getKey();
        long getValue();
        long getLocation();
        long getLeftLocation();
        long getRightLocation();

        // setters
        void setRoot(bool isRoot);

        // manipulation
        void save();
        void add(long key, long value);
        bool del(long key);
        long findByKey(long key);

        // destructors
        void freeNode();
        void invalidate();
        void destroy();
        // ~TreeNode();

        // operators
        bool operator < (TreeNode &node) const;
        friend ostream & operator << (ostream &out, const TreeNode &node);
};

union IndexRecord {
    FreeListNode freeNode;
    TreeNode treeNode;
};

class MemoryManager {
    private:
        int blockSize;
        string fileName;
        fstream *file;
        void checkFile();
        IndexRecord FreeListHead;
        bool hasFreeListHead;
        bool hasTreeRoot;

        
    public:
        MemoryManager(string fileName);
        ~MemoryManager();
        void FreeListInit();
        int getBlockSize();
        void readAt(int location, IndexRecord &record);
        void readAt(int location, FreeListNode &record);
        void readAt(int location, TreeNode &record);
        void writeAt(int location, IndexRecord record);
        void writeAt(int location, FreeListNode record);
        void writeAt(int location, TreeNode record);
        int getSize();
        int getNumLocations();
        void freeLocation(long location);
        long getNextFreeLocation();

        bool test();
        
};

class IntIndex {

};



// colors found here: https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
string highlightRed(string s) {
	return "\033[1;31m" + s + "\033[0m";
}

string highlightGreen(string s) {
	return "\033[1;32m" + s + "\033[0m";
}

string highlightYellow(string s) {
	return "\033[1;33m" + s + "\033[0m";
}

string highlightCyan(string s) {
	return "\033[1;36m" + s + "\033[0m";
}