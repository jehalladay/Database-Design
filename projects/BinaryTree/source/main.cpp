/**
 * @file main.cpp
 * @author James Halladay 
 * 
 * Class: Database Design
 * Professor: Karl Castleton
 * 
 * @brief This program will manage an index file for the primary key for a database
 * 
 * 
 * @details
 *      This program will create a structure on disk that will store 
 *          the key for a database along with the location of the record
 *          in the database file.
 * 
 *      We will use a binary tree to store the key and a linked list to 
 *          store the location of free space in the index file.
 * 
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
#include "main.h"

using namespace std;

bool TreeNodeTest();
bool FreeListNodeTest();
bool IntIndexTest();
bool MemoryManagerTest();

MemoryManager *mm;


DBException::DBException(string msg) {
    cerr << highlightRed("DBException: " + msg);
    errorMessage = msg;
}

string DBException::message() {
    return errorMessage;
}

void FreeListNode::init(long memLocation, long nextLocation = -1) {
    type = FREE;
    location = memLocation;
    this->nextLocation = nextLocation;
    mm->writeAt(location, *this);
}

void FreeListNode::push(int newLocation) {
    FreeListNode newNode;
    newNode.init(newLocation, nextLocation);
    nextLocation = newLocation;
    mm->writeAt(newLocation, newNode);
    mm->writeAt(location, *this);
}


int FreeListNode::pop() {
    IndexRecord newNode;
    int result = nextLocation;

    if (nextLocation < 0) {
        throw DBException("Cannot pop from empty list");
    }

    mm->readAt(nextLocation, newNode);
    nextLocation = newNode.freeNode.nextLocation;
    mm->writeAt(location, *this);

    return result;
};

bool FreeListNode::isEmpty() {
    return nextLocation == -1;
};

long FreeListNode::getLocation() {
    return location;
};

long FreeListNode::getNextLocation() {
    return nextLocation;
};

void TreeNode::init(long memLocation, bool root = true) {
    type = INVALID;
    location = memLocation;
    key = -1;
    value = -1;
    leftLocation = -1;
    rightLocation = -1;

    save();
    setRoot(root);
}   

void TreeNode::init(long memLocation, long key, long value, long left = -1, long right = -1) {
    if (memLocation < 0) {
        throw DBException("Cannot initialize node at negative location");
    } else if (memLocation == 0) {
        throw DBException("Cannot initialize node at Free List Head location");
    }

    type = OCCUPIED;
    this->key = key;
    this->value = value;
    
    leftLocation = left;
    rightLocation = right;
    location = memLocation;
    mm->writeAt(location, *this);
}

long TreeNode::getLocation() {
    return location;
}

void TreeNode::setRoot(bool isRoot) {
    this->root = isRoot;
}

bool TreeNode::isRoot() {
    return root;
}

bool TreeNode::isLeaf() {
    return leftLocation < 0 && rightLocation < 0;
}

void TreeNode::save() {
    mm->writeAt(location, *this);
}

long TreeNode::getKey() {
    return key;
}

long TreeNode::getValue() {
    return value;
}

long TreeNode::getLeftLocation() {
    return leftLocation;
}

long TreeNode::getRightLocation() {
    return rightLocation;
}

bool TreeNode::isValid() {
    return type == OCCUPIED;
}

void TreeNode::invalidate() {
    type = INVALID;
    key = -1;
    value = -1;
    leftLocation = -1;
    rightLocation = -1;

    mm->writeAt(location, *this);
}

void TreeNode::copy(TreeNode &node) {

    key = node.getKey();
    value = node.getValue();
    type = node.type;
    leftLocation = node.getLeftLocation();
    rightLocation = node.getRightLocation();
    // leaf = node.isLeaf();

    // cout << "Copied node " << key << " from " << node.getKey() << endl;

    save();
    // cout << "Copied node " << key << " from " << node.getKey() << endl;
}

void TreeNode::freeNode() {
    if (root) {
        invalidate();
    } else {
        mm->freeLocation(location);
    }
}

void TreeNode::from(long memLocation) {
    mm->readAt(memLocation, *this);
}

/**
 * @brief Add a new key value pair to the tree
 * 
 * @param newKey 
 * @param newValue 
 */
void TreeNode::add(long newKey, long newValue) {
    if (!isValid() && root) {
        key = newKey;
        value = newValue;
        type = OCCUPIED;
        save();

    } else if (key == newKey) {
        value = newValue;
        save();        

    } else if (key > newKey) {
        if (leftLocation < 0) {
            leftLocation = mm->getNextFreeLocation();
            TreeNode newNode;
            newNode.init(leftLocation, newKey, newValue);
            // leaf = false;
        } else {
            IndexRecord newNode;
            mm->readAt(leftLocation, newNode);
            newNode.treeNode.add(newKey, newValue);
        }

        save();        
        
    } else if (key < newKey) {
        if (rightLocation < 0) {
            rightLocation = mm->getNextFreeLocation();
            TreeNode newNode;
            newNode.init(rightLocation, newKey, newValue);
            // leaf = false;
        } else {
            IndexRecord newNode;
            mm->readAt(rightLocation, newNode);
            newNode.treeNode.add(newKey, newValue);
        }

        save();
    }
}


/**
 * @brief Function will add a tree node and its children to the current node
 * 
 * @param node 
 */
void TreeNode::addNode(TreeNode &node) {
    
    // Case 1: empty tree and current node is root
    if (!isValid() && root) { 
        copy(node);
        node.freeNode();


    // Case 2: key already exists and node is a leaf
    } else if (key == node.getKey() && node.getLeftLocation() < 0 && node.getRightLocation() < 0) {  
        value = node.getValue();

        node.freeNode();

    // Case 3: key already exists and input node has a right child
    } else if (key == node.getKey() && node.getLeftLocation() < 0) { 
        // set current nodes values to input node, except for children locations
        value = node.getValue();

        // now we check to see if the current node has a right child
        if (rightLocation < 0) { // if not, we just set the right child to the input nodes right child
            rightLocation = node.getRightLocation();

        } else { // if so, we add the input nodes right child to the current nodes right child
            IndexRecord nodeRight, currentRight;

            mm->readAt(node.getRightLocation(), nodeRight);
            mm->readAt(rightLocation, currentRight);

            currentRight.treeNode.addNode(nodeRight.treeNode);
            node.freeNode();
        }
        
    // Case 4: key already exists and input node has a left child
    } else if (key == node.getKey() && node.getRightLocation() < 0) { 
        // set current nodes values to input node, except for children locations
        value = node.getValue();

        // now we check to see if the current node has a left child
        if (leftLocation < 0) { // if not, we just set the left child to the input nodes left child
            leftLocation = node.getLeftLocation();

        } else { // if so, we add the input nodes left child to the current nodes left child
            IndexRecord nodeLeft, currentLeft;

            mm->readAt(node.getLeftLocation(), nodeLeft);
            mm->readAt(leftLocation, currentLeft);

            currentLeft.treeNode.addNode(nodeLeft.treeNode);
            node.freeNode();
        }


    // Case 5: key already exists and input node has both children
    } else if (key == node.getKey()) { 
        // set current nodes values to input node, except for children locations
        value = node.getValue();

        // now we check to see if the current node has a left child
        if (leftLocation < 0) {
            leftLocation = node.getLeftLocation();

        } else {
            IndexRecord nodeLeft, currentLeft;

            mm->readAt(node.getLeftLocation(), nodeLeft);
            mm->readAt(leftLocation, currentLeft);

            currentLeft.treeNode.addNode(nodeLeft.treeNode);
            node.freeNode();
        }

        // now we check to see if the current node has a right child
        if (rightLocation < 0) {
            rightLocation = node.getRightLocation();

        } else {
            IndexRecord nodeRight, currentRight;

            mm->readAt(node.getRightLocation(), nodeRight);
            mm->readAt(rightLocation, currentRight);

            currentRight.treeNode.addNode(nodeRight.treeNode);
            node.freeNode();
        }

    // Case 6: input node key is less than current node key, goes on left side
    } else if (key > node.getKey()) { 
    
        if (leftLocation < 0) {
            leftLocation = node.getLocation();
            // leaf = false;

        } else {
            IndexRecord newNode;

            mm->readAt(leftLocation, newNode);
            
            newNode.treeNode.addNode(node);
        }

    // Case 7: input node key is greater than current node key, goes on right side
    } else if (key < node.getKey()) { 

        if (rightLocation < 0) {
            rightLocation = node.getLocation();
            // leaf = false;
        } else {
            IndexRecord newNode;

            mm->readAt(rightLocation, newNode);

            newNode.treeNode.addNode(node);
        }
    }

    save();
}

/**
 * @brief Delete a key value pair from the tree
 * 
 * Note: if a non-root node is deleted, we will give the location 
 *          back to the free list
 *        if a root node is deleted, we will invalidate it
 *        
 * 
 * @param key 
 * @return true -- only if key == this->key and this node is a leaf 
 */
bool TreeNode::del(long key) {
    bool result = false; 

    // cout << "Delete\tkey: " << key << " from node: " << location << " with key: " << this->key << endl;

    if (this->key == key) {
        
        // Case 1: root node with no children
        if (leftLocation < 0 && rightLocation < 0 && root) { 
            // cout << "Case 1: root node with no children" << endl;

            freeNode();
            result = true;
            
        // Case 2: non-root node with no children
        } else if (leftLocation < 0 && rightLocation < 0) { 
            // cout << "Case 2: non-root node with no children" << endl;

            freeNode();
            result = true;

        // Case 3: current node only has right child
        } else if (leftLocation < 0) { 
            IndexRecord rightNode;
            // cout << "Case 3: current node only has right child" << endl;

            rightNode.treeNode.from(rightLocation);
            copy(rightNode.treeNode);

            rightNode.treeNode.freeNode();
            save();
            
        // Case 4: current node only has left child
        } else if (rightLocation < 0) { 
            IndexRecord leftNode;
            // cout << "Case 4: current node only has left child" << endl;

            leftNode.treeNode.from(leftLocation);
            copy(leftNode.treeNode);

            leftNode.treeNode.freeNode();
            save();

        // Case 5: current node has two children
        } else { 
            IndexRecord leftNode, rightNode;
            // cout << "Case 5: current node has two children" << endl;

            leftNode.treeNode.from(leftLocation);
            rightNode.treeNode.from(rightLocation);

            leftNode.treeNode.addNode(rightNode.treeNode);
            copy(leftNode.treeNode);
            save();
        }

    } else if (key < this->key) {
        // cout << "Case 6: current node key > input key" << endl;

        // case 1: current node has no left child
        if (leftLocation < 0) { 
            throw DBException("Cannot delete key from node with no left child, key does not exist");
        
        // case 2: current node has left child
        } else {

            IndexRecord leftNode;
            bool tempResult = false;
            mm->readAt(leftLocation, leftNode);

            tempResult = leftNode.treeNode.del(key);

            if(tempResult) {
                this->leftLocation = -1;
            }
            
            save();
        }
        
    } else if (key > this->key) {
        // cout << "Case 7: current node key < input key" << endl;

        // case 1: current node has no right child
        if (rightLocation < 0) { 
            throw DBException("Cannot delete key from node with no right child, key does not exist");
        
        // case 2: current node has right child
        } else {

            IndexRecord rightNode;
            bool tempResult = false;
            mm->readAt(rightLocation, rightNode);

            tempResult = rightNode.treeNode.del(key);

            if(tempResult) {
                this->rightLocation = -1;
            }

            save();
        }
    }

    return result;
}


/* ************************ untested and uninspected code below ************************ */
void TreeNode::destroy() {

}





void MemoryManager::checkFile() {
    cout << "Start\tMemoryManager::checkFile()" << endl;

    if (file->is_open()) {
        cout << "Check\tFile is open" << endl;
    } else {
        cout << "Check\tFile is not open" << endl;
    }

    cout << "End\tMemoryManager::checkFile()" << endl;
}

MemoryManager::MemoryManager(string fileName) {
    struct stat s;
    ofstream t;

    this->fileName = fileName;
    blockSize = sizeof(IndexRecord);
    hasFreeListHead = false;
    hasTreeRoot = false;

    cout << "Create\tMemory Manager" << endl;

    if (stat(fileName.c_str(), &s) != 0) {
        cout << "Create\tfile: " << fileName << endl;
        t.open(fileName.c_str());
        t.close();

        file = new fstream;
        file->open(fileName);
        
        checkFile();

    } else {
        cout << "Opening file: " << fileName << endl;
        file = new fstream(fileName.c_str());
        file->open(fileName);

        checkFile();
    }
}



MemoryManager::~MemoryManager() {
    cout << "Destroy\tMemory Manager" << endl;
    file->close();
    delete file;
}

int MemoryManager::getBlockSize() {
    return blockSize;
}

void MemoryManager::writeAt(int location, IndexRecord record) {
    if (location >= 0) {
        file->seekp(location * blockSize);
        file->write((char*) (&record), blockSize);
    } else {
        throw DBException("Invalid location");
    }
}

void MemoryManager::writeAt(int location, FreeListNode fln) {
    if (location >= 0) {
        file->seekp(location * blockSize);
        file->write((char*) (&fln), blockSize);
    } else {
        throw DBException("Invalid location");
    }
}

void MemoryManager::writeAt(int location, TreeNode tn) {
    if (location >= 0) {
        file->seekp(location * blockSize);
        file->write((char*) (&tn), blockSize);
    } else {
        throw DBException("Invalid location");
    }
}

void MemoryManager::readAt(int location, IndexRecord &record) {
    if (location >= 0) {
        file->seekg(location * blockSize);
        file->read((char*) (&record), blockSize);
    } else {
        throw DBException("Invalid location");
    }
}

void MemoryManager::readAt(int location, FreeListNode &fln) {
    if (location >= 0) {
        file->seekg(location * blockSize);
        file->read((char*) (&fln), sizeof(FreeListNode));
        // file->read((char*) (&fln), blockSize);
    } else {
        throw DBException("Invalid location");
    }
}

void MemoryManager::readAt(int location, TreeNode &tn) {
    if (location >= 0) {
        file->seekg(location * blockSize);
        file->read((char*) (&tn), sizeof(TreeNode));
        // file->read((char*) (&tn), blockSize);
    } else {
        throw DBException("Invalid location");
    }
}

void MemoryManager::FreeListInit() {
    if (!hasFreeListHead) {
        FreeListHead.freeNode.init(0);
        hasFreeListHead = true;
    } else {
        throw DBException("Free List already initialized");
    }
}

int MemoryManager::getSize() {
    file->seekg(0, ios::end);
    int size = file->tellg();
    return size;
}

int MemoryManager::getNumLocations() {
    return getSize() / blockSize;
}


/**
 * @brief returns the next free location in the file
 * 
 * Danger!! This function is destructive!!!!!!!!!!!!!
 * Do not use outside of the Binary Tree class
 * 
 * If the result is not used to write to the file, the location will be lost.
 * 
 * @return long 
 */
long MemoryManager::getNextFreeLocation() {
    long result = -1;

    if (FreeListHead.freeNode.isEmpty()) {
        result = getNumLocations();
    } else {
        result = FreeListHead.freeNode.pop();
    }

    if (result < 0) {
        throw DBException("Memory Manager gave invalid location");
    }

    return result;
}

/**
 * @brief function will free a location in the file
 * 
 * Danger!! This function is destructive!!!!!!!!!!!!!
 * Do not use outside of the Binary Tree class
 * 
 * if the result passed in is already in the free list or is currently being used by
 *      the tree, undefined behavior will occur
 * 
 * @param location 
 */
void MemoryManager::freeLocation(long location) {
    if (location == 0 && hasFreeListHead) {
        throw DBException("Cannot free head node of the free list");
    } else if (location == 1 && hasFreeListHead) {
        throw DBException("Cannot free root node of the tree");
    } else if (location < 0) {
        throw DBException("Cannot free invalid location");
    }

    FreeListHead.freeNode.push(location);
}


/* ***************************************************** */
/*                          Main                        */
/* ***************************************************** */

int main(int argc, char const *argv[]) {
    string fileName = "IntIndex.idx";

    if (argc > 1) {
        fileName = argv[1];
    }

    basicTest();
    MemoryManagerTest();
    FreeListNodeTest();
    TreeNodeTest();
}


/* ***************************************************** */
/*                          Tests                        */
/* ***************************************************** */


int basicTest() {
    cout << "hello from test" << endl;
    return 0;
}



bool MemoryManager::test() {
    const int tests = 7;
    bool pass[tests], allPass = true;
    // IndexRecord record, fln;
    IndexRecord record, rec2;
    FreeListNode fln;
    int location, testNum = 0;
    string message = "";
    
    cout << "Start\tMemoryManager::test()" << endl;
    
    /*
        First, We test the basic 
    */
    
    {   // We test blockSize = sizeof(IndexRecord)

        // setup
        message = "\tTest " + to_string(testNum + 1) + ": Block Size == sizeof(IndexRecord):  ";
        cout << highlightCyan(message) << sizeof(IndexRecord) << endl;
        pass[testNum] = false;

        // execute
        pass[testNum] = getBlockSize() == sizeof(IndexRecord);

        // cleanup
        cout << "\t\t" << (pass[testNum]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
        testNum++;
    } 

    {   // We test that the file is open
    
        // setup
        message = "\tTest " + to_string(testNum + 1) + ": File is open: ";
        cout << highlightCyan(message) << endl;
        pass[testNum] = false;

        // execute
        pass[testNum] = file->is_open();

        // cleanup
        cout << "\t\t" << (pass[testNum]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
        testNum++;
    }


    {   // We test getNumLocations
    
        // setup
        message = "\tTest " + to_string(testNum + 1) + ": getNumLocations: ";
        cout << highlightCyan(message) << endl;
        pass[testNum] = false;
        
        // execute
        pass[testNum] = getNumLocations() == 0;

        // cleanup
        cout << "\t\t" << (pass[testNum]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
        testNum++;
    }


    {   // We test writing to the index file
    
        // setup
        message = "\tTest " + to_string(testNum + 1) + ": Writing to the index file:";
        cout << highlightCyan(message) << endl;
        pass[testNum] = false;
        location = 2;

        // execute
        record.freeNode.init(location);
        writeAt(location, record);
        pass[testNum] = getNumLocations() == (location + 1);

        // cleanup
        cout << "\t\t" << (pass[testNum]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
        testNum++;
    }


    {   // We test reading from the index file
    
        // setup
        message = "\tTest " + to_string(testNum + 1) + ": Reading from the index file: ";
        cout << highlightCyan(message) << endl;
        pass[testNum] = false;
     
        // execute
        readAt(location, fln); // we test on both a FreeListNode and an IndexRecord
        readAt(location, rec2);

        pass[testNum] = rec2.freeNode.type              == record.freeNode.type
                     && rec2.freeNode.location          == record.freeNode.location
                     && rec2.freeNode.getNextLocation() == record.freeNode.getNextLocation()
                     && fln.type                        == record.freeNode.type
                     && fln.location                    == record.freeNode.location
                     && fln.getNextLocation()           == record.freeNode.getNextLocation();

        // cleanup
        cout << "\t\t" << (pass[testNum]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
        testNum++;
    }


    {   // We test FreeListInit and memory management
    
        // setup
        message = "\tTest " + to_string(testNum + 1) + ": FreeListInit and memory management: ";
        cout << highlightCyan(message) << endl;
        pass[testNum] = false;
        
        // execute
        FreeListInit();
        readAt(0, fln);
        pass[testNum] = fln.type == FreeListHead.freeNode.type
               && fln.location == FreeListHead.freeNode.location
               && fln.getNextLocation() == FreeListHead.freeNode.getNextLocation();
               
        // cleanup
        cout << "\t\t" << (pass[testNum]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
        testNum++;
    }

    {   // We test getNextFreeLocation, its important to test when it is a root node
    
        // setup
        message = "\tTest " + to_string(testNum + 1) + ": getNextFreeLocation: ";
        cout << highlightCyan(message) << endl;
        pass[testNum] = false;

        // execute
        pass[testNum] = getNextFreeLocation() == location + 1 /* non-destructive */
                     && getNextFreeLocation() == location + 1; /* non-destructive */
        // cleanup
        cout << "\t\t" << (pass[testNum]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
        testNum++;
    }

    {   // We test free location 

        // setup
        message = "\tTest " + to_string(testNum + 1) + ": freeLocation: ";
        cout << highlightCyan(message) << endl;
        pass[testNum] = false;

        // execute
        freeLocation(location);
        pass[testNum] = getNextFreeLocation() == location /* destructive */
                        && getNumLocations() == location + 1
                        && getNextFreeLocation() == location + 1; /* non-destructive */

        // cleanup
        cout << "\t\t" << (pass[testNum]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
        testNum++;
    }

    for(int i = 0; i < tests; i++) {
        allPass = allPass && pass[i];
    }

    cout << "\t" << (allPass? highlightGreen("All Tests Passed"): highlightRed("Some Tests Failed")) << endl;
    cout << "End\tMemoryManager::test()" << endl;

    return allPass;
}


bool MemoryManagerTest() {
    bool pass = true;
    
    cout << highlightGreen("\nMemoryManager Test") << endl;
    
    mm = new MemoryManager("test.idx");
    pass = mm->test();
    delete mm;

    cout << (pass? highlightGreen("MemoryManager Test Passed"): highlightRed("MemoryManager Test Failed")) << endl << endl;
    
    return pass;
}


bool FreeListNodeTest() {
    string dbFile = "FreeTest.idx";
    int tests = 7, retval = -2;
    bool pass[tests], allPass = true;
    IndexRecord record;


    cout << highlightGreen("\nFreeListNode Test") << endl;
    mm = new MemoryManager(dbFile);
    
    {   // We test the creation of the head free node
        // setup
        cout << highlightCyan("\tTest 1: Creation of the head free node: ") << endl;
        pass[0] = false;

        // execute
        record.freeNode.init(0);
        pass[0] = record.freeNode.type == FREE 
            && record.freeNode.location == 0
            && record.freeNode.getNextLocation() == -1;

        // cleanup
        cout << "\t\t" << (pass[0]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
    }


    {   // We test the isEmpty method when list is empty
        // setup
        cout << highlightCyan("\tTest 2: isEmpty method when list is empty: ") << endl;
        pass[1] = false;

        // execute
        pass[1] = record.freeNode.isEmpty();


        // cleanup
        cout << "\t\t" << (pass[1]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
    }

    {   // We test pushing a new location onto the free list
        // setup
        cout << highlightCyan("\tTest 3: Pushing a new location onto the free list:") << endl;
        pass[2] = false;

        // execute
        record.freeNode.push(2);
        pass[2] = record.freeNode.type == FREE 
            && record.freeNode.location == 0
            && record.freeNode.getNextLocation() == 2;

        // cleanup
        cout << "\t\t" << (pass[2]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
    }

    {   // We test isEmpty method when list is not empty
        // setup
        cout << highlightCyan("\tTest 4: isEmpty method when list is not empty: ") << endl;
        pass[3] = false;

        // execute
        pass[3] = !record.freeNode.isEmpty();

        // cleanup
        cout << "\t\t" << (pass[3]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
    }

    {   // We test pushing another element onto the free list
        // setup
        cout << highlightCyan("\tTest 5: Pushing another element onto the free list: ") << endl;
        pass[4] = false;

        // execute
        record.freeNode.push(7);
        pass[4] = record.freeNode.type == FREE 
            && record.freeNode.location == 0
            && record.freeNode.getNextLocation() == 7;

        // cleanup
        cout << "\t\t" << (pass[4]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
    }


    {   // We test popping a location off the free list
        // setup
        cout << highlightCyan("\tTest 6: Popping a location off the free list: ") << endl;
        pass[5] = false;

        // cout << record.freeNode.nextLocation << " " << retval << endl;
        // execute
        retval = record.freeNode.pop();
        pass[5] = record.freeNode.type == FREE 
            && record.freeNode.location == 0
            && record.freeNode.getNextLocation() == 2
            && retval == 7;


        // cout << record.freeNode.nextLocation << " " << retval << endl;
        // cleanup
        cout << "\t\t" << (pass[5]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
        retval = -2;
    }


    {   // We test that we can recover items pushed onto the free list in reverse order
        // setup
        cout << highlightCyan("\tTest 7: Recovering items pushed onto the free list in reverse order: ") << endl;
        pass[6] = true;
        int items[10] = {
            11, 2, 3, 4, 5, 6, 7, 8, 9, 10
        };

        // execute
        for(int i = 0; i < 10; i++) {
            record.freeNode.push(items[i]);
        }

        for(int i = 9; i >= 0; i--) {
            retval = record.freeNode.pop();
            pass[6] = pass[6] && (retval == items[i]);
        }

        // cleanup
        cout << "\t\t" << (pass[6]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
    }

    for(int i = 0; i < tests; i++) {
        allPass = allPass && pass[i];
        if (!pass[i]) {
            cout << "\t\tTest " << i+1 << " failed" << endl;
        }
    }

    cout << (allPass? highlightGreen("FreeListNode Test Passed"): highlightRed("FreeListNode Test Failed")) << endl << endl;

    return allPass;
}


bool TreeNodeTest() {
    string dbFile = "TreeTest.idx";
    const int tests = 5, numRecords = 7;
    long keys[numRecords], values[numRecords];
    bool pass[tests], allPass = true, tempResult = false;
    IndexRecord firstRecord, secondRecord, thirdRecord;
    int location, testNum = 0;
    string message = "";
    
    cout << highlightGreen("\nTreeNode Test") << endl;
    mm = new MemoryManager(dbFile);
    mm->FreeListInit();

    {   // We test the creation of the root of our binary tree

        // setup
        message = "\tTest " + to_string(testNum + 1) + ": Creation of the root of our binary tree: ";
        cout << highlightCyan(message) << endl;
        pass[testNum] = false;
        location = mm->getNextFreeLocation();


        // execute
        firstRecord.treeNode.init(location, false);
        secondRecord.treeNode.from(location);
        
        pass[testNum] = firstRecord.treeNode.type               == secondRecord.treeNode.type
                     && firstRecord.treeNode.getLocation()      == secondRecord.treeNode.getLocation()
                     && firstRecord.treeNode.getLeftLocation()  == secondRecord.treeNode.getLeftLocation()
                     && firstRecord.treeNode.getRightLocation() == secondRecord.treeNode.getRightLocation()
                     && firstRecord.treeNode.getKey()           == secondRecord.treeNode.getKey()
                     && firstRecord.treeNode.getValue()         == secondRecord.treeNode.getValue()
                     && location                                == 1 /* The root must always be placed after the free list head */
                     && firstRecord.treeNode.isLeaf()
                     && !firstRecord.treeNode.isRoot();

        firstRecord.treeNode.setRoot(true);
        pass[testNum] = pass[testNum] && firstRecord.treeNode.isRoot();

        // cleanup
        cout << "\t\t" << (pass[testNum]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
        testNum++;
    } 

    {   // We test adding a key value pair to the root of our binary tree

        // setup
        message = "\tTest " + to_string(testNum + 1) + ": Adding a key value pair to the root of our binary tree: ";
        cout << highlightCyan(message) << endl;
        pass[testNum] = false; 
        keys[0] = 20;
        values[0] = 12;

        // execute
        firstRecord.treeNode.add(keys[0], values[0]);
        pass[testNum] = firstRecord.treeNode.getKey()   == keys[0]
                     && firstRecord.treeNode.getValue() == values[0]
                     && mm->getNumLocations()           == 2
                     && firstRecord.treeNode.isLeaf();


        // cleanup
        cout << "\t\t" << (pass[testNum]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
        testNum++;
    }

    {   // We test adding a left node to the root of our binary tree

        // setup
        message = "\tTest " + to_string(testNum + 1) + ": Adding a left node to the root of our binary tree: ";
        cout << highlightCyan(message) << endl;
        pass[testNum] = false;
        keys[1] = 10;
        values[1] = 11;

        // execute
        firstRecord.treeNode.add(keys[1], values[1]);
        pass[testNum] = firstRecord.treeNode.getLeftLocation()  != -1
                     && firstRecord.treeNode.getRightLocation() == -1
                     && mm->getNumLocations()                   == 3
                     && !firstRecord.treeNode.isLeaf();

        if (pass[testNum]) {
            mm->readAt(firstRecord.treeNode.getLeftLocation(), secondRecord);
            pass[testNum] = secondRecord.treeNode.getKey()   == keys[1]
                         && secondRecord.treeNode.getValue() == values[1]
                         && secondRecord.treeNode.isLeaf();
        }


        // cleanup
        cout << "\t\t" << (pass[testNum]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
        testNum++;
    }

    {   // we test adding a right node to the root of our binary tree
    
        // setup
        message = "\tTest " + to_string(testNum + 1) + ": Adding a right node to the root of our binary tree: ";
        cout << highlightCyan(message) << endl;
        pass[testNum] = false;
        keys[2] = 30;
        values[2] = 13;

        // execute
        firstRecord.treeNode.add(keys[2], values[2]);
        pass[testNum] = firstRecord.treeNode.getLeftLocation()  != -1
                     && firstRecord.treeNode.getRightLocation() != -1
                     && mm->getNumLocations()                   == 4
                     && !firstRecord.treeNode.isLeaf();

        if (pass[testNum]) {
            mm->readAt(firstRecord.treeNode.getRightLocation(), secondRecord);
            pass[testNum] = secondRecord.treeNode.getKey()   == keys[2]
                         && secondRecord.treeNode.getValue() == values[2]
                         && secondRecord.treeNode.isLeaf();
        }

        // cleanup
        cout << "\t\t" << (pass[testNum]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
        testNum++;
    }


    {   // We test all paths of the delete function

        // setup
        message = "\tTest " + to_string(testNum + 1) + ": All paths of our del method, ending with the root node: ";
        cout << highlightCyan(message) << endl;
        pass[testNum] = false;
        keys[3] = 40;
        values[3] = 14;
        keys[4] = 5;
        values[4] = 16;
        keys[5] = 15;
        values[5] = 17;

        // execute
        firstRecord.treeNode.add(keys[3], values[3]);


        pass[testNum] = firstRecord.treeNode.getLeftLocation()  != -1
                     && firstRecord.treeNode.getRightLocation() != -1
                     && mm->getNumLocations()                   == 5
                     && !firstRecord.treeNode.isLeaf();


        if (pass[testNum]) {
            cout << highlightYellow("\t\tCases: 7->3") << endl;

            firstRecord.treeNode.del(keys[2]);
            secondRecord.treeNode.from(firstRecord.treeNode.getRightLocation());

            pass[testNum] = secondRecord.treeNode.getKey()   == keys[3]
                         && secondRecord.treeNode.getValue() == values[3]
                         && mm->getNumLocations()            == 5
                         && secondRecord.treeNode.isLeaf();

            cout << "\t\t\t" << (pass[testNum]? highlightGreen("Cases Passed"): highlightRed("Cases Failed")) << endl;
        }

        firstRecord.treeNode.add(keys[4], values[4]);
        firstRecord.treeNode.add(keys[5], values[5]);

        if (pass[testNum]) {
            cout << highlightYellow("\t\tCases: 6->5") << endl;

            // firstRecord.treeNode.from(firstRecord.treeNode.getLocation());

            firstRecord.treeNode.del(keys[1]);
            secondRecord.treeNode.from(firstRecord.treeNode.getLeftLocation());

            pass[testNum] = secondRecord.treeNode.getKey()   == keys[4]
                         && secondRecord.treeNode.getValue() == values[4]
                         && mm->getNumLocations()            == 6
                         && !secondRecord.treeNode.isLeaf();

            cout << "\t\t\t" << (pass[testNum]? highlightGreen("Cases Passed"): highlightRed("Cases Failed")) << endl;
        }

        if(pass[testNum]) {
            cout << highlightYellow("\t\tCases: 6->7->2") << endl;

            firstRecord.treeNode.del(keys[5]);
            secondRecord.treeNode.from(firstRecord.treeNode.getLeftLocation());

            pass[testNum] = secondRecord.treeNode.getKey()   == keys[4]
                         && secondRecord.treeNode.getValue() == values[4]
                         && mm->getNumLocations()            == 6
                         && secondRecord.treeNode.isLeaf();

            cout << "\t\t\t" << (pass[testNum]? highlightGreen("Cases Passed"): highlightRed("Cases Failed")) << endl;
        }

        if(pass[testNum]) {
            cout << highlightYellow("\t\tCases: 7->2") << endl;

            firstRecord.treeNode.del(keys[3]);
            secondRecord.treeNode.from(firstRecord.treeNode.getLeftLocation());

            pass[testNum] = firstRecord.treeNode.getRightLocation() == -1 
                         && secondRecord.treeNode.getKey()          == keys[4]
                         && secondRecord.treeNode.getValue()        == values[4]
                         && mm->getNumLocations()                   == 6
                         && secondRecord.treeNode.isLeaf();
            
            cout << "\t\t\t" << (pass[testNum]? highlightGreen("Cases Passed"): highlightRed("Cases Failed")) << endl;
        }

        if(pass[testNum]) {
            cout << highlightYellow("\t\tCase: 4") << endl;

            firstRecord.treeNode.del(keys[0]);

            pass[testNum] = firstRecord.treeNode.getKey()           == keys[4]
                         && firstRecord.treeNode.getValue()         == values[4]
                         && firstRecord.treeNode.getLeftLocation()  == -1
                         && firstRecord.treeNode.getRightLocation() == -1
                         && mm->getNumLocations()                   == 6
                         && firstRecord.treeNode.isLeaf();

            cout << "\t\t\t" << (pass[testNum]? highlightGreen("Cases Passed"): highlightRed("Cases Failed")) << endl;
        }

        if (pass[testNum]) {
            cout << highlightYellow("\t\tCase: 1") << endl;

            firstRecord.treeNode.del(keys[4]);

            firstRecord.treeNode.from(firstRecord.treeNode.getLocation());

            pass[testNum] = firstRecord.treeNode.getKey()           == -1
                         && firstRecord.treeNode.getValue()         == -1
                         && firstRecord.treeNode.getLeftLocation()  == -1
                         && firstRecord.treeNode.getRightLocation() == -1
                         && mm->getNumLocations()                   == 6
                         && firstRecord.treeNode.isLeaf();

            cout << "\t\t\t" << (pass[testNum]? highlightGreen("Cases Passed"): highlightRed("Cases Failed")) << endl;
        }

        // cleanup
        cout << "\t\t" << (pass[testNum]? highlightGreen("Passed"): highlightRed("Failed")) << endl;
        testNum++;
    }


    for(int i = 0; i < tests; i++) {
        allPass = allPass && pass[i];
    }

    cout << "\t" << (allPass? highlightGreen("All Tests Passed"): highlightRed("Some Tests Failed")) << endl;
    cout << (allPass? highlightGreen("TreeNode Test Passed"): highlightRed("TreeNode Test Failed")) << endl << endl;

    return allPass;
}

// {   // We test

//     // setup

//     // execute

//     // cleanup

// }