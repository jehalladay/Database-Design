#include <iostream>
#include <cstring>

using namespace std;

const int FIRSTSIZE=30;
const int LASTSIZE=30;
const int ADDRESSSIZE=45;

const int MAXPEOPLE = 100;
int numPeople = 0;


class Person{
    private:
        int zip;
        float salary;
        char first[FIRSTSIZE];
        char last[LASTSIZE];
        char address[ADDRESSSIZE];

    public:
        Person(string newFirst = "", string newLast = "", string newAddress = "", int newZip = 0, float newSalary = 0.0) {
            strncpy(first, newFirst.c_str(), FIRSTSIZE);
            strncpy(last, newLast.c_str(), LASTSIZE);
            strncpy(address, newAddress.c_str(), ADDRESSSIZE);
            zip = newZip;
            salary = newSalary;
        }

        friend ostream & operator << (ostream &out, const Person &p){
            return out << "First: "   << p.first   << " Last:" << p.last << endl 
                       << "Address: " << p.address << " Zip:"  << p.zip 
                       << " Salary:"  << p.salary  << endl;
        }

        bool operator < (const Person &p) const {
            if (strncmp(p.last, last, LASTSIZE) == 0) {
                return strncmp(first, p.first, FIRSTSIZE) == -1;
            } else {
                return strncmp(last, p.last, FIRSTSIZE) == -1;
            } 
        }

        bool operator == (const Person &p) const {
            if (strncmp(p.last, last, LASTSIZE) == 0) {
                return strncmp(first,p.first,FIRSTSIZE)==0;
            } 

            return false;
        }
};

Person people[MAXPEOPLE];

class DBException {
    public:
        string message(){
            return "A database error occurred";
        }
};

void create(Person p) {
    if (numPeople == MAXPEOPLE) {
        throw DBException();
    } 

    people[numPeople] = p;
    numPeople++;
}

Person retrieve(Person p) {
    for (int i = 0; i < numPeople; i++) {
        cout << i << endl;
        if (people[i] == p) {
            return people[i];
        } 
    }

    cout << "Last" << endl;

    return Person();
}

void update(Person p) {
    for (int i = 0; i < numPeople; i++) {
        if (people[i] == p) { 
            people[i] = p;
            break;
        }
    }
}

void del(Person p) {
    for (int i=0;i<numPeople;i++) {
        if (people[i]==p) {
            numPeople--;
            for ( ; i < numPeople; i++) {
                people[i] = people[i + 1];
            }

            break;
        }
    }
}

int main() {
	
    create(Person("Karl", "Castleton", "1100 North Avenue", 81501, 50000.0));
	create(Person("Kim", "Castleton", "1200 North Avenue", 81502, 60000.0));

	Person t = retrieve(Person("Karl", "Castleton"));
	
    cout << t;
	
    update(Person("Karl", "Castleton", "1100 North Avenue", 81503, 65000.0));
	
    t = retrieve(Person("Karl", "Castleton"));
	
    cout << t;
	
    del(Person("Karl", "Castleton"));
	
    t = retrieve(Person("Karl", "Castleton"));
	
    cout << t;

	return 0;
}



