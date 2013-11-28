#include <string>
#include <ostream>
#include <iostream>
#include "Instance.h"

using std::cout;
using std::cerr;
using std::endl;

void EQUAL(std::string a, std::string b);

int main(int argc, char *argv[]) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    Ptr<Instance> fleet = m->instanceNew("fleet", "Fleet");

    /* Make instances of varying types, then test stats */
    m->instanceNew("seg1", "Boat segment");
    m->instanceNew("seg2", "Plane segment");
    m->instanceNew("seg3", "Plane segment");
    m->instanceNew("seg4", "Truck segment");
    m->instanceNew("seg5", "Truck segment");
    m->instanceNew("seg6", "Truck segment");
    m->instanceNew("seg7", "Truck segment");
    m->instanceNew("seg8", "Truck segment");

    m->instanceNew("loc1", "Customer");
    m->instanceNew("loc2", "Port");
    m->instanceNew("loc3", "Truck terminal");
    m->instanceNew("loc4", "Boat terminal");
    m->instanceNew("loc5", "Plane terminal");
    m->instanceNew("loc6", "Truck terminal");
    m->instanceNew("loc7", "Truck terminal");
    m->instanceNew("loc8", "Truck terminal");

    /* Stats should still work even if created after some
       segments and locations have been made */
    Ptr<Instance> stats = m->instanceNew("stats", "Stats");

    /* Delete an object, which should reduce the count */
    m->instanceDel("loc8");
    EQUAL(stats->attribute("Truck terminal"), "3");

    Ptr<Instance> seg1 = m->instance("seg1");
    seg1->attributeIs("expedite support", "yes");
    EQUAL(stats->attribute("expedite percentage"), "12.50");

    /* Delete some more objects */
    m->instanceDel("seg5");
    m->instanceDel("seg6");
    m->instanceDel("seg7");
    m->instanceDel("seg8");
    EQUAL(stats->attribute("expedite percentage"), "25.00");

    /* Make sure attributes are read-only */
    stats->attributeIs("Boat segment", "99");
    EQUAL(stats->attribute("Boat segment"), "1");
    stats->attributeIs("Truck segment", "293823");
    EQUAL(stats->attribute("Truck segment"), "1");
    return 0;
}

void EQUAL(std::string a, std::string b){
    if (a.compare(b) != 0) 
    	cout << "ERROR, got " << a << " not " << b << endl;
    else 
    	cout << "Answer = " << a << " Key =  " << b << endl;
}
