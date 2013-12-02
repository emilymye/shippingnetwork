#include <string>
#include <set>
#include <ostream>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include "Instance.h"
#include <sstream>

using std::cout;
using std::cerr;
using std::endl;
using std::set;

/* Tokenizes the multi-line input into a set of strings */
set<string> tokenize(const std::string& output) {
	set<string> tokens; 
	static const char *delim = "\n";
	size_t last = output.find_first_not_of(delim, 0);
	size_t pos = output.find_first_of(delim, last);

    /* Find all lines that are non-empty */
	while (string::npos != pos || string::npos != last) {
		string token = output.substr(last, pos - last);
		for (size_t i = 0; i < token.size(); i++) {
			if (!isspace(token[i])) {
                /* Only insert if non-empty */
				cout << output.substr(last, pos - last) << endl;
				tokens.insert(output.substr(last, pos - last));
				break;
			}
		}

		last = output.find_first_not_of(delim, pos);
		pos = output.find_first_of(delim, last);
	}
	return tokens;
}

bool contains(const set<string>& s, const string& str) {
	return s.find(str) != s.end();
}

void EQUAL(std::string a, std::string b){
	if (a.compare(b) != 0) 
		cout << "ERROR, got " << a << " not " << b << endl;
	else 
		cout << "Answer = " << a << " Key =  " << b << endl;
}

void ASSERT(bool a){
	if (a) 
		cout << "PASSED" << endl;
	else 
		cout << "FAILED" << endl;
}

void testExplore1() {

    /* Simple network, arranged in a line */
	Ptr<Instance::Manager> m = shippingInstanceManager();
	Ptr<Instance> fleet = m->instanceNew("fleet", "Fleet");

	m->instanceNew("1", "Truck terminal");
	m->instanceNew("2", "Port");
	m->instanceNew("3", "Port");
	m->instanceNew("4", "Boat terminal");

	Ptr<Instance> s12 = m->instanceNew("s12", "Truck segment");
	Ptr<Instance> s21 = m->instanceNew("s21", "Truck segment");
	Ptr<Instance> s23 = m->instanceNew("s23", "Plane segment");
	Ptr<Instance> s32 = m->instanceNew("s32", "Plane segment");
	Ptr<Instance> s34 = m->instanceNew("s34", "Boat segment");
	Ptr<Instance> s43 = m->instanceNew("s43", "Boat segment");

	s12->attributeIs("length", "10");
	s21->attributeIs("length", "10");
	s23->attributeIs("length", "20");
	s32->attributeIs("length", "20");
	s34->attributeIs("length", "30"); 
	s43->attributeIs("length", "30");

	s12->attributeIs("source", "1");
	s12->attributeIs("return segment", "s21");
	s21->attributeIs("source", "2");

	s23->attributeIs("source", "2");
	s23->attributeIs("return segment", "s32");
	s32->attributeIs("source", "3");

	s34->attributeIs("source", "3");
	s34->attributeIs("return segment", "s43");
	s43->attributeIs("source", "4");

	Ptr<Instance> conn = m->instanceNew("conn", "Conn");
	set<string> t = tokenize(conn->attribute("explore 1 : distance 100000000"));
	ASSERT(contains(t, "1(s12:10.00:s21) 2"));
	ASSERT(contains(t, "1(s12:10.00:s21) 2(s23:20.00:s32) 3"));
	ASSERT(contains(t, "1(s12:10.00:s21) 2(s23:20.00:s32) 3(s34:30.00:s43) 4"));
}

void testExplore2() {
	Ptr<Instance::Manager> m = shippingInstanceManager();
	Ptr<Instance> fleet = m->instanceNew("fleet", "Fleet");

    /* Make a ring of segments that meets back around at the first port */
	Ptr<Instance> loc1 = m->instanceNew("1", "Port"); 
	Ptr<Instance> loc2 = m->instanceNew("2", "Port"); 
	Ptr<Instance> loc3 = m->instanceNew("3", "Port"); 
	Ptr<Instance> loc4 = m->instanceNew("4", "Port"); 

	Ptr<Instance> seg1 = m->instanceNew("s12", "Truck segment");
	Ptr<Instance> seg2 = m->instanceNew("s21", "Truck segment");
	Ptr<Instance> seg3 = m->instanceNew("s23", "Truck segment");
	Ptr<Instance> seg4 = m->instanceNew("s32", "Truck segment");
	Ptr<Instance> seg5 = m->instanceNew("s34", "Truck segment");
	Ptr<Instance> seg6 = m->instanceNew("s43", "Truck segment");
	Ptr<Instance> seg7 = m->instanceNew("s41", "Truck segment");
	Ptr<Instance> seg8 = m->instanceNew("s14", "Truck segment");

	seg1->attributeIs("length", "10");
	seg2->attributeIs("length", "10");
	seg3->attributeIs("length", "10");
	seg4->attributeIs("length", "10");
	seg5->attributeIs("length", "10");
	seg6->attributeIs("length", "10");
	seg7->attributeIs("length", "10");
	seg8->attributeIs("length", "10");

	seg1->attributeIs("source", "1");
	seg2->attributeIs("source", "2");
		
	seg1->attributeIs("return segment", "s21");
	seg3->attributeIs("return segment", "s32");
	seg5->attributeIs("return segment", "s43");
	seg7->attributeIs("return segment", "s14");

	seg3->attributeIs("source", "2");
	seg4->attributeIs("source", "3");

	seg5->attributeIs("source", "3");
	seg6->attributeIs("source", "4");

	seg7->attributeIs("source", "4");
	seg8->attributeIs("source", "1");


    /* Now query to see what happens when there is a cycle in 
       the graph */
	Ptr<Instance> conn = m->instanceNew("conn", "Conn");
	set<string> t = tokenize(conn->attribute("explore 1 : distance 100000000"));

	ASSERT(contains(t, "1(s12:10.00:s21) 2(s23:10.00:s32) 3(s34:10.00:s43) 4"));
	ASSERT(contains(t, "1(s14:10.00:s41) 4(s43:10.00:s34) 3(s32:10.00:s23) 2"));
	ASSERT(contains(t, "1(s12:10.00:s21) 2(s23:10.00:s32) 3"));
	ASSERT(contains(t, "1(s14:10.00:s41) 4(s43:10.00:s34) 3"));
	ASSERT(contains(t, "1(s12:10.00:s21) 2"));
	ASSERT(contains(t, "1(s14:10.00:s41) 4"));
}

void testExplore3() {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    Ptr<Instance> fleet = m->instanceNew("fleet", "Fleet");

    /* Create a tree-shaped network that branches out from a 
       single root node */
    m->instanceNew("a", "Port");
    m->instanceNew("b", "Port");
    m->instanceNew("c", "Port");
    m->instanceNew("d", "Port");
    m->instanceNew("e", "Port");
    m->instanceNew("f", "Port");
    m->instanceNew("g", "Port");

    Ptr<Instance> s;

    s = m->instanceNew("1", "Truck segment");
    s->attributeIs("source", "a");
    s->attributeIs("length", "1");
    s->attributeIs("difficulty", "1");
    s = m->instanceNew("1r", "Truck segment");
    s->attributeIs("source", "b");
    s->attributeIs("length", "1");
    s->attributeIs("difficulty", "1");
    s->attributeIs("return segment", "1");
    s->attributeIs("expedite support", "yes");

    s = m->instanceNew("2", "Truck segment");
    s->attributeIs("source", "a");
    s->attributeIs("length", "1");
    s->attributeIs("difficulty", "1");
    s->attributeIs("expedite support", "yes");
    s = m->instanceNew("2r", "Truck segment");
    s->attributeIs("source", "c");
    s->attributeIs("length", "1");
    s->attributeIs("difficulty", "1");
    s->attributeIs("return segment", "2");

    s = m->instanceNew("3", "Truck segment");
    s->attributeIs("source", "b");
    s->attributeIs("length", "1");
    s->attributeIs("difficulty", "1");
    s = m->instanceNew("3r", "Truck segment");
    s->attributeIs("source", "d");
    s->attributeIs("length", "1");
    s->attributeIs("difficulty", "1");
    s->attributeIs("return segment", "3");
    s->attributeIs("expedite support", "yes");

    s = m->instanceNew("4", "Truck segment");
    s->attributeIs("source", "b");
    s->attributeIs("length", "1");
    s->attributeIs("difficulty", "1");
    s = m->instanceNew("4r", "Truck segment");
    s->attributeIs("source", "e");
    s->attributeIs("length", "1");
    s->attributeIs("difficulty", "1");
    s->attributeIs("return segment", "4");

    s = m->instanceNew("5", "Truck segment");
    s->attributeIs("source", "c");
    s->attributeIs("length", "1");
    s->attributeIs("difficulty", "1");
    s = m->instanceNew("5r", "Truck segment");
    s->attributeIs("source", "f");
    s->attributeIs("length", "1");
    s->attributeIs("difficulty", "1");
    s->attributeIs("return segment", "5");

    s = m->instanceNew("6", "Truck segment");
    s->attributeIs("source", "c");
    s->attributeIs("length", "1");
    s->attributeIs("difficulty", "1");
    s->attributeIs("expedite support", "yes");
    s = m->instanceNew("6r", "Truck segment");
    s->attributeIs("source", "g");
    s->attributeIs("length", "1");
    s->attributeIs("difficulty", "1");
    s->attributeIs("return segment", "6");

    fleet->attributeIs("Truck, cost", "10");
    fleet->attributeIs("Truck, speed", "10");
    fleet->attributeIs("Truck, capacity", "10");

    /* The graph created above is a tree, with "a" at the root.
     * All links have length 1 and difficulty 1 */
    Ptr<Instance> conn = m->instanceNew("conn", "Conn");
    set<string> t = tokenize(conn->attribute("explore a : distance 1"));
    ASSERT(contains(t, "a(1:1.00:1r) b"));
    ASSERT(contains(t, "a(2:1.00:2r) c"));
    ASSERT(t.size() == 2);

    t = tokenize(conn->attribute("explore a : cost 20"));
    ASSERT(contains(t, "a(1:1.00:1r) b"));
    ASSERT(contains(t, "a(1:1.00:1r) b(3:1.00:3r) d"));
    ASSERT(contains(t, "a(1:1.00:1r) b(4:1.00:4r) e"));
    ASSERT(contains(t, "a(2:1.00:2r) c"));
    ASSERT(contains(t, "a(2:1.00:2r) c(5:1.00:5r) f"));
    ASSERT(contains(t, "a(2:1.00:2r) c(6:1.00:6r) g"));
    ASSERT(t.size() == 6);

    t = tokenize(conn->attribute("explore d : expedited"));
    ASSERT(contains(t, "d(3r:1.00:3) b(1r:1.00:1) a(2:1.00:2r) c(6:1.00:6r) g"));
}

void testExploreNoFleet() {
    /* Simple network, arranged in a line */
    Ptr<Instance::Manager> m = shippingInstanceManager();

    m->instanceNew("1", "Truck terminal");
    m->instanceNew("2", "Port");
    m->instanceNew("3", "Port");
    m->instanceNew("4", "Boat terminal");

    Ptr<Instance> s12 = m->instanceNew("s12", "Truck segment");
    Ptr<Instance> s21 = m->instanceNew("s21", "Truck segment");

    s12->attributeIs("length", "10");
    s21->attributeIs("length", "10");

    s12->attributeIs("source", "1");
    s12->attributeIs("return segment", "s21");
    s21->attributeIs("source", "2");

    Ptr<Instance> conn = m->instanceNew("conn", "Conn");
    set<string> t = tokenize(conn->attribute("explore 1 : distance 100000000"));
    ASSERT(contains(t, "1(s12:10.00:s21) 2"));
}

int main(int argc, char *argv[]) {
    testExplore3();
	//testExplore2();
    //testExplore3();
    //testExploreNoFleet();
}