#include <math.h>
#include <vector>
#include <time.h>

#include <algorithm>
#include "Reactors.h"
#include "Ptr.h"
#include "PtrInterface.h"
#include "Activity.h"

namespace Shipping {
    void SegmentReactor::onShipmentDel()
    {
        segment_->source()->notifiee()->onSegmentShipmentDel(segment_.ptr());
    }

    void SegmentReactor::onShipmentNew(Shipment* shipment)
    {
        //cout << "Seg " << segment_->name() << " got shipment" << endl

        double traversals = ceil( ((double) (shipment->packages).value())/(fleet_->capacity(segment_->mode())).value());
        double travelTime = segment_->length().value() * traversals /fleet_->speed(segment_->mode()).value();
        float travelCost = segment_->length().value() * segment_->difficulty().value() * traversals * fleet_->cost(segment_->mode()).value();

        stringstream ss;
        ss << segment_->name()<<"_" << segment_->receivedShipments().value();
        shipment->totalCost = shipment->totalCost.value() + travelCost;
        shipment->totalTime = shipment->totalTime.value() + travelTime;
        if (shipment->act == NULL) {
            shipment->act = manager_->activityNew(ss.str());   
            shipment->act->lastNotifieeIs(new ForwardActivityReactor(manager_,shipment->act.ptr(),shipment));
            shipment->act->nextTimeIs( manager_->now().value() ); 
        }
        shipment->act->nextTimeIs(shipment->act->nextTime().value() + travelTime);
        shipment->act->statusIs(Activity::nextTimeScheduled);

    }

    void LocationReactor::onSegmentShipmentDel(Segment* seg){
        map<Segment*,queue<Shipment*> >::iterator it = holdQueues_.find(seg);
        if (it == holdQueues_.end()) return;
        queue<Shipment*> q = it->second;

        Shipment* s = q.front();
        q.pop();
        if (!seg->shipmentNew(s)) 
            q.push(s);
    }

    void LocationReactor::onShipmentRecieved(Shipment* s){
        vector<Segment*> & segTable = routingTable_[s->src];
        
        if (segTable.size() == 0) {
            cerr << "dropping impossible shipment" << endl;
            return;
        }

        //cout << "Location " << location_->name() << endl;
        for (vector<Segment*>::iterator it = segTable.begin(); it != segTable.end(); ++it){
            s->forwardSeg = *it;
            //cout << "Testing " << (*it)->name() << endl;
            if ((*it)->shipmentNew(s)) 
                return; //Shipment accepted package
        }
        holdQueues_[segTable[0]].push(s);
    }

    //CUSTOMER REACTOR
    void CustomerReactor::onShipmentRecieved(Shipment* s)
    {
        if (!s->dest->name().compare(customer_->name())) {
            ++recieved_;
            totalCost_ = totalCost_.value() + (s->totalCost).value();
            totalTime_ = totalTime_.value() + s->totalTime.value();
        } else if (!s->src->name().compare(customer_->name())){
            if (!routed_){
                findRoutes();
                routed_ = true;
            }
            if (routingTable_.empty()) {
                cerr << "Destination cannot be reached- dropping shipment..." << endl;
                manager_->activityDel(customer_->name());
            }

            vector<Segment*> & segTable = routingTable_[customer_.ptr()];
            for (vector<Segment*>::iterator it = segTable.begin(); it != segTable.end(); ++it){
                s->forwardSeg = *it;
                if ((*it)->shipmentNew(s)) return; //Shipment accepted package
            }
            srand(time(NULL));
            int idx = 0;
            if (segTable.size() >= 2) 
                idx = (int) max(0.0,(double) (rand() % segTable.size()) - 1);
            if (idx >= segTable.size()) idx = 0;
            holdQueues_[segTable[idx]].push(s);
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
                injectAct->nextTimeIs(manager_->now());
                injectAct->statusIs(Activity::nextTimeScheduled);
        }
    }

    void CustomerReactor::findRoutes(){
        if (routed_ || customer_->destination() == NULL || customer_->segments() == 0) 
            return;
        string destName = customer_->destination()->name();

        priority_queue<Route, vector<Route>, RouteCompare> connections;
        queue<Route> searchQ;
        searchQ.push(Route());

        while (!searchQ.empty()) {
            Route r = searchQ.front();
            searchQ.pop();

            Location* currL = (r.last) ? r.last->returnSegment()->source() : customer_.ptr();
            for (unsigned int i = 1; i <= currL->segments();i++){

                Segment* testS = currL->segment(i).ptr();
                Location* testL = testS->returnSegment()->source();

                bool isDest = (testL->name().compare(destName) == 0);
                if (r.nodes.find(testL) != r.nodes.end()) continue;
                if (testL->type() == Location::customer() && !isDest)
                    continue; //non-destination customer

                Route newRoute = r;
                newRoute.last = testS;
                newRoute.totalTime = newRoute.totalTime.value() + segShipTime(testS);
                newRoute.nodes[currL] = testS;
                if (isDest) connections.push(newRoute);
                else searchQ.push(newRoute);
            }
        }
        map<Location*,vector<Segment*> > tables; //non-customer -> segments on paths
        while(!connections.empty()){
            Route r = connections.top();
            connections.pop();
            for (map<Location*,Segment*>::iterator it = r.nodes.begin(); it != r.nodes.end(); ++it){
                tables[it->first].push_back(it->second);
            }
        }
        for(map<Location*,vector<Segment*> >::iterator midIt = tables.begin(); 
            midIt != tables.end(); ++midIt){
                midIt->first->notifiee()->routeTableIs(customer_.ptr(),midIt->second);
        }
    }

    //ACTIVITY REACTORS - on status 
    void InjectActivityReactor::onStatus() {
        ActivityImpl::ManagerImpl::Ptr managerImpl = Fwk::ptr_cast<ActivityImpl::ManagerImpl>(manager_);
        switch (activity_->status()) {

        case Activity::executing:
            source_->shipmentNew(new Shipment(source_.ptr(), source_->destination(),source_->shipmentSize()));
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
        Location * next; 
        switch (activity_->status()) {

        case Activity::executing:
            shipment_->forwardSeg->shipmentDel(shipment_);
            next = shipment_->forwardSeg->returnSegment()->source();
            if (next) next->shipmentNew(shipment_);
            break;
        case Activity::free:
            //each forward activity reactor is unique to a shipment, scheduled by SegmentReactor
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