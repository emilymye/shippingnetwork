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
        LocationReactor( Location* l) 
            : Location::Notifiee(l), location_(l) { }
        void onShipmentRecieved(Shipment * shipment);
        void onSegmentCapacity(Segment* s); 
    protected: 
        Location::Ptr location_;
        queue<Shipment*> holdQ_;
    };

    class CustomerReactor : public CustomerLocation::Notifiee {
    public:
        CustomerReactor( CustomerLocation* n, Fwk::Ptr<Activity::Manager> m, ShippingNetwork::Ptr sn) 
            : CustomerLocation::Notifiee(n) , customer_(n), manager_(m), network_(sn), routed_(false) {}
        void onDestination() { 
            if (customer_->destination() != NULL) {
                routed_ = false;
            }
            changeInjectActivity(); 
        }
        void onTransferRate() { changeInjectActivity(); }
        void onShipmentSize() { changeInjectActivity(); }
        void onShipmentRecieved(Shipment * shipment);
        RouteNode* route(Location * l) {

        }
    protected:
        void changeInjectActivity();
        CustomerLocation::Ptr customer_;
        Fwk::Ptr<Activity::Manager> manager_;
        ShippingNetwork::Ptr network_;

        //ROUTING
        void findRoutes();
        double segShipTime(Segment* s) ;
        bool routed_;

        vector<Route> routes_;
        queue<Shipment*> holdQ_;
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
            Fwk::Ptr<Activity::Manager> manager, Activity* activity, double rate, 
            Location::Ptr l ,  Shipment* shipment) 
            : Notifiee(activity), activity_(activity), manager_(manager), 
            forwardLoc_(l), shipment_(shipment), rate_(rate){}
        void onStatus();
    protected:
        double rate_;
        Activity::Ptr activity_;
        Fwk::Ptr<Activity::Manager> manager_;
        Location::Ptr forwardLoc_;
        Shipment* shipment_;
    };

}/* end namespace */
#endif
