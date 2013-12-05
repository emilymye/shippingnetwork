#include <math.h>
#include "Reactors.h"
#include "Ptr.h"
#include "PtrInterface.h"
#include "Activity.h"
/*
TODO:
LocationReactor::onShipmentRecieved
ForwardActivityReactor::onStatus
*/
namespace Shipping {
    void SegmentReactor::onShipmentRecieved(Shipment* shipment)
    {
        double traversals = ceil( ((double) (shipment->packages).value())/(fleet_->capacity(segment_->mode())).value());
        double travelTime = segment_->length().value() /fleet_->speed(segment_->mode()).value() * traversals;
        float travelCost = segment_->length().value() * segment_->difficulty().value() * traversals;
        if (fleet_ != NULL)
            travelCost *= fleet_->cost(segment_->mode()).value();
        shipment->totalCost = shipment->totalCost.value() + travelCost;
        shipment->totalTime = shipment->totalTime.value() + travelTime;

        Fwk::Ptr<Activity> forwardAct = manager_->activityNew(segment_->name());
        Location* nextLoc = segment_->returnSegment()->source();
        forwardAct->lastNotifieeIs( 
            new ForwardActivityReactor(manager_,forwardAct.ptr(),travelTime,nextLoc,shipment));
        forwardAct->statusIs(Activity::nextTimeScheduled);
    }

    void LocationReactor::onShipmentRecieved(Shipment* shipment)
    {
        cout << "on Shipment" << endl;
        
    }

    void CustomerReactor::onShipmentRecieved(Shipment* shipment)
    {
        if (!shipment->src->name().compare(customer_->name())){
            cout << "on Shipment Customer" << endl;
            
        }
    }

    void CustomerReactor::testCanInject() {
        if (manager_->activity(customer_->name()))
            manager_->activityDel(customer_->name());

        if ((customer_->destination() != NULL) &&
            (customer_->transferRate() > 0) &&
            (customer_->shipmentSize() > 0)) {
                Fwk::Ptr<Activity> injectAct = manager_->activityNew(customer_->name());
                double rate = 24.0/(customer_->transferRate()).value();
                injectAct->lastNotifieeIs(
                    new InjectActivityReactor(manager_,injectAct.ptr(),customer_,rate));
                injectAct->statusIs(Activity::nextTimeScheduled);
        }
    }

    //ACTIVITY REACTORS - on status 
    void InjectActivityReactor::onStatus() {
        ActivityImpl::ManagerImpl::Ptr managerImpl = Fwk::ptr_cast<ActivityImpl::ManagerImpl>(manager_);
        switch (activity_->status()) {

        case Activity::executing:
            cout << "InjectActivityReactor EXECUTE" << activity_->nextTime().value() << endl;
            source_->shipmentNew( 
                new Shipment(source_.ptr(),source_->destination(), source_->shipmentSize()) 
                );
            break;

        case Activity::free: //auto rescheduled
            activity_->nextTimeIs(Time(activity_->nextTime().value() + rate_));
            activity_->statusIs(Activity::nextTimeScheduled);
            break;

        case Activity::nextTimeScheduled: //add to be scheduled
            manager_->lastActivityIs(activity_);
            break;

        default:
            break;
        }
    }

    void ForwardActivityReactor::onStatus() {
        ActivityImpl::ManagerImpl::Ptr managerImpl = Fwk::ptr_cast<ActivityImpl::ManagerImpl>(manager_);
        switch (activity_->status()) {

        case Activity::executing:
            //forwardLoc_->shipmentNew(shipment_);
        case Activity::free:
            cout << "ForwardActivityReactor" << activity_->nextTime().value() << ", " << rate_ << endl;
            activity_->nextTimeIs(Time(activity_->nextTime().value() + rate_));
            activity_->statusIs(Activity::nextTimeScheduled);
            break;

        case Activity::nextTimeScheduled:
            //add myself to be scheduled
            manager_->lastActivityIs(activity_);
            break;

        default:
            break;
        }

    }
}