#include <string>
#include <set>
#include <ostream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include "Instance.h"

using namespace std;

int main() {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    Ptr<Instance> inst;

    /* Set up destination(dest) and the terminal it connects to(t0) */
    m->instanceNew("dest", "Customer");
    m->instanceNew("t0", "Truck terminal");
    inst = m->instanceNew("seg_dest_t0", "Truck segment");
    inst->attributeIs("source", "dest");
    inst = m->instanceNew("seg_t0_dest", "Truck segment");
    inst->attributeIs("source", "t0");
    inst->attributeIs("return segment", "seg_dest_t0");

    /* Set up the 10 terminals that t0 connects to 
     * (t1_0, t1_1, ..., t1_9) 
     * as well as the 100 sources that they connect to
     * (s_0, s_1, ..., s_99) */
    stringstream convert_ss;
    for (int i = 0; i < 10; i++) {
        string name1 = "t1_";
        convert_ss.str("");
        convert_ss.clear();
        convert_ss << i;
        name1 = name1 + string(convert_ss.str());
        m->instanceNew(name1, "Truck terminal");
        inst = m->instanceNew("seg_t0_"+name1, "Truck segment");
        inst->attributeIs("source", "t0");
        inst = m->instanceNew("seg_"+name1+"_t0", "Truck segment");
        inst->attributeIs("source", name1);
        inst->attributeIs("return segment", "seg_t0_"+name1);
        for (int j = 0; j < 10; j++) {
            string name2 = "s_";
            convert_ss.str("");
            convert_ss.clear();
            convert_ss << i*10+j;
            name2 = name2 + string(convert_ss.str());
            m->instanceNew(name2, "Truck terminal");
            inst = m->instanceNew("seg_"+name1+"_"+name2, "Truck segment");
            inst->attributeIs("source", name1);
            inst = m->instanceNew("seg_"+name2+"_"+name1, "Truck segment");
            inst->attributeIs("source", name2);
            inst->attributeIs("return segment", "seg_"+name1+"_"+name2);
        }
    }
    return 0;
}
