#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

const int FIRSTSIZE=30;
const int LASTSIZE=30;
const int ADDRESSSIZE=45;

enum RecordType {PERSON,FREELISTNODE};

class Person{
  public:
  char start;
  RecordType type;
  private:
  int zip;
  float salary;
  char first[FIRSTSIZE],last[LASTSIZE],address[ADDRESSSIZE];
  char end;
  public:
//  Person() { init(); }
  void init(string newFirst="",string newLast="",string newAddress="",int newZip=0,
    float newSalary=0.0) {
	  start='[';
	  end=']';
      strncpy(first,newFirst.c_str(),FIRSTSIZE);
      strncpy(last,newLast.c_str(),LASTSIZE);
      strncpy(address,newAddress.c_str(),ADDRESSSIZE);
      zip=newZip;
      salary=newSalary;
      type=PERSON;
    }
  friend ostream & operator <<(ostream &out,const Person &p){
      return out << "First: " << p.first<< " Last:"<<p.last<<endl<<"Address: "<<p.address<<
      " Zip:"<<p.zip<< " Salary:"<<p.salary <<endl;
  }
  bool operator <(const Person &p) const {
    if (strncmp(p.last,last,LASTSIZE)==0) return strncmp(first,p.first,FIRSTSIZE)==-1;
    else return strncmp(last,p.last,FIRSTSIZE)==-1;
  }
  bool operator ==(const Person &p) const {
    if (strncmp(p.last,last,LASTSIZE)==0) return strncmp(first,p.first,FIRSTSIZE)==0;
    return false;
  }
};

const long NULLRECORD=-1;
long nextFreeNode=NULLRECORD;

class FreeListNode {
	public:
	char start;
	RecordType type;
	long next;
	char message[32];
	char end;
	void init(long newNext) {
		start='[';
		end=']';
		next=newNext;
		type=FREELISTNODE;
		strcpy(message,"Free List Node");
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

void readAt(int i,PersonRecord &p) {
	dbfile.seekg(i*sizeof(PersonRecord));
	dbfile.read((char *)(&p),sizeof(PersonRecord));
}

void writeAt(int i,PersonRecord other) {
	dbfile.seekg(i*sizeof(PersonRecord));
	dbfile.write((char *)(&other),sizeof(PersonRecord));
}

void connectTable(string fname) {
    struct stat s;
	PersonRecord pr;
    if (stat(fname.c_str(),&s)!=0){
		ofstream temp;
		temp.open(fname);
		temp.close();
		dbfile.open(fname);
		pr.n.init(NULLRECORD);
		writeAt(0,pr); // First record is where we store the next (head of the linked list)
    }  else {
	  dbfile.open(fname);
    }
	if (!dbfile.is_open()) throw DBException();
	readAt(0,pr);        // Read first record to get the head of the free list
	nextFreeNode=pr.n.next;
}

void disconnectTable() {
	PersonRecord pr;
	pr.n.init(nextFreeNode);
	writeAt(0,pr); // First record is where we store the next (head of the linked list)	
	dbfile.close();
}

int getNumPeople() {
  dbfile.seekg(0,ios::end);
  int filesize=dbfile.tellg();
  return filesize/sizeof(PersonRecord);
}

/*
nextFreeNode=2 after connection to database 
 
0 [FreeListNode]  2
1 [Person      ]
2 [FreeListNode]  4
3 [Person      ]
4 [FreeListNode]
*/
void create(Person p) {  // O(1)
  int i;
  PersonRecord pr;
  if (nextFreeNode==NULLRECORD) 
    i=getNumPeople(); 
  else {
    readAt(nextFreeNode,pr);
    i=nextFreeNode;
    nextFreeNode=pr.n.next;
  }
  pr.p=p;
  writeAt(i,pr);
}

int find(Person p) {  // O(n)
  int numPeople=getNumPeople();
  for (int i=0;i<numPeople;i++) {
	 PersonRecord otherRecord;
	 readAt(i,otherRecord);
	 if (otherRecord.p.type==PERSON) {  // Skip freeNodeList records
	   Person other=otherRecord.p;
       if (other==p) return i;
     }
  }
  return NULLRECORD;
}

Person retrieve(Person p) {  // O(n)
  int numPeople=getNumPeople();
  for (int i=0;i<numPeople;i++) {
	 PersonRecord otherRecord;
	 readAt(i,otherRecord);
	 if (otherRecord.p.type==PERSON) {  // Skip freeNodeList records
	   Person other=otherRecord.p;
       if (other==p) return other;
     }
  }
  return Person();
}

void update(Person p) {  // O(n)
  int numPeople=getNumPeople();
  for (int i=0;i<numPeople;i++) {
	 PersonRecord otherRecord;
	 readAt(i,otherRecord);
	 if (otherRecord.p.type==PERSON) { // Skip freeNodeList records
	   Person other=otherRecord.p;
       if (other==p) {
	     PersonRecord pr;
	     pr.p=p; 
         writeAt(i,pr);
         break;
       }
     }
  }
}

void del(Person p) {  // O(n)
  int numPeople=getNumPeople();
  for (int i=0;i<numPeople;i++) {
	 PersonRecord otherRecord;
	 readAt(i,otherRecord);
	 if (otherRecord.p.type==PERSON) {
	   Person other=otherRecord.p;
       if (other==p) {
		 PersonRecord pr;
		 pr.n.init(nextFreeNode);
		 nextFreeNode=i;
		 writeAt(i,pr);
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
	karl.init("Karl","Castleton","1100 North Avenue",81501,50000.0);
	create(karl);

	Person kim;
	kim.init("Kim","Castleton","1200 North Avenue",81502,60000.0);
	create(kim);

	Person karlKey;
	karlKey.init("Karl","Castleton");
	Person t=retrieve(karlKey);
	cout << t;

	Person karlUpdate;
	karlUpdate.init("Karl","Castleton","1100 North Avenue",81503,65000.0);
	update(karlUpdate);

	t=retrieve(karlKey);
	cout << t;

	del(karlKey);
	t=retrieve(karlKey);
	cout << t; 

	disconnectTable();
  } catch (DBException dbe) {
	  cerr << "A database exception occurred" << endl;
  }
	return 0;
}



