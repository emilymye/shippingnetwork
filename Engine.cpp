#include "Engine.h"
#include "Entity.h"
#include <queue>
#include <iostream>

namespace Shipping {
    using namespace std;
    ShippingNetwork::~ShippingNetwork(){
        
    
    }
    //----------| NotifieeConst Implementation |------------//
    ShippingNetwork::NotifieeConst::~NotifieeConst() {
        if(notifier_&&isNonReferencing()) notifier_->newRef();
    }

    void ShippingNetwork::NotifieeConst::notifierIs(const ShippingNetwork::PtrConst& _notifier) {
            ShippingNetwork::Ptr notifierSave(const_cast<ShippingNetwork *>(notifier_.ptr()));
            if(_notifier==notifier_) return;
            notifier_ = _notifier;
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

    //----------| Notifiee Implementation |------------//

    //Fwk::String
    //    ShippingNetwork::attributeString( Fwk::RootNotifiee::AttributeId a ) const {
    //        Fwk::String str = "unknown";
    //        switch(a) {
    //        case NotifieeConst::version__: str = "version"; break;
    //        case NotifieeConst::notificationException__: str = "notificationException"; break;
    //        case NotifieeConst::cell__: str = "location"; break;
    //        default: str = Fwk::RootNotifiee::attributeString(a);
    //        }
    //        return str;
    //}

    /* Explore - explore:true, loc:start location, dst:NULL, 
                 if not needed , set max_dist:FLT_MAX, max_cost:FLT_MAX, max_time:FLT_MAX, expedited:false
       Connect - explore:false, loc:start location, dst:end location,
                 max_dist, max_cost, max_time can be any value. If not needed, set expedited:false*/
    typedef struct Node {
        Location *loc;
        string *path;
        float dist;
        float cost;
        float time;
    } Node_T;


    void ShippingNetwork::explore(Location* loc, Location* dst, Mile max_dist, Cost max_cost, Time max_time, bool expedited, bool explore) {
        map<string, bool> nodes_traversed;
        std::queue<Node_T> search_queue;
        Node_T node;
        node.loc = loc;
        node.path = new string(loc->name());
        node.dist = 0.0;
        node.cost = 0.0;
        node.time = 0.0;
        search_queue.push(node);
        nodes_traversed[loc->name()] = true;

        int pass;
        if (explore && expedited) {
            pass = 1;
        }
        else {
            pass = 0;
        }
        /* traverse (at most) two times. First time non-expedited, second time (if requested) expedited */
        for (; pass < 2; pass++) { // pass=0:non-expedited  pass=1:expedited
            while (!search_queue.empty()) {
                Location *current_loc = search_queue.front().loc;
                string *current_path = search_queue.front().path;
                Mile current_distance = search_queue.front().dist;
                Cost current_cost = search_queue.front().cost;
                Time current_time = search_queue.front().time;
                search_queue.pop();
                if (!explore && current_loc == dst) { // print path
                    if (pass == 0) { // non-expedited
                        std::cout << (float)current_cost << ' ' << current_time << " no; " << *current_path << endl;
                    }
                    else { //expedited
                        std::cout << current_cost << ' ' << current_time << " yes; " << *current_path << endl;
                    }
                }

                bool newNode = false;
                for (unsigned int i = 0; i < current_loc->num_segments(); i++) {
                    Segment::Ptr seg = current_loc->segment(i);
                    if (pass == 1 && !seg->expediteSupport()) {
                        continue;
                    }
                    if (explore && (current_distance + seg->length() > max_dist)) { // distance
                        continue;
                    }
                    if (pass == 0) { // non-expedited
                        if (explore && (current_cost + segTravCost(seg, false) > max_cost)) { // cost
                            continue;
                        }
                        if (explore && (current_time + segTravTime(seg, false) > max_time)) { // time
                            continue;
                        }
                    }
                    else { // expedited
                        if (explore && (current_cost + segTravCost(seg, true) > max_cost)) { // cost
                            continue;
                        }
                        if (explore && (current_time + segTravTime(seg, true) > max_time)) { // time
                            continue;
                        }
                    }

                    Location *seg_end = seg->returnSegment()->source();
                    if (nodes_traversed.find(seg_end->name()) == map::end) {
                        nodes_traversed[seg_end->name()] = true;
                        Node_T node;
                        node.loc = seg_end;
                        node.path = new string(*current_path);
                        node.path->append("("+seg->name().String()+":"+seg->length().value()+":"+seg->name().String()+") "+seg_end->name().String());
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
                    }
                }

                if (explore && !newNode) { // print path
                    std::cout << *current_path << endl;
                }
                delete current_path;
            }
        }
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
