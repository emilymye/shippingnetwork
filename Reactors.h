#ifndef REACTORS_H
#define REACTORS_H

#include <queue>
#include "Notifiee.h"
#include "Ptr.h"
#include "PtrInterface.h"
#include "Entity.h"
#include "Engine.h"
#include "ActivityImpl.h"

namespace Shipping{
    class SegmentReactor : public Segment::Notifiee {
    public:
        SegmentReactor(Segment* n, Fleet* f, Fwk::Ptr<Activity::Manager> m) 
            : Segment::Notifiee(n), segment_(n), fleet_(f), manager_(m) {}
        void onShipmentNew(Shipment*);
        void onShipmentDel();
    protected: 
        Segment::Ptr segment_;
        Fwk::Ptr<Activity::Manager> manager_;
        Fleet* fleet_;
    };

    class LocationReactor : public Location::Notifiee{
    public:
        LocationReactor( Location* l, Fwk::Ptr<Activity::Manager> m) 
            : Location::Notifiee(l), location_(l),manager_(m) { }
        void onShipmentRecieved(Shipment * shipment);
        void onSegmentShipmentDel(Segment* seg); 
    protected: 
        Location::Ptr location_;
        Fwk::Ptr<Activity::Manager> manager_;

    };

    class CustomerReactor : public CustomerLocation::Notifiee {
    public:
        CustomerReactor( CustomerLocation* n, Fwk::Ptr<Activity::Manager> m, ShippingNetwork::Ptr sn) 
            : CustomerLocation::Notifiee(n) , customer_(n), manager_(m), network_(sn), 
            routed_(false), recieved_(0), totalTime_(0.0), totalCost_(0.f){}
        void onDestination() { 
            if (customer_->destination() != NULL) {
                routed_ = false;
            }
            changeInjectActivity(); 
        }
        void onTransferRate() { changeInjectActivity(); }
        void onShipmentSize() { changeInjectActivity(); }
        void onShipmentRecieved(Shipment * shipment);

        Capacity recieved() const { return recieved_; }
        Time latency() const { return (totalTime_.value()/recieved_.value()); }
        Cost totalCost() const { return totalCost_; }

    protected:
        void changeInjectActivity();
        CustomerLocation::Ptr customer_;
        Fwk::Ptr<Activity::Manager> manager_;
        ShippingNetwork::Ptr network_;

        Capacity recieved_;
        Time totalTime_;
        Cost totalCost_; 

        struct Route {
            map<Location*,Segment*> nodes;
            Segment* last;
            Time totalTime;
            Route () : totalTime(0.0), last(NULL) {}
        };

        class RouteCompare {
        public:
            bool operator() (Route & a, Route & b){
                return (a.totalTime > b.totalTime);
            } 
        };

        bool compareRoutes(Route a, Route b){
            return (a.totalTime < b.totalTime);
        }

        //ROUTING
        void findRoutes();
        double segShipTime(Segment* s) ;
        bool routed_;
    };

    class InjectActivityReactor : public Activity::Notifiee {
    public:
        InjectActivityReactor(Fwk::Ptr<Activity::Manager> manager, Activity* activity, CustomerLocation::Ptr source, double rate) 
            : Notifiee(activity), activity_(activity), manager_(manager), source_(source), rate_(rate){}
        void onStatus();
    protected:
        double rate_;
        Activity::Ptr activity_;
        Fwk::Ptr<Activity::Manager> manager_;
        CustomerLocation::Ptr source_;
    };

    class ForwardActivityReactor : public Activity::Notifiee {
    public:
        ForwardActivityReactor(
            Fwk::Ptr<Activity::Manager> manager, Activity* activity, Shipment* shipment) : Notifiee(activity), activity_(activity), manager_(manager), shipment_(shipment){}
        void onStatus();
    protected:
        Activity::Ptr activity_;
        Fwk::Ptr<Activity::Manager> manager_;
        Shipment* shipment_;
    };

}/* end namespace */
#endif
