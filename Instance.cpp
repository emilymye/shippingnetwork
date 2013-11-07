#include "fwk/NamedInterface.h"
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <cctype>
#include "Instance.h"
#include "Engine.h"
#include "Entity.h"
#include "Nominal.h"

namespace Shipping {

    using namespace std;

    static const string customerStr = "Customer";
    static const string portStr = "Port";
    static const string truckTerminalStr = "Truck terminal";
    static const string boatTerminalStr = "Boat terminal";
    static const string planeTerminalStr = "Plane terminal";
    static const string truckSegmentStr = "Truck segment";
    static const string boatSegmentStr = "Boat segment";
    static const string planeSegmentStr = "Plane segment";

    static const string statsStr = "Stats";
    static const string connStr = "Conn";
    static const string fleetStr = "Fleet";

    class ManagerImpl : public Instance::Manager {
    public:
        ManagerImpl();
        Ptr<Instance> instanceNew(const string& name, const string& type);
        Ptr<Instance> instance(const string& name);
        void instanceDel(const string& name);
    private:
        ShippingNetwork::Ptr network_;
        ShippingNetworkReactor::Ptr reactor_;
        map<string,Ptr<Instance> > instance_;
    };

    /*** Rep layer interfaces ***/
    // LOCATION INSTANCE ==========================
    class LocationRep : public Instance {
    public:
        LocationRep(const string& name, ManagerImpl* manager, Ptr<Location> loc) : Instance(name),manager_(manager), loc_(loc){}
        string attribute(const string& name);
        void attributeIs(const string& name, const string& v);
    private:
        Ptr<Location> loc_;
        Ptr<ManagerImpl> manager_;
        int segmentNumber(const string& name);
    };

    static const string segmentStr = "segment";
    static const unsigned int segmentStrlen = segmentStr.length();
    string LocationRep::attribute(const string& name) {
        if (name.length() > segmentStrlen) {
            int idx = segmentNumber(name);
            if (idx) {
                Segment::Ptr s = loc_->segment(idx);
                if (s) return s->name();
            }
        }

        cerr << "Invalid attribute: " << name << endl;
        return "";
    }

    void LocationRep::attributeIs(const string& name, const string& v) {
        cerr << "Cannot set attribute for Location\n" << endl;
    }

    int LocationRep::segmentNumber(const string& name) {
        if (name.substr(0, segmentStrlen) == segmentStr) {
            const char* t = name.c_str() + segmentStrlen;
            return atoi(t);
        }
        return 0;
    }

    //LOCATION INSTANCES SUBCLASSES
    class CustomerRep : public LocationRep {
    public:
        CustomerRep(const string& name, ManagerImpl *manager, Location::Ptr _loc) : 
            LocationRep(name, manager, _loc) { }
    };

    class PortRep : public LocationRep {
    public:
        PortRep(const string& name, ManagerImpl *manager, Location::Ptr _loc) : 
            LocationRep(name, manager, _loc) { }
    };

    class TruckTerminalRep : public LocationRep {
    public:
        TruckTerminalRep(const string& name, ManagerImpl *manager, Location::Ptr _loc) : 
            LocationRep(name, manager, _loc) { }
    };

    class BoatTerminalRep : public LocationRep {
    public:
        BoatTerminalRep(const string& name, ManagerImpl *manager, Location::Ptr _loc) : 
            LocationRep(name, manager, _loc) { }
    };

    class PlaneTerminalRep : public LocationRep {
    public:
        PlaneTerminalRep(const string& name, ManagerImpl *manager, Location::Ptr _loc) : 
            LocationRep(name, manager, _loc) { }
    };

    // SEGMENT INSTANCE ==========================
    class SegmentRep : public Instance {
    public:
        SegmentRep(const string& name, ManagerImpl* manager, Segment::Ptr segment, ShippingNetwork::Ptr sn) : Instance(name), manager_(manager), sn_(sn){}
        string attribute(const string& name);
        void attributeIs(const string& name, const string& v);
    private:
        Segment::Ptr seg_;
        Ptr<ManagerImpl> manager_;
        ShippingNetwork::Ptr sn_;
    };

    static const string sourceStr = "source";
    static const string lengthStr = "length";
    static const string returnSegStr = "return segment";
    static const string difficultyStr = "difficulty";
    static const string expSupportStr = "expedite support";
    static const int MAXDIGITS = 20;

    string SegmentRep::attribute(const string& name) {
        if (!name.compare(sourceStr)) {
            return seg_->source()->name();
        }
        else if (!name.compare(lengthStr)) {
            char *str = new char[MAXDIGITS];
            sprintf(str, "%.2f", seg_->length().value());
            return str;
        }
        else if (!name.compare(returnSegStr)) {
            return seg_->returnSegment()->name();
        }

        else if (!name.compare(difficultyStr)) {
            char *str = new char[MAXDIGITS];
            sprintf(str, "%.2f", seg_->difficulty().value());
            return str;
        }
        else if (!name.compare(expSupportStr)) {
            return (seg_->expediteSupport()) ? "yes" : "no";
        }
        return "";
    }
    void SegmentRep::attributeIs(const string& name, const string& v){
        try{
            if (!name.compare(sourceStr)) {
                seg_->sourceIs( sn_->location(v) );
            } else if (!name.compare(lengthStr)) {
                seg_->lengthIs(atof(v.c_str()));
            } else if (!name.compare(returnSegStr)) {
                seg_->returnSegmentIs( sn_->segment(v));
            } else if (!name.compare(difficultyStr)) {
                seg_->difficultyIs( atof(v.c_str()));
            } else if (!name.compare(expSupportStr)) {
                seg_->expediteSupportIs( !v.compare("yes") ? true : false);
            }
        } catch (Fwk::Exception e) {
            cerr << "invalid call to attributeIs() on segment" << endl;
        }
    }

    class TruckSegmentRep : public SegmentRep {
    public:
        TruckSegmentRep(const string& name, ManagerImpl* manager, Segment::Ptr seg, ShippingNetwork::Ptr sn) :
            SegmentRep(name, manager,seg,sn) {}
    };

    class BoatSegmentRep : public SegmentRep {
    public:
        BoatSegmentRep(const string& name, ManagerImpl* manager, Segment::Ptr seg, ShippingNetwork::Ptr sn) :
            SegmentRep(name, manager,seg,sn) {}
    };

    class PlaneSegmentRep : public SegmentRep {
    public:
        PlaneSegmentRep(const string& name, ManagerImpl* manager, Segment::Ptr seg, ShippingNetwork::Ptr sn) :
            SegmentRep(name, manager,seg,sn) {}
    };

    class StatsRep : public Instance {
    public:
        StatsRep(const string& name, ManagerImpl* manager, ShippingNetworkReactor::Ptr reactor) :
            Instance(name), manager_(manager), reactor_(reactor) {}
        string attribute(const string& name);
        void attributeIs(const string& name, const string& v);
    private:
        ShippingNetworkReactor::Ptr reactor_;
        Ptr<ManagerImpl> manager_;
    };

    static const string expPercentStr = "expedite percentage";

    string StatsRep::attribute(const string& name) {
        stringstream ss;
        if (!name.compare(expPercentStr)) {
            ss.precision(2);
            ss << reactor_->expeditedPercent().value();
        } else if (name == customerStr) {
            ss << reactor_->shippingEntities( ShippingNetworkReactor::customer() );
        } else if (name == portStr) {
            ss << reactor_->shippingEntities( ShippingNetworkReactor::port() );
        } else if (name == truckTerminalStr) {
            ss << reactor_->shippingEntities( ShippingNetworkReactor::truckTerminal() );
        } else if (name == boatTerminalStr) {
            ss << reactor_->shippingEntities( ShippingNetworkReactor::boatTerminal() );
        } else if (name == planeTerminalStr) {
            ss << reactor_->shippingEntities( ShippingNetworkReactor::planeTerminal() );
        } else if (name == truckSegmentStr) {
            ss << reactor_->shippingEntities( ShippingNetworkReactor::truckSegment() );
        } else if (name == boatSegmentStr) {
            ss << reactor_->shippingEntities( ShippingNetworkReactor::boatSegment() );
        } else if (name == planeSegmentStr) {
            ss << reactor_->shippingEntities( ShippingNetworkReactor::planeSegment() );
        }
        string str;
        ss >> str;
        return str;
    }

    void StatsRep::attributeIs(const string& name, const string& v) {
        cerr << "cannot set attribute for Statistics" << endl;
    }

    class ConnRep : public Instance {
    public:
        ConnRep(const string& name, ManagerImpl* manager, ShippingNetwork::Ptr sn) :
            Instance(name), manager_(manager), network_(sn) {}
        string attribute(const string& name);
        void attributeIs(const string& name, const string& v);
    private:
        Ptr<ManagerImpl> manager_;
        ShippingNetwork::Ptr network_;
    };

    /*    string ConnRep::attribute(const string& name) {
    }*/

    class FleetRep : public Instance {
    public:
        FleetRep(const string& name, ManagerImpl* manager, Fleet::Ptr f) :
            Instance(name), manager_(manager), fleet_(f){}
        string attribute(const string& name);
        void attributeIs(const string& name, const string& v);
    private:
        Ptr<ManagerImpl> manager_;
        Fleet::Ptr fleet_;
    };

    ManagerImpl::ManagerImpl() {
        network_ = ShippingNetwork::ShippingNetworkNew("network");
        reactor_ = ShippingNetworkReactor::ShippingNetworkReactorIs(network_.ptr());
    }

    Ptr<Instance> ManagerImpl::instanceNew(const string& name, const string& type) {
        if (instance_[name]) 
            return NULL;

        if (type == customerStr) {
            Location::Ptr entity = network_->locationNew(CustomerLocation::CustomerLocationNew(name));
            Ptr<CustomerRep> t = new CustomerRep(name, this, entity);
            instance_[name] = t;
            return t;
        }

        if (type == portStr) {
            Location::Ptr entity = network_->locationNew(PortLocation::PortLocationNew(name));
            Ptr<PortRep> t = new PortRep(name, this, entity);
            instance_[name] = t;
            return t;
        }

        else if (type == truckTerminalStr) {
            Location::Ptr entity = network_->locationNew(TruckTerminal::TruckTerminalNew(name));
            Ptr<TruckTerminalRep> t = new TruckTerminalRep(name, this, entity);
            instance_[name] = t;
            return t;
        }
        else if (type == boatTerminalStr) {
            Location::Ptr entity = network_->locationNew(BoatTerminal::BoatTerminalNew(name));
            Ptr<BoatTerminalRep> t = new BoatTerminalRep(name, this, entity);
            instance_[name] = t;
            return t;
        }
        else if (type == planeTerminalStr) {
            Location::Ptr entity = network_->locationNew(PlaneTerminal::PlaneTerminalNew(name));
            Ptr<PlaneTerminalRep> t = new PlaneTerminalRep(name, this, entity);
            instance_[name] = t;
            return t;
        }
        else if (type == truckSegmentStr) {
            Segment::Ptr entity = network_->segmentNew(TruckSegment::TruckSegmentNew(name));
            Ptr<TruckSegmentRep> t = new TruckSegmentRep(name, this, entity, network_);
            instance_[name] = t;
            return t;
        }
        else if (type == boatSegmentStr) {
            Segment::Ptr entity = network_->segmentNew(BoatSegment::BoatSegmentNew(name));
            Ptr<BoatSegmentRep> t = new BoatSegmentRep(name, this, entity, network_);
            instance_[name] = t;
            return t;
        }
        else if (type == planeSegmentStr) {
            Segment::Ptr entity = network_->segmentNew(PlaneSegment::PlaneSegmentNew(name));
            Ptr<PlaneSegmentRep> t = new PlaneSegmentRep(name, this, entity, network_);
            instance_[name] = t;
            return t;
        }
        else if (type == statsStr) {
            Ptr<StatsRep> t = new StatsRep(name, this, reactor_);
            instance_[name] = t;
            return t;
        }
        else if (type == connStr) {
            Ptr<ConnRep> t = new ConnRep(name, this, network_);
            instance_[name] = t;
            return t;
        }
        else if (type == fleetStr) {
            Ptr<FleetRep> t = new FleetRep(name, this, network_->fleet());
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
        map<string,Ptr<Instance> >::iterator t = instance_.find(name);
        if (t == instance_.end()) {
            cerr << name << " does not exist as an instance\n";
        }
        else {
            instance_.erase(t);
        }
        return;
    }

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
