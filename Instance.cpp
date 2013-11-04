#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <cstdlib>
#include <cctype>
#include "Instance.h"
#include "Engine.h"

namespace Shipping {

using namespace std;

/*** Rep layer interfaces ***/

class ManagerImpl : public Instance::Manager {
public:
    ManagerImpl();

    // Manager method
    Ptr<Instance> instanceNew(const string& name, const string& type);

    // Manager method
    Ptr<Instance> instance(const string& name);

    // Manager method
    void instanceDel(const string& name);

//    ShippingNetwork* network_;

private:
    map<string,Ptr<Instance> > instance_;
};

ManagerImpl::ManagerImpl() {}

Ptr<Instance> ManagerImpl::instanceNew(const string& name, const string& type) {
    if (type == "Customer") {
        Ptr<CustomerRep> t = new CustomerRep(name, this);
        instance_[name] = t;
        return t;
    }
    else if (type == "Port") {
        Ptr<PortRep> t = new PortRep(name, this);
        instance_[name] = t;
        return t;
    }
    else if (type == "Truck terminal") {
        Ptr<TruckTerminalRep> t = new TruckTerminalRep(name, this);
        instance_[name] = t;
        return t;
    }
    else if (type == "Boat terminal") {
        Ptr<BoatTerminalRep> t = new BoatTerminalRep(name, this);
        instance_[name] = t;
        return t;
    }
    else if (type == "Plane terminal") {
        Ptr<PlaneTerminalRep> t = new PlaneTerminalRep(name, this);
        instance_[name] = t;
        return t;
    }
    else if (type == "Truck segment") {
        Ptr<TruckSegmentRep> t = new TruckSegmentRep(name, this);
        instance_[name] = t;
        return t;
    }
    else if (type == "Boat segment") {
        Ptr<BoatSegmentRep> t = new BoatSegmentRep(name, this);
        instance_[name] = t;
        return t;
    }
    else if (type == "Plane segment") {
        Ptr<PlaneSegmentRep> t = new PlaneSegmentRep(name, this);
        instance_[name] = t;
        return t;
    }
    else if (type == "Stats") {
        Ptr<StatsRep> t = new StatsRep(name, this);
        instance_[name] = t;
        return t;
    }
    else if (type == "Conn") {
        Ptr<ConnRep> t = new ConnRep(name, this);
        instance_[name] = t;
        return t;
    }
    else if (type == "Fleet") {
        Ptr<FleetRep> t = new FleetRep(name, this);
        instance_[name] = t;
        return t;
    }
    return NULL;
}

Ptr<Instance> ManagerImpl::instance(const string& name) {
    map<string,Ptr<Instance> >::const_iterator t = instance_.find(name);
    if (t == instance_.end()) {
        cerr << name << " does not exist as an instance\n";
        return NULL;
    }
    else {
        return (*t).second;
    }
}

void ManagerImpl::instanceDel(const string& name) {
    map<string,Ptr<Instance> >::const_iterator t = instance_.find(name);
    if (t == instance_.end()) {
        cerr << name << " does not exist as an instance\n";
    }
    else {
        instance_.erase(t);
    }
    return;
}


class LocationRep : public Instance {
public:
    LocationRep(const string& name, ManagerImpl* manager);

    // Instance method
    string attribute(const string& name);

    // Instance method
    void attributeIs(const string& name, const string& v);

protected:
    Location* loc_;

private:
    Ptr<ManagerImpl> manager_;
    int segmentNumber(const string& name);
};

LocationRep::LocationRep(const string& name, ManagerImpl* manager) :
    Instance(name), manager_(manager) {
    loc_ = new LocationNew(name);
}

static const string segmentStr = "segment";
static const int segmentStrlen = segmentStr.length();
string LocationRep::attribute(const string& name) {
    if (name.length() <= segmentStrlen) {
        cerr << "Invalid attribute\n"
        return "";
    }
    if (name.substr(0, segmentStrlen).compare(segmentStr) != 0) {
        cerr << "Invalid attribute\n"
        return "";
    }
    for (int i = segmentStrlen; i < name.length(); i++) {
        if (!isdigit(name[i])) {
            cerr << "Invalid attribute\n"
            return "";
        }
    }

    int segNum = atoi(name.substr(7).c_str());
    Ptr<Segment> segment = loc_.segment(segNum);
    return segment.name();
}

void LocationRep::attributeIs(const string& name, const string& v) {
    cerr << "Cannot set attribute of Location entity\n" <<
    "Set source attribute of segment instead\n";
    return;
}


class CustomerRep : public LocationRep {
public:
    CustomerRep(const string& name, ManagerImpl *manager) :
        LocationRep(name, manager) {
        loc_->typeIs(Location::customer);
    }

};

class PortRep : public LocationRep {
public:

    PortRep(const string& name, ManagerImpl *manager) :
        LocationRep(name, manager) {
        loc_->typeIs(Location::port);
    }

};

class TruckTerminalRep : public LocationRep {
public:

    TruckTerminalRep(const string& name, ManagerImpl *manager) :
        LocationRep(name, manager) {
        loc_->typeIs(Location::truckTerminal);
    }

};

class BoatTerminalRep : public LocationRep {
public:
    BoatTerminalRep(const string& name, ManagerImpl *manager) :
        LocationRep(name, manager) {
        loc_->typeIs(Location::boatTerminal);
    }
};

class PlaneTerminalRep : public LocationRep {
public:
    PlaneTerminalRep(const string& name, ManagerImpl *manager) :
        LocationRep(name, manager) {
        loc_->typeIs(Location::planeTerminal);
    }

};


class SegmentRep : public Instance {
public:
    SegmentRep(const string& name, ManagerImpl* manager) :
        Instance(name), manager_(manager) {}

    // Instance method
    string attribute(const string& name);

    // Instance method
    void attributeIs(const string& name, const string& v);
protected:
    Segment* seg_;
private:
    Ptr<ManagerImpl> manager_;
};

static const string sourceStr = "source";
static const string lengthStr = "length";
static const string returnSegStr = "return segment";
static const string difficultyStr = "difficulty";
static const string expSupportStr = "expedite support";
static const MAXDIGITS = 20;
string SegmentRep::attribute(const string& name) {
    if (!name.compare(sourceStr)) {
        return seg_->source().name();
    }
    else if (!name.compare(lengthStr)) {
        char *str = new char[MAXDIGITS];
        sprintf(str, "%.2f", seg_->length());
        return str;
    }
    else if (!name.compare(returnSegStr)) {
        return seg_->returnSegment()->name();
    }
    else if (!name.compare(difficultyStr)) {
        char *str = new char[MAXDIGITS];
        sprintf(str, "%.2f", seg_->difficulty());
        return str;
    }
    else if (!name.compare(expSupportStr)) {
        if (seg_->expediteSupport()) {
            return "yes";
        }
        else {
            return "no";
        }
    }
}

class TruckSegmentRep : public SegmentRep {
public:
    TruckSegmentRep(const string& name, ManagerImpl* manager) :
        SegmentRep(name, manager) {
        seg_ = new SegmentNew(name, truck);
    }
};

class BoatSegmentRep : public SegmentRep {
public:
    BoatSegmentRep(const string& name, ManagerImpl* manager) :
        SegmentRep(name, manager) {
        seg_ = new SegmentNew(name, boat);
    }
};

class PlaneSegmentRep : public SegmentRep {
public:
    PlaneSegmentRep(const string& name, ManagerImpl* manager) :
        SegmentRep(name, manager) {
        seg_ = new SegmentNew(name, plane);
    }
};


class StatsRep : public Instance {
public:
    StatsRep(const string& name, ManagerImpl* manager) :
        Instance(name), manager_(manager) {}

    // Instance method
    string attribute(const string& name);

    // Instance method
    void attributeIs(const string& name, const string& v);

private:
    Ptr<ManagerImpl> manager_;
};


class ConnRep : public Instance {
public:
    ConnRep(const string& name, ManagerImpl* manager) :
        Instance(name), manager_(manager) {}

    // Instance method
    string attribute(const string& name);

    // Instance method
    void attributeIs(const string& name, const string& v);

private:
    Ptr<ManagerImpl> manager_;
};

class FleetRep : public Instance {
public:
    FleetRep(const string& name, ManagerImpl* manager) :
        Instance(name), manager_(manager) {}

    // Instance method
    string attribute(const string& name);

    // Instance method
    void attributeIs(const string& name, const string& v);

private:
    Ptr<ManagerImpl> manager_;
};


}

/*
 * This is the entry point for your library.
 * The client program will call this function to get a handle
 * on the Instance::Manager object, and from there will use
 * that object to interact with the middle layer (which will
 * in turn interact with the engine layer).
 */
Ptr<Instance::Manager> shippingInstanceManager() {
    return new Shipping::ManagerImpl();
}
