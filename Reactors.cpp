#include <math.h>
#include <algorithm>
#include "Reactors.h"
#include "Ptr.h"
#include "PtrInterface.h"
#include "Activity.h"

namespace Shipping {
    void SegmentReactor::onShipmentDel()
    {

    }

    void SegmentReactor::onShipmentNew(Shipment* shipment)
    {
        double traversals = ceil( ((double) (shipment->packages).value())/(fleet_->capacity(segment_->mode())).value());
        double travelTime = segment_->length().value() /fleet_->speed(segment_->mode()).value() * traversals;
        float travelCost = segment_->length().value() * segment_->difficulty().value() * traversals;
        if (fleet_ != NULL)
            travelCost *= fleet_->cost(segment_->mode()).value();
        shipment->totalCost = shipment->totalCost.value() + travelCost;
        shipment->totalTime = shipment->totalTime.value() + travelTime;
        stringstream ss;
        ss << segment_->name() << segment_->receivedShipments().value() << endl;
        Fwk::Ptr<Activity> forwardAct = manager_->activityNew(ss.str());
        Location* nextLoc = segment_->returnSegment()->source();
        forwardAct->lastNotifieeIs( 
            new ForwardActivityReactor(manager_,forwardAct.ptr(),travelTime,nextLoc,shipment));
        forwardAct->statusIs(Activity::nextTimeScheduled);
    }

    void LocationReactor::onSegmentCapacity(Segment* s){


    }

    void LocationReactor::onShipmentRecieved(Shipment* shipment)
    {
        //shipment->currRouteNode
    }

    //CUSTOMER REACTOR
    void CustomerReactor::onShipmentRecieved(Shipment* s)
    {
        if (!s->dest->name().compare(customer_->name())) {
            delete s;
            // statistics updated in CustomerLocation method
        } else if (!s->src->name().compare(customer_->name())){
            if (!routed_){
                findRoutes();
                routed_ = true;
            }
            if (routes_.empty()) 
                cout << "Destination cannot be reached- dropping shipment..." << endl;

            for (vector<Route>::iterator it = routes_.begin(); it != routes_.end(); ++it){
                s->currRouteNode = &(it->nodes[0]);
                if(it->nodes[0].segment->shipmentNew(s)) return;
            }
            holdQ_.push(s);
            //IMPLEMENT HOLD QUEUE HERE
        } else throw Fwk::InternalException(
            "Error in routing - non-destination/source customers should not recieve packages" );
    }

    double CustomerReactor::segShipTime(Segment* s) {
        return s->length().value() / ((network_->fleet()) ? network_->fleet()->speed(s->mode()).value() : 50);
    }

    void CustomerReactor::changeInjectActivity() {
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

    bool compareRoutes(Route a, Route b){
        return (a.totalTime < b.totalTime);
    }
    void CustomerReactor::findRoutes(){
        if (routed_ || customer_->destination() == NULL || customer_->segments() == 0) return;
        string destName = customer_->destination()->name();
        routes_.empty();

        queue<Route> searchRoutes;
        searchRoutes.push(Route());

        Location* currL; 
        RouteNode temp = RouteNode(NULL);

        while (!searchRoutes.empty()) {
            Route r = searchRoutes.front();
            if (r.nodes.empty()) currL = customer_.ptr();
            else currL = (r.nodes.back()).segment->returnSegment()->source();

            searchRoutes.pop();

            for (unsigned int i = 1; i <= currL->segments();i++){
                Segment* testS = currL->segment(i).ptr();
                Location* testL = testS->returnSegment()->source();
                bool isDest = (testL->name().compare(destName) == 0);
                if (r.locations.count(testL)) continue;
                if (testL->type() == Location::customer() && !isDest)
                    continue; //non-destination customer

                Route newRoute = r;
                newRoute.totalTime = newRoute.totalTime.value() + segShipTime(testS);
                newRoute.nodes.push_back(RouteNode(testS));
                if (isDest) {
                    routes_.push_back(newRoute);
                } else {
                    newRoute.segments.insert(testS);
                    newRoute.locations.insert(testL);
                    searchRoutes.push(newRoute);
                }
            }
        }
        sort(routes_.begin(),routes_.end(),compareRoutes);
        for (vector<Route>::iterator it = routes_.begin(); it != routes_.end(); ++it){
            for (vector<RouteNode>::iterator nodeIt = it->nodes.begin(); nodeIt < it->nodes.end() - 1; ++nodeIt){
                nodeIt->next = &(*(nodeIt+1));
            }
        }
    }

    //ACTIVITY REACTORS - on status 
    void InjectActivityReactor::onStatus() {
        ActivityImpl::ManagerImpl::Ptr managerImpl = Fwk::ptr_cast<ActivityImpl::ManagerImpl>(manager_);
        switch (activity_->status()) {

        case Activity::executing:
            source_->shipmentNew( new Shipment(source_.ptr(), source_->destination(),source_->shipmentSize()));
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
            cout << "ForwardActivityReactor" << activity_->nextTime().value() << ", " << rate_ << endl; 
            if (shipment_->currRouteNode)
                shipment_->currRouteNode = shipment_->currRouteNode->next;
            forwardLoc_->shipmentNew(shipment_);
            
            break;
        case Activity::free:
            //each forward activity reactor is unique to a shipment
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