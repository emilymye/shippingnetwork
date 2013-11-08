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
    static const string truckStr = "Truck";
    static const string boatStr = "Boat";
    static const string planeStr = "Plane";
    static const string speedStr = "speed";
    static const string capacityStr = "capacity";

    static const string segmentStr = "segment";
    static const unsigned int segmentStrlen = segmentStr.length();

    static const string sourceStr = "source";
    static const string lengthStr = "length";
    static const string returnSegStr = "return segment";
    static const string difficultyStr = "difficulty";
    static const string expSupportStr = "expedite support";
    static const int MAXDIGITS = 20;

    static const string expPercentStr = "expedite percentage";

    static const string exploreStr = "explore";
    static const string connectStr = "connect";
    static const string distanceStr = "distance";
    static const string costStr = "cost";
    static const string timeStr = "time";
    static const string expeditedStr = "expedited";

    static const unsigned int connTypeStrLen = exploreStr.length();

    class ManagerImpl : public Instance::Manager {
    public:
        ManagerImpl();
        Ptr<Instance> instanceNew(const string& name, const string& type);
        Ptr<Instance> instance(const string& name);
        void instanceDel(const string& name);
    private:
        bool statsCreated;
        bool connCreated;
        bool fleetCreated;
        ShippingNetwork::Ptr network_;
        ShippingNetworkReactor::Ptr reactor_;

        enum InstanceType {
            None,
            Location,
            Segment,
            Fleet,
            Stats,
            Connection
        };        
        struct InstanceStore{
            Ptr<Instance> instance;
            InstanceType type;
            InstanceStore() : instance(0), type(None) {}
            InstanceStore(Ptr<Instance> p, InstanceType t): 
                instance(p), type(t) {}
        };
        map<string, InstanceStore> instance_;

    };

    /*** Rep layer interfaces ***/
    // LOCATION INSTANCE ==========================
    class LocationRep : public Instance {
    public:
        LocationRep(const string& name, ManagerImpl* manager, ShippingNetwork::Ptr network) : 
            Instance(name), manager_(manager), network_(network){}
        string attribute(const string& name);
        void attributeIs(const string& name, const string& v);
    protected:
        Ptr<Location> loc_;
        Ptr<ManagerImpl> manager_;
        Ptr<ShippingNetwork> network_;
        int segmentNumber(const string& name);
    };

    string LocationRep::attribute(const string& name) {
        if (name.length() > segmentStrlen) {
            int idx = segmentNumber(name);
            if (idx) {
                Segment::Ptr s = loc_->segment(idx);
                if (s) return s->name();
            }
        }

        cerr << "Invalid attribute for Location: " << name << endl;
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
        CustomerRep(const string& name, ManagerImpl *manager, ShippingNetwork::Ptr network) : 
            LocationRep(name, manager, network) {
                loc_ = CustomerLocation::CustomerLocationNew(name);
                network_->locationNew(loc_);
        }
    };

    class PortRep : public LocationRep {
    public:
        PortRep(const string& name, ManagerImpl *manager, ShippingNetwork::Ptr network) : 
            LocationRep(name, manager, network) {
                loc_ = PortLocation::PortLocationNew(name);
                network_->locationNew(loc_);
        }
    };

    class TruckTerminalRep : public LocationRep {
    public:
        TruckTerminalRep(const string& name, ManagerImpl *manager, ShippingNetwork::Ptr network) : 
            LocationRep(name, manager, network) {
                loc_ = TruckTerminal::TruckTerminalNew(name);
                network_->locationNew(loc_);
        }
    };

    class BoatTerminalRep : public LocationRep {
    public:
        BoatTerminalRep(const string& name, ManagerImpl *manager, ShippingNetwork::Ptr network) : 
            LocationRep(name, manager, network) {
                loc_ = BoatTerminal::BoatTerminalNew(name);
                network_->locationNew(loc_);
        }
    };

    class PlaneTerminalRep : public LocationRep {
    public:
        PlaneTerminalRep(const string& name, ManagerImpl *manager, ShippingNetwork::Ptr network) : 
            LocationRep(name, manager, network) {
                loc_ = PlaneTerminal::PlaneTerminalNew(name);
                network_->locationNew(loc_);
        }
    };

    // SEGMENT INSTANCE ==========================
    class SegmentRep : public Instance {
    public:
        SegmentRep(const string& name, ManagerImpl* manager, ShippingNetwork::Ptr sn) : Instance(name), manager_(manager), sn_(sn){}
        string attribute(const string& name);
        void attributeIs(const string& name, const string& v);
    protected:
        Segment::Ptr seg_;
        Ptr<ManagerImpl> manager_;
        ShippingNetwork::Ptr sn_;
    };

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
                Location::Ptr source_ = sn_->location(v);
                if (!source_) {
                    cerr << "invalid location for Segment source" << endl;
                    return;
                }
                if (source_->segmentNew(seg_)) 
                    seg_->sourceIs( sn_->location(v) );
                else 
                    cerr << "Terminal cannot be different mode than segment" << endl;
            } else if (!name.compare(lengthStr)) {
                seg_->lengthIs(atof(v.c_str()));
            } else if (!name.compare(returnSegStr)) {
                Segment::Ptr rSeg = sn_->segment(v);
                if (!rSeg) {
                    cerr << "invalid segment for Segment returnSegment" << endl;
                    return;
                }
                seg_->returnSegmentIs( rSeg );
                rSeg->returnSegmentIs( seg_ );
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
        TruckSegmentRep(const string& name, ManagerImpl* manager, ShippingNetwork::Ptr sn) :
            SegmentRep(name, manager, sn) {
                seg_ = TruckSegment::TruckSegmentNew(name);
                sn_->segmentNew(seg_);
        }
    };

    class BoatSegmentRep : public SegmentRep {
    public:
        BoatSegmentRep(const string& name, ManagerImpl* manager, ShippingNetwork::Ptr sn) :
            SegmentRep(name, manager, sn) {
                seg_ = BoatSegment::BoatSegmentNew(name);
                sn_->segmentNew(seg_);
        }
    };

    class PlaneSegmentRep : public SegmentRep {
    public:
        PlaneSegmentRep(const string& name, ManagerImpl* manager, ShippingNetwork::Ptr sn) :
            SegmentRep(name, manager, sn) {
                seg_ = PlaneSegment::PlaneSegmentNew(name);
                sn_->segmentNew(seg_);
        }
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

    string ConnRep::attribute(const string& name) {
        if (name.length() <= connTypeStrLen){
            return "";
        } 

        try {

            string connType = name.substr(0,connTypeStrLen);
            stringstream ss;
            ss << name.substr(connTypeStrLen + 1);
            string startLoc;
            string token;
            ss >> startLoc >> token; 

            if (!connType.compare(exploreStr)) {
                ShippingNetwork::ExplorationQuery q;
                while (!ss.eof()){
                    ss >> token;
                    if (!token.compare(distanceStr)){
                        float dist;
                        ss >> dist;
                        q.maxDist = dist;
                    } else if (!token.compare(costStr)){
                        float cost;
                        ss >> cost;
                        q.maxCost = cost;
                    } else if (!token.compare(timeStr)){
                        float time;
                        ss >> time;
                        q.maxTime = time;
                    } else if (!token.compare(expeditedStr)){
                        q.expedited = true;
                    }
                    else {
                        cerr << "Invalid explore query attribute" << token << endl;
                        return "";
                    }
                }
                return network_->path(startLoc,  q);
            } else if (!connType.compare(connectStr)) {
                string endLoc;
                ss >> endLoc;
                return network_->path(startLoc, endLoc);
            }
        } catch (Fwk::Exception e) {
            cerr << "Invalid attribute for Connection" << endl;
            return "";
        }

        return "";
    }

    void ConnRep::attributeIs(const string& name, const string& v) {
        cerr << "cannot set attribute for Connection" << endl;
    }

    class FleetRep : public Instance {
    public:
        FleetRep(const string& name, ManagerImpl* manager, Fleet::Ptr f) :
            Instance(name), manager_(manager), fleet_(f){ }      
        string attribute(const string& name);
        void attributeIs(const string& name, const string& v);
    private:
        Ptr<ManagerImpl> manager_;
        Fleet::Ptr fleet_;
    };



    string FleetRep::attribute(const string& name) {
        size_t idx = name.find(',');
        if (idx==std::string::npos) {
            cerr << "invalid attribute for Fleet" << endl;
            return "";
        }

        string mode = name.substr(0, idx);
        string property = name.substr(idx + 2);

        ShippingMode m;
        if (!mode.compare("Truck")){
            m = Fleet::truck();            
        } else if (!mode.compare("Boat")){
            m = Fleet::boat();
        } else if (!mode.compare("Plane")){
            m = Fleet::plane();
        } else {
            cerr << "invalid mode for Fleet attribute" << endl;
            return "";
        }

        stringstream ss;
        string str = "";
        if (!property.compare(speedStr)){
            ss.precision(2);
            ss << fleet_->speed(m).value();
        } else if (!property.compare(costStr)){
            ss.precision(2);
            ss << fleet_->cost(m).value();
        } else if (!property.compare(capacityStr)){
            ss << fleet_->capacity(m).value();
        } else {
            cerr << "invalid property for Fleet attribute" << endl;
        }

        ss >> str;
        return str;
    }

    void FleetRep::attributeIs(const string& name, const string& v) {
        size_t idx = name.find(',');
        if (idx==std::string::npos) {
            cerr << "invalid attribute for Fleet" << endl;  
            return;
        }

        string mode = name.substr(0, idx);
        string property = name.substr(idx + 2);

        ShippingMode m;
        if (!mode.compare("Truck")){
            m = Fleet::truck();            
        } else if (!mode.compare("Boat")){
            m = Fleet::boat();
        } else if (!mode.compare("Plane")){
            m = Fleet::plane();
        } else {
            cerr << "invalid mode for Fleet attribute" << endl;
            return;
        }

        try{
            if (!property.compare(speedStr)){
                fleet_->speedIs(m,atof(v.c_str()));
            } else if (!property.compare(costStr)){
                fleet_->costIs(m,atof(v.c_str()));
            } else if (!property.compare(capacityStr)){
                fleet_->capacityIs(m,atoi(v.c_str()));
            } 
        } catch (Fwk::Exception e) {
            cerr << "Error setting attribute " << name << " to " << v << endl;
        } 


    }

    ManagerImpl::ManagerImpl() {
        network_ = ShippingNetwork::ShippingNetworkNew("network");
        reactor_ = ShippingNetworkReactor::ShippingNetworkReactorIs(network_.ptr());
    }

    Ptr<Instance> ManagerImpl::instanceNew(const string& name, const string& type) {
        if (instance_.find(name) != instance_.end()) 
            return NULL;

        if (type == customerStr) {
            Ptr<CustomerRep> t = new CustomerRep(name, this, network_);
            instance_[name] = InstanceStore(t, Location);
            return t;
        }

        if (type == portStr) {
            Ptr<PortRep> t = new PortRep(name, this, network_);
            instance_[name] = InstanceStore(t,Location);
            return t;
        }

        else if (type == truckTerminalStr) {
            Ptr<TruckTerminalRep> t = new TruckTerminalRep(name, this, network_);
            instance_[name] = InstanceStore(t,Location);
            return t;
        }
        else if (type == boatTerminalStr) {
            Ptr<BoatTerminalRep> t = new BoatTerminalRep(name, this, network_);
            instance_[name] = InstanceStore(t,Location);
            return t;
        }
        else if (type == planeTerminalStr) {
            Ptr<PlaneTerminalRep> t = new PlaneTerminalRep(name, this, network_);
            instance_[name] = InstanceStore(t,Location);
            return t;
        }
        else if (type == truckSegmentStr) {
            Ptr<TruckSegmentRep> t = new TruckSegmentRep(name, this, network_);
            instance_[name] = InstanceStore(t,Segment);
            return t;
        }
        else if (type == boatSegmentStr) {
            Ptr<BoatSegmentRep> t = new BoatSegmentRep(name, this, network_);
            instance_[name] = InstanceStore(t,Segment);
            return t;
        }
        else if (type == planeSegmentStr) {
            Ptr<PlaneSegmentRep> t = new PlaneSegmentRep(name, this, network_);
            instance_[name] = InstanceStore(t,Segment);
            return t;
        }
        else if (type == statsStr) {
            if (statsCreated) return NULL;
            Ptr<StatsRep> t = new StatsRep(name, this, reactor_);
            statsCreated = true;
            instance_[name] = InstanceStore(t,Stats);
            return t;
        }
        else if (type == connStr) {
            if (connCreated) return NULL;
            Ptr<ConnRep> t = new ConnRep(name, this, network_);
            connCreated = true;
            instance_[name] = InstanceStore(t,Connection);
            return t;
        }
        else if (type == fleetStr) {
            if (fleetCreated) return NULL;
            Ptr<FleetRep> t = new FleetRep(name, this, network_->fleetNew(name));
            fleetCreated = true;
            instance_[name] = InstanceStore(t,Fleet);
            return t;
        }
        return NULL;
    }

    Ptr<Instance> ManagerImpl::instance(const string& name) {
        map<string,InstanceStore>::const_iterator t = instance_.find(name);
        if (t == instance_.end()) {
            cerr << name << " does not exist as an instance\n";
            return NULL;
        }

        return (t->second).instance;
    }

    void ManagerImpl::instanceDel(const string& name) {
        map<string,InstanceStore>::iterator t = instance_.find(name);
        if (t == instance_.end()) {
            cerr << name << " does not exist as an instance\n";
            return;
        }

        switch( t->second.type ){
        case Location: network_->locationDel( name ); break;
        case Segment: network_->locationDel( name ); break;
        case Fleet: network_->fleetDel( name ); fleetCreated = false; break;
        case Stats: statsCreated = false; break;
        case Connection: connCreated = false; break;
        default: break;
        }

        instance_.erase(t->first);
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
