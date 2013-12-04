#ifndef __SHIPPING_REACTOR_H
#define __SHIPPING_REACTOR_H

#include <queue>
#include "Notifiee.h"
#include "Ptr.h"
#include "PtrInterface.h"
#include "Entity.h"
#include "Engine.h"
#include "Activity.h"

namespace Shipping{
    class CustomerReactor : public CustomerLocation::Notifiee {
    public:
        CustomerReactor( CustomerLocation* c) 
            : CustomerLocation::Notifiee(c) { }
        void onDestination(Location::Ptr) {}
        void onTransferRate(Capacity) {}
        void onSize(Capacity) {}
    };

    class InjectActivityReactor : Activity::Notifiee {
    };

    class LocationReactor : public Location::Notifiee{
    public:
        LocationReactor( Location* l) 
            : Location::Notifiee(l) { }
        void onShipment(Shipment*) {}
    private:
    };

    class SegmentReactor : public Segment::Notifiee {
    public:
        SegmentReactor(Segment* n) : Segment::Notifiee(n) {}
        void onShipment(Shipment*) {}
    private:
    };



} /* end namespace */
#endif
