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
        if (locationMap.count(loc->name())) return 0;
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
                ss << currPath.cost.value() <<" "<< currPath.time.value() << ((currPath.expedited) ? " yes;":" no;") << currPath.pathStr << endl;
                
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
                //cout << "DSSDGS" << currPath.pathStr;
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
                if ( query.maxCost <= (currPath.cost + travelCost(seg, query.expedited)) )
                    continue;
                if ( currPath.time >= (query.maxTime + travelTime(seg, query.expedited)) )
                    continue;
                if ( currPath.length >= query.maxDist + seg->length())
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
        return (seg->expediteSupport() && expedited) ? 
            seg->length().value() * (fleet_->cost(seg->mode()).value()) * 1.5 : 
            seg->length().value() * (fleet_->cost(seg->mode()).value());
    }

    float ShippingNetwork::travelTime(Segment::Ptr seg, bool expedited) {
        return (seg->expediteSupport() && expedited) ? 
            seg->length().value() / (fleet()->speed(seg->mode()).value() * 1.3) :
            seg->length().value() / fleet()->speed(seg->mode()).value();
    }

    /* -------------- || Shipping Network Reactor || -----------------------*/
    void ShippingNetworkReactor::onLocationNew(Location::Ptr loc) {
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

//----------| PRIVATE CALLS |----//
//typedef struct Node {
//    Location *loc;
//    string *path;
//    float dist;
//    float cost;
//    float time;
//    Node (Location * l, string * p) : loc(l), path(p), dist(0.f), cost(0.f), time(0.f) {}
//} Node_T;

/* Explore - explore:true, loc:start location, dst:NULL, 
if not needed , set max_dist:FLT_MAX, max_cost:FLT_MAX, max_time:FLT_MAX, expedited:false
Connect - explore:false, loc:start location, dst:end location,
max_dist, max_cost, max_time can be any value. If not needed, set expedited:false
string ShippingNetwork::explore(Location* loc, Location* dst, Mile max_dist, Cost max_cost, 
Time max_time, bool expedited, bool exploration) {
map<string, bool> nodes_traversed;
std::queue<Node_T> search_queue;

std::stringstream ss;
int pass = (exploration && expedited) ? 1 : 0; 

map<string, bool> exploration_path_printed;

// traverse (at most) two times. First time non-expedited, second time (if requested) expedited 
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
if ( (exploration && (current_cost + travelCost(seg, false) > max_cost)) || // cost
(exploration && (current_time + travelTime(seg, false) > max_time)) ) { // time
continue;
}
} else { // expedited
if ((exploration && (current_cost + travelCost(seg, true) > max_cost)) || // cost
(exploration && (current_time + travelTime(seg, true) > max_time))) { // time
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
node.cost = current_cost.value() + travelCost(seg, false);
node.time = current_time.value() + travelTime(seg, false);
}
else { // expedited
node.cost = current_cost.value() + travelCost(seg, true);
node.time = current_time.value() + travelTime(seg, true);
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
*/