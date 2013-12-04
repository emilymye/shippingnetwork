#include <string>
#include <sstream>
#include <set>
#include <ostream>
#include <iostream>
#include <cstdlib>
#include "Instance.h"
#include "Activity.h"

using namespace std;

int main(int argc, char *argv[]) {
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

    return 0;
}

