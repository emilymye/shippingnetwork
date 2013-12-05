#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include "Exception.h"
#include "Ptr.h"
#include "PtrInterface.h"
#include "Instance.h"
#include "Entity.h"
#include "Activity.h"
#include "Nominal.h"
#include "Notifiee.h"
#include <map>
#include <queue>
#include <cfloat>
#include <sstream>

namespace Shipping {
    using namespace std;
    //ordinal value types

    class ShippingNetwork : public Fwk::PtrInterface<ShippingNetwork> {
    public:
        typedef Fwk::Ptr<ShippingNetwork> Ptr;
        static ShippingNetwork::Ptr ShippingNetworkNew(string _name) {
            Ptr m = new ShippingNetwork();
            return m;
        }
        ~ShippingNetwork();

        //LOCATION ==============================================
        Location::Ptr locationNew( Location::Ptr l);
        void locationDel( string _name );
        Location* location( string _name) {
            return (locationMap.count(_name) > 0) ?
                locationMap[_name].ptr() : NULL;
        };

        //SEGMENT =============================================
        Segment::Ptr segmentNew( Segment::Ptr s );
        void segmentDel( string _name );
        Segment * segment( string _name) {
            return (segmentMap.count(_name) > 0) ?
                segmentMap[_name].ptr() : NULL;
        }

        //FLEET ==============================================
        Fleet::Ptr fleetNew(string _name);
        void fleetDel(string _name);
        Fleet * fleet() { return (fleet_) ? fleet_.ptr(): NULL; };

        //CONNECTIVITY ==============================================
        struct ExplorationQuery{
            Mile maxDist;
            Cost maxCost;
            Time maxTime;
            bool expedited;
            ExplorationQuery() : maxDist(FLT_MAX),maxCost(FLT_MAX), maxTime(FLT_MAX), expedited(false) {}
        };

        string path(string startLocation, string endLocation) {
            Location* start = location(startLocation);
            Location* end = location(endLocation);
            if (!start || !end) {
                cerr << "Invalid locations for connection" << endl;
                return "";
            }
            return connection(start,end);
        }
        string path(string startLocation, ExplorationQuery query){
            Location* start = location(startLocation);
            if (!start) {
                cerr << "Invalid start location for exploration" << endl;
                return "";
            }
            return exploration(start, query);
        }

        // NOTIFIEE =================================================
        class Notifiee : public Fwk::BaseNotifiee<ShippingNetwork> {
        public:
            typedef Fwk::Ptr<Notifiee> Ptr;
            Notifiee(ShippingNetwork* sn) : Fwk::BaseNotifiee<ShippingNetwork>(sn) {}
            virtual void onSegmentExpediteChange(bool) {}
            virtual void onLocationNew(Location::Ptr ) {}
            virtual void onSegmentNew(Segment::Ptr ) {}
            virtual void onLocationDel(Location::Ptr ) {}
            virtual void onSegmentDel( Segment::Ptr ) {}
        };
        virtual Notifiee::Ptr notifiee() const { return notifiee_; }
        virtual void lastNotifieeIs(Notifiee* n) {
            ShippingNetwork* me = const_cast<ShippingNetwork*>(this);
            me->notifiee_ = n;
        }
        //=========== END NOTIFIEE ================
    protected:
        ShippingNetwork() : fleet_(NULL), notifiee_(NULL) { } 
    private:
        ShippingNetwork::Notifiee * notifiee_;
        map<string,Location::Ptr> locationMap;
        map<string,Segment::Ptr> segmentMap;

        Activity::Manager::Ptr manager;
        Fleet::Ptr fleet_;

        //Private methods
        string connection(Location* start, Location* end);
        string exploration(Location* start, ExplorationQuery & query);
        struct Path {
            Location* end;
            Mile length;
            Cost cost;
            Time time;
            bool expedited;
            string pathStr;
            Path(Location* loc, string str, Mile l, Cost c, Time t, bool exp) :
                end(loc), length(l), cost(c), time(t), expedited(exp), pathStr(str) {}
        };
        void addPathWithSegment(Path &currPath, Segment::Ptr seg, queue<Path>& paths, bool strictExpedite);
        float travelCost(Segment::Ptr, bool);
        float travelTime(Segment::Ptr, bool);
    };

    class Percent : public Ordinal<Percent,float>{
    public:
        Percent(float num):Ordinal<Percent,float>(num){
            if (num < 0) {
                cerr << "Invalid Percent value: " << num << endl;
                throw Fwk::RangeException("Percent");
            }
        }
    };

    class ShippingNetworkReactor : public ShippingNetwork::Notifiee {
    public:
        ShippingNetworkReactor(ShippingNetwork * sn) 
            : ShippingNetwork::Notifiee(sn), expeditedSegments(0) {}
        void onLocationNew(Location::Ptr loc);
        void onSegmentNew(Segment::Ptr seg);
        void onLocationDel(Location::Ptr loc);
        void onSegmentDel(Segment::Ptr seg);
        void onSegmentExpediteChange(bool support);

        enum StatsEntityType{
            customer_ = 0,
            port_,
            truckTerminal_,
            boatTerminal_,
            planeTerminal_,
            truckSegment_,
            boatSegment_,
            planeSegment_,
            SHIPPING_ENTITY_COUNT
        };
        static inline StatsEntityType customer() { return customer_; };
        static inline StatsEntityType port() { return port_; };
        static inline StatsEntityType truckTerminal() { return truckTerminal_; };
        static inline StatsEntityType boatTerminal() { return boatTerminal_; };
        static inline StatsEntityType planeTerminal() { return planeTerminal_; };
        static inline StatsEntityType truckSegment() { return truckSegment_; };
        static inline StatsEntityType boatSegment() { return boatSegment_; };
        static inline StatsEntityType planeSegment() { return planeSegment_; };

        Percent expeditedPercent();
        unsigned int shippingEntities( StatsEntityType type );
    private:
        int expeditedSegments;
        unsigned int entityCounts [SHIPPING_ENTITY_COUNT];
    };
}

#endif
