#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

const int FIRST_SIZE = 30, LAST_SIZE = 30, ADDRESS_SIZE = 45;
const long NULL_RECORD = -1;

long nextFreeNode = NULL_RECORD;


enum RecordType {
	PERSON,
	FREELIST_NODE
};

class Person{
	private:
		char start;
		float salary;
		char first[FIRST_SIZE], last[LAST_SIZE], address[ADDRESS_SIZE];
		int zip;
		char end;
		
	public:
		RecordType type;

	void init(string newFirst = "", string newLast = "", string newAddress= "", int newZip = 0, float newSalary = 0.0) {
		start = '[';
		end = ']';
		strncpy(first, newFirst.c_str(), FIRST_SIZE);
		strncpy(last, newLast.c_str(), LAST_SIZE);
		strncpy(address, newAddress.c_str(), ADDRESS_SIZE);
		zip = newZip;
		salary = newSalary;
		type = PERSON;
	}

	friend ostream & operator << (ostream &out,const Person &p) {
		return out  << "First: "   << p.first 
					<< " Last: "   << p.last << endl 
					<< "Address: " << p.address
					<< " Zip: "    << p.zip 
					<< " Salary: " << p.salary <<endl;
	}
	
	bool operator < (const Person &p) const {
		if (strncmp(p.last, last, LAST_SIZE) == 0) {
			return strncmp(first, p.first, FIRST_SIZE) == -1;	
		} else {
			return strncmp(last, p.last, FIRST_SIZE) == -1;
		} 
	}
	
	bool operator == (const Person &p) const {
		if (strncmp(p.last, last, LAST_SIZE) == 0) {
			return strncmp(first, p.first, FIRST_SIZE) == 0;
		}
		
		return false;
	}
};


class FreeListNode {
	public:
		long next;
		RecordType type;
		char message[32];
		char start;
		char end;

		void init(long newNext) {
			start = '[';
			end = ']';
			next = newNext;
			type = FREELIST_NODE;

			strcpy(message, "Free List Node");
		}
};

union PersonRecord {
	Person p;
	FreeListNode n;
};

class DBException {
	public:
		string message(){
			return "A database error occurred";
		}
};

fstream dbfile;

void readAt(int i, PersonRecord &p) {
	dbfile.seekg(i*sizeof(PersonRecord));
	dbfile.read((char *)(&p),sizeof(PersonRecord));
}

void writeAt(int i, PersonRecord other) {
	dbfile.seekg(i * sizeof(PersonRecord));
	dbfile.write((char *)(&other), sizeof(PersonRecord));
}

void connectTable(string fname) {
    struct stat s;
	PersonRecord pr;
	ofstream temp;

    if (stat(fname.c_str(), &s) != 0){
		temp.open(fname);
		temp.close();
		dbfile.open(fname);
		pr.n.init(NULL_RECORD);
		writeAt(0, pr); // First record is where we store the next (head of the linked list)
    }  else {
		dbfile.open(fname);
    }

	if (!dbfile.is_open()) {
		throw DBException();
	}

	readAt(0, pr);        // Read first record to get the head of the free list
	nextFreeNode = pr.n.next;
}

void disconnectTable() {
	PersonRecord pr;
	pr.n.init(nextFreeNode);
	writeAt(0, pr); // First record is where we store the next (head of the linked list)	
	dbfile.close();
}

int getNumPeople() {
  dbfile.seekg(0, ios::end);
  int filesize = dbfile.tellg();
  return filesize / sizeof(PersonRecord);
}

/** 
 * nextFreeNode=2 after connection to database 
 *		0 [FreeListNode]  2
 *		1 [Person      ]
 *		2 [FreeListNode]  4
 *		3 [Person      ]
 *		4 [FreeListNode]
 * 
*/
void create(Person p) {  // O(1)
	int i;
	PersonRecord pr;

	if (nextFreeNode == NULL_RECORD) 
		i = getNumPeople(); 
	else {
		readAt(nextFreeNode, pr);
		i = nextFreeNode;
		nextFreeNode = pr.n.next;
	}

	pr.p = p;
	writeAt(i, pr);
}

int find(Person p) {  // O(n)
	int numPeople = getNumPeople();

	for (int i = 0; i < numPeople; i++) {
		PersonRecord otherRecord;
		readAt(i, otherRecord);

		if (otherRecord.p.type == PERSON) {  // Skip freeNodeList records
			Person other = otherRecord.p;

			if (other == p) {
				return i;
			} 
		}
	}

	return NULL_RECORD;
}

Person retrieve(Person p) {  // O(n)
	int numPeople = getNumPeople();

	for (int i = 0; i < numPeople; i++) {
		PersonRecord otherRecord;
		readAt(i, otherRecord);

		if (otherRecord.p.type == PERSON) {  // Skip freeNodeList records
			Person other = otherRecord.p;

			if (other == p) {
				return other;
			} 
		}
	}
	
	return Person();
}

void update(Person p) {  // O(n)
	int numPeople = getNumPeople();

	for (int i = 0; i < numPeople; i++) {
		PersonRecord otherRecord;
		readAt(i, otherRecord);

		if (otherRecord.p.type == PERSON) { // Skip freeNodeList records
			Person other = otherRecord.p;

			if (other == p) {
				PersonRecord pr;
				pr.p = p; 
				writeAt(i, pr);

				break;
			}
		}
	}
}

void del(Person p) {  // O(n)
	int numPeople = getNumPeople();

	for (int i = 0; i < numPeople; i++) {
		PersonRecord otherRecord;
		readAt(i, otherRecord);

		if (otherRecord.p.type == PERSON) {
			Person other = otherRecord.p;
			
			if (other == p) {
				PersonRecord pr;
				pr.n.init(nextFreeNode);
				nextFreeNode = i;
				writeAt(i, pr);

				break;
			}
		}
	}
}

Person p;

int main() {
	try {
		connectTable("TestLinked.bin");

		Person karl;
		karl.init("Karl", "Castleton", "1100 North Avenue", 81501, 50000.0);
		create(karl);

		Person kim;
		kim.init("Kim", "Castleton", "1200 North Avenue", 81502, 60000.0);
		create(kim);

		Person karlKey;
		karlKey.init("Karl", "Castleton");
		Person t = retrieve(karlKey);

		cout << "should show karl castleton 1100 North Avenue, 81501, 50000" << endl;
		cout << t << endl;

		Person karlUpdate;
		karlUpdate.init("Karl", "Castleton", "1100 North Avenue", 81503, 65000.0);
		update(karlUpdate);

		cout << "should show karl castleton 1100 North Avenue, 81501, 65000" << endl;
		t = retrieve(karlKey);
		cout << t << endl;

		del(karlKey);
		t = retrieve(karlKey);
		cout << "should show nothing" << endl;
		cout << t << endl; 

		disconnectTable();
		
		connectTable("TestLinked.bin");
		
		Person kimKey;
		kimKey.init("Kim", "Castleton");
		Person s = retrieve(karlKey);
		cout << s << endl;
		s = retrieve(kimKey);
		cout << s << endl;

		disconnectTable();
	} catch (DBException dbe) {
		cerr << "A database exception occurred" << endl;
	}

	return 0;
}



