#include "Engine.h"
#include "Entity.h"
#include "Nominal.h"
#include <queue>
#include <map>
#include <set>
#include <iostream>
#include <cstdio>
using namespace std;

namespace Shipping {
    ShippingNetwork::~ShippingNetwork(){
        for (map<string,Location::Ptr>::iterator it = locationMap.begin(); it!=locationMap.end(); ++it){
            locationDel(it->first);
        }
        for (map<string,Segment::Ptr>::iterator it = segmentMap.begin(); it!=segmentMap.end(); ++it){
            segmentDel(it->first);
        }
    }
    //----------| NotifieeConst Implementation |------------//
    ShippingNetwork::NotifieeConst::~NotifieeConst() {
        if(notifier_&&isNonReferencing()) notifier_->newRef();
    }
    void ShippingNetwork::NotifieeConst::notifierIs(const ShippingNetwork::PtrConst& _notifier) {
        ShippingNetwork::Ptr notifierSave(const_cast<ShippingNetwork *>(notifier_.ptr()));
        if(_notifier==notifier_) return;
        notifier_ = _notifier;
        if(notifierSave) {
            notifierSave->deleteNotifiee(this);
        }
        if(_notifier) {
            _notifier->newNotifiee(this);
        }
        if(isNonReferencing_) {
            if(notifierSave) notifierSave->newRef();
            if(notifier_) notifier_->deleteRef();
        }
    }
    void ShippingNetwork::NotifieeConst::isNonReferencingIs(bool _isNonReferencing){
        if(_isNonReferencing==isNonReferencing_) return;
        isNonReferencing_ = _isNonReferencing;
        if(notifier_) {
            if(_isNonReferencing) notifier_->deleteRef();
            else notifier_->newRef();
        }
    }

    // ----------| Attribute Implementation | --------//
    Location::Ptr ShippingNetwork::locationNew( Location::Ptr loc){
        if (locationMap.count(loc->name())) return NULL;
        locationMap[loc->name()] = loc;
        if (notifiee_) {
            notifiee_->onLocationNew(loc);
        }
        return loc;
    }
    void ShippingNetwork::locationDel( Fwk::String _name ){
        if (locationMap.find(_name) == locationMap.end()) return;
        Location::Ptr l = locationMap[_name];
        for (unsigned int i = 1; i <= l->segments(); i++){
            l->segment(i)->sourceIs(0);
        }
        if (notifiee_) {
            notifiee_->onLocationDel(l);
        }
        locationMap.erase(_name);
    }

    Segment::Ptr ShippingNetwork::segmentNew( Segment::Ptr seg ){
        if (segmentMap.count(seg->name())) return 0;
        segmentMap[seg->name()] = seg;
        if (notifiee_) 
            notifiee_->onSegmentNew(seg);
        return seg;
    }
    void ShippingNetwork::segmentDel( Fwk::String _name ){
        if (!segmentMap.count(_name)) return;
        Segment::Ptr s = segmentMap[_name];
        if (s->source())
            s->source()->segmentDel(s);
        if (notifiee_) 
            notifiee_->onSegmentDel(s);
        segmentMap.erase(_name);
    }

    Fleet::Ptr ShippingNetwork::fleetNew (Fwk::String _name){
        if (fleet_) return 0;
        fleet_ = Fleet::FleetNew(_name);
        return fleet_;
    }
    void ShippingNetwork::fleetDel(Fwk::String _name){
        if (!fleet_) return;
        if (fleet_->name() != _name)  return;
        fleet_ = 0;
    }

    string ShippingNetwork::connection(Location * start, Location *end) {
        string results = "";

        stringstream ss;
        ss.precision(2);
        ss << fixed;

        queue<Path> paths;
        Path p = Path(start, start->name(), 0.f, 0.f, 0.f, false);
        paths.push(p);

        while (!paths.empty()){
            Path currPath = paths.front();
            paths.pop();
            Location* currLoc = currPath.end;

            // path ends at destination
            if (currLoc->name().compare(end->name()) == 0) { 
                // add "COST^TIME^EXPEDITED <path>$" to results
                ss.str(""); 
                ss << currPath.cost.value() <<" "<< currPath.time.value() << ((currPath.expedited) ? " yes; ":" no; ") << currPath.pathStr << endl;
                
                results += ss.str();
                continue;
            } 
            for (unsigned int i = 1; i <= currLoc->segments(); i++) {
                addPathWithSegment(
                    currPath, currLoc->segment(i), paths,false);
            }
        }

        return results;
    }
    string ShippingNetwork::exploration(Location* start, ExplorationQuery & query){
        string results = ""; //String of paths to return
        queue<Path> paths;
        Path p = Path(start, start->name(), 0.f, 0.f, 0.f, query.expedited);
        paths.push(p);

        while (!paths.empty()){
            Path currPath = paths.front();
            paths.pop();

            //add path to result list if not start location only
            if (currPath.pathStr.find(")") != string::npos) {
                results += currPath.pathStr;
                results += "\n";
            }
            Location* currLoc = currPath.end;

            //add each segment to current path if doing so does not violate constraints
            for (unsigned int i = 1; i <= currLoc->segments(); i++) {
                Segment::Ptr seg = currLoc->segment(i);

                //check constraints valid 
                if (query.expedited && !seg->expediteSupport()) 
                    continue;
                if ( query.maxCost < (currPath.cost + travelCost(seg, query.expedited)))
                    continue;
                if ( query.maxTime < (currPath.time + travelTime(seg, query.expedited)))
                    continue;
                if ( query.maxDist < (currPath.length + seg->length()))
                    continue;

                addPathWithSegment(currPath, seg, paths, true);
            } 
        }
        return results;
    }

    void ShippingNetwork::addPathWithSegment(Path &currPath, Segment::Ptr seg, queue<Path>& paths, bool strictExpedite){
        Segment::Ptr returnSeg = seg->returnSegment();
        //check segment valid and not circling back
        if (!seg->returnSegment()) return;
        if (currPath.pathStr.find(returnSeg->source()->name()) != string::npos) 
            return;

        stringstream ss;
        ss.precision(2);
        ss << fixed;

        //construct "currPath... currLoc" + "(seg:segLength:seg')"
        ss.str("");
        ss << currPath.pathStr << "(" << seg->name() << ":"<<seg->length().value()<<":"<<returnSeg->name()<<") " << returnSeg->source()->name(); //add current end location of path

        if (!currPath.expedited) {
            paths.push(
                Path(returnSeg->source(), ss.str(), 
                currPath.length.value() + seg->length().value(), 
                currPath.cost.value() + travelCost(seg,false),
                currPath.time.value() + travelTime(seg,false),
                false)
            );
        }
        
        //add expedited path only if current path is expedited 
        //or path is new, not restricted by explore query 
        if (seg->expediteSupport()){
            if (currPath.expedited || 
                (!strictExpedite && currPath.pathStr.find('(') == string::npos)) {
                    paths.push( Path(returnSeg->source(), ss.str(), 
                        currPath.length.value() + seg->length().value(), 
                        currPath.cost.value() + travelCost(seg,true),
                        currPath.time.value() + travelTime(seg,true),
                        true)
                        );
            }
        }
    }

    float ShippingNetwork::travelCost(Segment::Ptr seg, bool expedited) {
        float transCost = (fleet_) ? fleet_->cost(seg->mode()).value() : 1.0;
        return (seg->expediteSupport() && expedited) ? seg->length().value() * seg->difficulty().value() * transCost * 1.5 : 
            seg->length().value() * seg->difficulty().value() * transCost;
    }

    float ShippingNetwork::travelTime(Segment::Ptr seg, bool expedited) {
        float transSpeed = (fleet_) ? fleet()->speed(seg->mode()).value() : 50;

        return (seg->expediteSupport() && expedited) ? 
            seg->length().value() / (transSpeed * 1.3) : seg->length().value() / transSpeed;
    }

    /* -------------- || Shipping Network Reactor || -----------------------*/
    void ShippingNetworkReactor::onSegmentExpediteChange( bool newExpedited )
    {
    	expeditedSegments += ((newExpedited) ? 1 : 0);
    }
    void ShippingNetworkReactor::onLocationNew(Location::Ptr loc) {
        if (!loc) return;
        if (loc->type() == loc->customer())
            entityCounts[customer_]++;
        else if (loc->type() == loc->port())
            entityCounts[port_]++;
        else if (loc->type() == loc->truckTerminal())
            entityCounts[truckTerminal_]++;
        else if (loc->type() == loc->boatTerminal())
            entityCounts[boatTerminal_]++;
        else if (loc->type() == loc->planeTerminal())
            entityCounts[planeTerminal_]++;
    }
    void ShippingNetworkReactor::onLocationDel(Location::Ptr loc) {
    	if (!loc) return;
        if (loc->type() == loc->customer())
            entityCounts[customer_]--;
        else if (loc->type() == loc->port())
            entityCounts[port_]--;
        else if (loc->type() == loc->truckTerminal())
            entityCounts[truckTerminal_]--;
        else if (loc->type() == loc->boatTerminal())
            entityCounts[boatTerminal_]--;
        else if (loc->type() == loc->planeTerminal())
            entityCounts[planeTerminal_]--;
    }

    void ShippingNetworkReactor::onSegmentNew(Segment::Ptr seg) { 
    	if (!seg) return;
        if (seg->mode() == Truck_) {
            entityCounts[truckSegment_]++;
        }
        else if (seg->mode() == Boat_) {
            entityCounts[boatSegment_]++;
        }
        else if (seg->mode() == Plane_) {
            entityCounts[planeSegment_]++;
        }
        if (seg->expediteSupport()) 
            expeditedSegments++;

        segmentreactors[seg->name()] = SegmentReactor::SegmentReactorIs(seg.ptr(), this);
    }
    void ShippingNetworkReactor::onSegmentDel(Segment::Ptr seg) {
    	if (!seg) return;
        if (seg->mode() == Truck_) {
            entityCounts[truckSegment_]--;
        } else if (seg->mode() == Boat_) {
            entityCounts[boatSegment_]--;
        } else if (seg->mode() == Plane_) {
            entityCounts[planeSegment_]--;
        }
        if (seg->expediteSupport())
            expeditedSegments--;
        segmentreactors.erase(seg->name());
    }

    unsigned int ShippingNetworkReactor::shippingEntities(StatsEntityType type) {
        if (type >= SHIPPING_ENTITY_COUNT) return 0;
        return entityCounts[type];
    }
    Percent ShippingNetworkReactor::expeditedPercent() {
        unsigned int segmentCount = entityCounts[truckSegment_] + entityCounts[boatSegment_] + entityCounts[planeSegment_];
        return (100.f * (float) expeditedSegments / (float) segmentCount);
    }
}
