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
            : Segment::Notifiee(n), segment_(n), fleet_(f) {}
        void onShipmentRecieved(Shipment*);
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
    protected: 
        Location::Ptr location_;
    };

    class CustomerReactor : public CustomerLocation::Notifiee {
    public:
        CustomerReactor( CustomerLocation* n, Fwk::Ptr<Activity::Manager> m) 
            : CustomerLocation::Notifiee(n) , customer_(n), manager_(m) {}
        void onDestination() { testCanInject(); }
        void onTransferRate() { testCanInject(); }
        void onShipmentSize() { testCanInject(); }
        void onShipmentRecieved(Shipment * shipment);
    protected:
        void testCanInject();
        CustomerLocation::Ptr customer_;
        Fwk::Ptr<Activity::Manager> manager_;
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
            forwardLoc_(l), shipment_(shipment) {}
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
