#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <cctype>
#include "Nominal.h"
#include "Instance.h"
#include "ActivityImpl.h"
#include "Engine.h"
#include "Entity.h"
//#include "EntityReactor.h"

namespace Shipping {
    using namespace std;

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
        ShippingNetworkReactor* reactor_;
        Ptr<Activity::Manager> manager_;

        enum InstanceType {
            None,
            Location,
            Segment,
            Fleet,
            Stats,
            Connection,

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

    /* ====== | STATIC STRINGS | ====================*/

    // INSTANCE & STATS TYPES
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

    static const string segmentStr = "segment";                     //LOCATION
    static const unsigned int segmentStrlen = segmentStr.length();
    //Customer only:
    static const string transferRateStr = "transfer rate";
    static const string shipmentSizeStr = "shipment size";
    static const string destinationStr = "destination";
    static const string shipmentsRecievedStr = "shipments recieved";
    static const string latencyStr = "latency";
    static const string totalCostStr = "total cost";

    static const string sourceStr = "source";                       //SEGMENT
    static const string lengthStr = "length";
    static const string returnSegStr = "return segment";
    static const string difficultyStr = "difficulty";
    static const string expSupportStr = "expedite support";
    static const string shipmentsRefusedStr = "shipments refused";

    static const string speedStr = "speed";                         //FLEET        
    static const string capacityStr = "capacity"; 

    static const string expPercentStr = "expedite percentage";      //STATS (ASSN2)
    static const string exploreStr = "explore";                     //CONNECTION
    static const string connectStr = "connect";
    static const string distanceStr = "distance";
    static const string costStr = "cost";
    static const string timeStr = "time";
    static const string expeditedStr = "expedited";

    static const unsigned int connTypeStrLen = exploreStr.length();

    // =========== | LOCATION INSTANCES |==========================
    class LocationRep : public Instance {
    public:
        LocationRep(const string& name, ManagerImpl* manager, ShippingNetwork::Ptr network) :
            Instance(name), manager_(manager), network_(network){}
        virtual string attribute(const string& name);
        virtual void attributeIs(const string& name, const string& v);
    protected:
        Ptr<Location> loc_;
        Ptr<ManagerImpl> manager_;
        Ptr<ShippingNetwork> network_;
        int segmentNumber(const string& name) {
            if (name.substr(0, segmentStrlen) == segmentStr) {
                const char* t = name.c_str() + segmentStrlen;
                return atoi(t);
            }
            return 0;
        }
    };

    string LocationRep::attribute(const string& name) {
        if (name.length() > segmentStrlen) {
            int idx = segmentNumber(name);
            if (idx) {
                Segment::Ptr s = loc_->segment(idx);
                if (s) return s->name();
            }
        }

        cerr << "Location - Invalid attribute/segment " << name << endl;
        return "";
    }

    void LocationRep::attributeIs(const string& name, const string& v) {
        throw Fwk::AttributeNotSupportedException("Invalid attribute: Location does not allow attributeIs \n");
    }

    //LOCATION INSTANCES SUBCLASSES
    class CustomerRep : public LocationRep {
    public:
        CustomerRep(const string& name, ManagerImpl *manager, ShippingNetwork::Ptr network) :
            LocationRep(name, manager, network) {
                loc_ = CustomerLocation::CustomerLocationNew(name);
                network_->locationNew(loc_);
        }
        string attribute(const string& name);
        void attributeIs(const string& name, const string& v);
    private:
        CustomerLocation::Ptr loc_;
    };

    string CustomerRep::attribute(const string& name) {
        try {
            stringstream ss;
            if (name == destinationStr) {
                return (loc_->destination()) ? loc_->destination()->name() : "";
            } else if (name == transferRateStr) {
                ss << loc_->transferRate().value();
                return ss.str();
            } else if (name == shipmentSizeStr) {
                ss << loc_->shipmentSize().value();
                return ss.str();
            } else if (name == shipmentsRecievedStr) {
                ss << loc_->recieved().value();
                return ss.str();
            } else if (name == latencyStr) {
                ss.precision(2);
                ss << fixed << loc_->latency().value();
                return ss.str();
            } else if (name == totalCostStr) {
                ss.precision(2);
                ss << fixed << loc_->totalCost().value();
                return ss.str();
            }

            throw Fwk::AttributeNotSupportedException("Invalid Customer Location attribute :" + name);
        } catch (Fwk::Exception &e) {
            cerr << e.what() << endl;
        } catch (...) { }
        return "";
    }

    void CustomerRep::attributeIs(const string& name, const string& v) { 
        if (name == transferRateStr) {
            loc_->transferRateIs(atoi(v.c_str()));
        } else if (name == shipmentSizeStr) {
            loc_->shipmentSizeIs(atoi(v.c_str()));
        } else if (name == destinationStr) {
            if (v.empty()) loc_->destinationIs(NULL);
            else {
                Location::Ptr dest = network_->location(v);
                if (dest == NULL) 
                    throw Fwk::EntityNotFoundException("Customer destination does not exist: " + v);
                else 
                    loc_->destinationIs(dest.ptr());
            }
        } else {
            throw Fwk::AttributeNotSupportedException("Invalid Customer Location attribute :" + name);
        }
    }

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
        SegmentRep(const string& name, ManagerImpl* manager, ShippingNetwork::Ptr sn, ShippingNetworkReactor::Ptr snr) 
            : Instance(name), manager_(manager), sn_(sn), snreactor_(snr){}
        string attribute(const string& name);
        void attributeIs(const string& name, const string& v);
    protected:
        Segment::Ptr seg_;
        Ptr<ManagerImpl> manager_;
        ShippingNetwork::Ptr sn_;
        ShippingNetworkReactor::Ptr snreactor_;
    };

    string SegmentRep::attribute(const string& name) {
        stringstream ss;
        if (name == sourceStr){
            return (seg_->source()) ?
                seg_->source()->name() : "";
        } else if (name == lengthStr) {
            stringstream ss;
            ss.precision(2);
            ss << fixed << seg_->length().value();
            return ss.str();
        } else if (name == returnSegStr) {
            return (seg_->returnSegment()) ?
                seg_->returnSegment()->name() : "";
        } else if (name == difficultyStr) {
            stringstream ss;
            ss.precision(2);
            ss << fixed << seg_->difficulty().value();
            return ss.str();
        } else if (name == expSupportStr) {
            return (seg_->expediteSupport()) ? "yes" : "no";
        } else if (name == shipmentsRecievedStr) {
            ss << seg_->receivedShipments().value();
            return ss.str();
        } else if (name == shipmentsRefusedStr) {
            ss << seg_->refusedShipments().value();
            return ss.str();
        } else if (name == capacityStr) {
            ss << seg_->shipmentCapacity().value();
            return ss.str();
        }

        cerr << "invalid Segment attribute: " << name << endl;
        return "";
    }

    void SegmentRep::attributeIs(const string& name, const string& v){
        try {
            if (!name.compare(sourceStr)) { //setting source
                Location* l = sn_->location(v);
                if (!l && !v.empty()){
                    throw Fwk::EntityNotFoundException("invalid source for Segment location");
                }
                if (v.empty()) {
                    if (seg_->source())
                        seg_->source()->segmentDel(seg_);
                    seg_->sourceIs(0);
                } else if ( l->type() <= Location::port() || l->type() == Location::terminalIdx() + seg_->mode()) {
                    if (seg_->source())
                        seg_->source()->segmentDel(seg_);
                    seg_->sourceIs(l);
                    l->segmentNew(seg_);
                } else {
                    throw Fwk::IllegalNameException("cannot attach segment of this mode to location"); 
                    return;
                }
            } else if (!name.compare(returnSegStr)) { //set return segment
                Segment::Ptr rSeg = sn_->segment(v);
                if (!rSeg && !v.empty()) {
                    throw Fwk::EntityNotFoundException("Segment returnSegment: Invalid segment name");
                }

                if (seg_->returnSegment()) {
                    seg_->returnSegment()->returnSegmentIs(0);
                }

                if (rSeg) {
                    seg_->returnSegmentIs(rSeg.ptr());
                    rSeg->returnSegmentIs(seg_.ptr());
                } else {
                    seg_->returnSegmentIs(0);
                }
            } else if (!name.compare(lengthStr)) {
                seg_->lengthIs(atof(v.c_str()));
            } else if (!name.compare(difficultyStr)) {
                seg_->difficultyIs( atof(v.c_str()));
            } else if (!name.compare(expSupportStr)){
                if (!v.compare("yes")){
                    if (seg_->expediteSupport()) return;
                    seg_->expediteSupportIs(true);
                    if (sn_->notifiee() != NULL) 
                        sn_->notifiee()->onSegmentExpediteChange(true);
                } else if (!v.compare("no")){
                    if (!seg_->expediteSupport()) return;
                    seg_->expediteSupportIs(false);
                    if (sn_->notifiee()) 
                        sn_->notifiee()->onSegmentExpediteChange(false);
                }
            } else if (name == capacityStr) {
                seg_->shipmentCapacityIs(atoi(v.c_str()));
            } else {
                throw Fwk::AttributeNotSupportedException("Invalid segment attribute: Segment " + name);
            } 
        } catch (Fwk::RangeException & e) {
            throw Fwk::RangeException("Segment attribute value out of range: " + name + v);
        } // Passes all but RangeExceptions back to client without editing
    }

    class TruckSegmentRep : public SegmentRep {
    public:
        TruckSegmentRep(const string& name, ManagerImpl* manager, ShippingNetwork::Ptr sn, ShippingNetworkReactor::Ptr snr) : 
            SegmentRep(name, manager, sn, snr) {
                seg_ = TruckSegment::TruckSegmentNew(name);
                sn_->segmentNew(seg_);
        }
    };

    class BoatSegmentRep : public SegmentRep {
    public:
        BoatSegmentRep(const string& name, ManagerImpl* manager, ShippingNetwork::Ptr sn, ShippingNetworkReactor::Ptr snr) : 
            SegmentRep(name, manager, sn, snr) {
                seg_ = BoatSegment::BoatSegmentNew(name);
                sn_->segmentNew(seg_);
        }
    };

    class PlaneSegmentRep : public SegmentRep {
    public:
        PlaneSegmentRep(const string& name, ManagerImpl* manager, ShippingNetwork::Ptr sn, ShippingNetworkReactor::Ptr snr) : 
            SegmentRep(name, manager, sn, snr) {
                seg_ = PlaneSegment::PlaneSegmentNew(name);
                sn_->segmentNew(seg_);
        }
    };

    class StatsRep : public Instance {
    public:
        StatsRep(const string& name, ManagerImpl* manager, ShippingNetworkReactor* reactor) :
            Instance(name), manager_(manager), reactor_(reactor) {}
        string attribute(const string& name);
        void attributeIs(const string& name, const string& v);
    private:
        Ptr<ManagerImpl> manager_;
        ShippingNetworkReactor* reactor_;
    };

    string StatsRep::attribute(const string& name) {
        stringstream ss;
        if (!name.compare(expPercentStr)) {
            ss.precision(2);
            ss << fixed << reactor_->expeditedPercent().value();
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
        } else {
            cerr << "Invalid Statistics attribute: " << name << endl;
            return "";
        }
        return ss.str();
    }

    void StatsRep::attributeIs(const string& name, const string& v) {
        cerr << "cannot set attribute for Statistics" << endl;
        throw Fwk::AttributeNotSupportedException("Statistics " + name);
    }

    /* =========== | CONNECTION REP | =======================*/
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
            cerr << "Invalid Connection attribute: " << name << endl;
            return "";
        }

        string connType = name.substr(0,connTypeStrLen);
        stringstream ss;
        ss << name.substr(connTypeStrLen + 1);
        string startLoc;
        string token;
        ss >> startLoc >> token;

        try {
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
            } else {
                cerr << "Invalid Connection attribute" << endl;
            }
        } catch (...) {
            cerr << "Error finding connection: " << name << endl;
            return "";
        }

        return "";
    }

    void ConnRep::attributeIs(const string& name, const string& v) {
        throw Fwk::AttributeNotSupportedException("Connection attributes are read-only");
    }

    /* =========== | FLEET REP | =======================*/
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
            cerr << "invalid Fleet attribute: " << name << endl;
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
            cerr << "invalid mode - Fleet attribute" << endl;
            return "";
        }

        stringstream ss;
        string str = "";

        if (!property.compare(speedStr)){
            ss.precision(2);
            ss << fixed << fleet_->speed(m).value();
        } else if (!property.compare(costStr)){
            ss.precision(2);
            ss << fixed << fleet_->cost(m).value();
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
            throw Fwk::AttributeNotSupportedException("invalid Fleet attribute: " + name);
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
            throw Fwk::AttributeNotSupportedException("invalid Fleet mode: " + name);
        }

        try{
            if (!property.compare(speedStr)){
                fleet_->speedIs(m,atof(v.c_str()));
            } else if (!property.compare(costStr)){
                fleet_->costIs(m,atof(v.c_str()));
            } else if (!property.compare(capacityStr)){
                fleet_->capacityIs(m,atoi(v.c_str()));
            } else {
                throw Fwk::AttributeNotSupportedException("invalid Fleet property: " + name);
            }
        } catch (Fwk::RangeException & e) {
            throw Fwk::RangeException("Fleet attribute value out of range: " + name + v);
        }
    }

    /* =========== | INSTANCE MANAGER IMPLEMENTATION | ======================= */
    ManagerImpl::ManagerImpl() : statsCreated(false),connCreated(false),fleetCreated(false) {
        network_ = ShippingNetwork::ShippingNetworkNew("network");
        reactor_ = new ShippingNetworkReactor(network_.ptr());
        network_->lastNotifieeIs(reactor_);

    }

    Ptr<Instance> ManagerImpl::instance(const string& name) {
        map<string,InstanceStore>::const_iterator t = instance_.find(name);
        if (t == instance_.end()) {
            cerr << name << " does not exist as an instance\n";
            return NULL;
        }

        return (t->second).instance;
    }

    Ptr<Instance> ManagerImpl::instanceNew (const string& name, const string& type) {
        if (instance_.find(name) != instance_.end()){
            cerr << "Instance with name already exists" << endl;
            return NULL;
        }

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
            Ptr<TruckSegmentRep> t = new TruckSegmentRep(name, this, network_, reactor_);
            instance_[name] = InstanceStore(t,Segment);
            return t;
        }
        else if (type == boatSegmentStr) {
            Ptr<BoatSegmentRep> t = new BoatSegmentRep(name, this, network_,reactor_);
            instance_[name] = InstanceStore(t,Segment);
            return t;
        }
        else if (type == planeSegmentStr) {
            Ptr<PlaneSegmentRep> t = new PlaneSegmentRep(name, this, network_,reactor_);
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

    void ManagerImpl::instanceDel(const string& name) {
        map<string,InstanceStore>::iterator t = instance_.find(name);
        if (t == instance_.end()) {
            cerr << name << " does not exist as an instance\n";
            return;
        }

        switch( t->second.type ){
        case Location: network_->locationDel( name ); break;
        case Segment: network_->segmentDel( name ); break;
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