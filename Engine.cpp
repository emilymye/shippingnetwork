#include "Engine.h"
#include "Entity.h"
#include <queue>
#include <iostream>

namespace Shipping {
    using namespace std;

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
        if (locationMap.find(loc->name()) != locationMap.end()) return 0;
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
        if (segmentMap.find(seg->name()) != segmentMap.end()) return 0;
        segmentMap[seg->name()] = seg;
        if (notifiee_) notifiee_->onSegmentNew(seg);
        return seg;
    }
    void ShippingNetwork::segmentDel( Fwk::String _name ){
        if (segmentMap.find(_name) == segmentMap.end()) return;
        Segment::Ptr s = segmentMap[_name];
        s->source()->segmentDel(s);
        if (notifiee_) notifiee_->onSegmentDel(s);
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

    string ShippingNetwork::path(Fwk::String startLocation, Fwk::String endLocation){
        return explore(locationMap[startLocation].ptr(),locationMap[endLocation].ptr(),
            0.f, 0.f, 0.f,false,false);
    }

    string ShippingNetwork::path(Fwk::String startLocation, ExplorationQuery q){
        Location* loc = locationMap[startLocation].ptr();
        return explore(loc, 0, q.maxDist,q.maxCost,q.maxTime,q.expedited,true);
    }

    //----------| Shipping Network Reactor |----//
    void ShippingNetworkReactor::onLocationNew(Location::Ptr loc) {
        if (loc->type() == loc->other()) return;
        entityCounts[loc->type() - loc->customer()]++;
    }

    void ShippingNetworkReactor::onSegmentNew(Segment::Ptr seg) {        
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

    void ShippingNetworkReactor::onLocationDel(Location::Ptr loc) {
        if (loc->type() == loc->other()) return;
        entityCounts[loc->customer() + loc->type()]--;
    }

    void ShippingNetworkReactor::onSegmentDel(Segment::Ptr seg) {
        if (seg->mode() == Truck_) {
            entityCounts[truckSegment_]--;
        } else if (seg->mode() == Boat_) {
            entityCounts[boatSegment_]--;
        } else if (seg->mode() == Plane_) {
            entityCounts[planeSegment_]--;
        }
        if (seg->expediteSupport()) expeditedSegments--;
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

    //----------| PRIVATE CALLS |----//
    typedef struct Node {
        Location *loc;
        string *path;
        float dist;
        float cost;
        float time;
        Node (Location * l, string * p) : loc(l), path(p), dist(0.f), cost(0.f), time(0.f) {}
    } Node_T;

    /* Explore - explore:true, loc:start location, dst:NULL, 
    if not needed , set max_dist:FLT_MAX, max_cost:FLT_MAX, max_time:FLT_MAX, expedited:false
    Connect - explore:false, loc:start location, dst:end location,
    max_dist, max_cost, max_time can be any value. If not needed, set expedited:false*/
    string ShippingNetwork::explore(Location* loc, Location* dst, Mile max_dist, Cost max_cost, 
        Time max_time, bool expedited, bool exploration) {
            map<string, bool> nodes_traversed;
            std::queue<Node_T> search_queue;

            std::stringstream ss;
            int pass = (exploration && expedited) ? 1 : 0; 

            map<string, bool> exploration_path_printed;

            /* traverse (at most) two times. First time non-expedited, second time (if requested) expedited */
            for (; pass < 2; pass++) { // pass=0:non-expedited  pass=1:expedited
                Node_T node = Node_T(loc, new string (loc->name()));
                search_queue.push(node);

                nodes_traversed.erase(nodes_traversed.begin(), nodes_traversed.end());
                nodes_traversed[loc->name()] = true;

                while (!search_queue.empty()) {
                    Location *current_loc = search_queue.front().loc;
                    string *current_path = search_queue.front().path;
                    Mile current_distance = search_queue.front().dist;
                    Cost current_cost = search_queue.front().cost;
                    Time current_time = search_queue.front().time;
                    search_queue.pop();

                    if (!exploration && current_loc == dst) { // print path
                        if (!pass) { // non-expedited
                            ss.precision(2);
                            ss << fixed << current_cost.value() << ' ';
                            ss.precision(4);
                            ss << fixed << current_time.value() << " no; " << *current_path << endl;
                        }
                        else { //expedited
                            ss.precision(2);
                            ss << fixed << current_cost.value() << ' ';
                            ss.precision(4);
                            ss << fixed << current_time.value() << " yes; " << *current_path << endl;
                        }
                    }

                    bool newNode = false;
                    for (unsigned int i = 1; i <= current_loc->segments(); i++) {
                        Segment::Ptr seg = current_loc->segment(i);
                        if (pass == 1 && !seg->expediteSupport()) {
                            continue;
                        }
                        if (exploration && (current_distance + seg->length() > max_dist)) { // distance
                            continue;
                        }
                        if (!pass) { // non-expedited
                            if ( (exploration && (current_cost + segTravCost(seg, false) > max_cost)) || // cost
                                (exploration && (current_time + segTravTime(seg, false) > max_time)) ) { // time
                                    continue;
                            }
                        } else { // expedited
                            if ((exploration && (current_cost + segTravCost(seg, true) > max_cost)) || // cost
                                (exploration && (current_time + segTravTime(seg, true) > max_time))) { // time
                                    continue;
                            }
                        }

                        Location *seg_end = seg->returnSegment()->source();
                        if (nodes_traversed.find(seg_end->name()) == nodes_traversed.end()){
                            nodes_traversed[seg_end->name()] = true;
                            Node_T node (seg_end, new string(*current_path));
                            stringstream append_ss;
                            append_ss.precision(2);
                            append_ss << "(" << seg->name() << ":" << fixed << seg->length().value() << 
                                ":" << seg->name() << ") " << seg_end->name();
                            string appendStr = append_ss.str();
                            node.path->append(appendStr);
                            node.dist = current_distance.value() + seg->length().value();
                            if (pass == 0) { // non-expedited
                                node.cost = current_cost.value() + segTravCost(seg, false);
                                node.time = current_time.value() + segTravTime(seg, false);
                            }
                            else { // expedited
                                node.cost = current_cost.value() + segTravCost(seg, true);
                                node.time = current_time.value() + segTravTime(seg, true);
                            }
                            search_queue.push(node);
                            newNode = true;

                            if (exploration) { // print path
                                if (exploration_path_printed.find(*(node.path)) == exploration_path_printed.end()) {
                                    exploration_path_printed[*(node.path)] = true;
                                    ss <<  *(node.path) << endl;
                                }
                            }
                        }
                    }
                    delete current_path;
                }
            }
            return ss.str();
    }

    float ShippingNetwork::segTravCost(Segment::Ptr seg, bool expedited) {
        if (seg->expediteSupport() && expedited) {
            return seg->length().value() * (fleet()->cost(seg->mode()).value() * 1.5);
        }
        else {
            return seg->length().value() * fleet()->cost(seg->mode()).value();
        }
    }

    float ShippingNetwork::segTravTime(Segment::Ptr seg, bool expedited) {
        if (seg->expediteSupport() && expedited) {
            return seg->length().value() / (fleet()->speed(seg->mode()).value() * 1.3);
        }
        else {
            return seg->length().value() / fleet()->speed(seg->mode()).value();
        }
    }

}
