#include "Engine.h"

namespace Shipping {
    ShippingNetwork::~ShippingNetwork(){
        LocationDict::Iterator li = location_.iterator();
        while (li.ptr()){
            locationDel( li->fwkKey());
            li = location_.iterator();
        }
        SegmentDict::Iterator si = segment_.iterator();
        while (si.ptr()){
            segmentDel( li->fwkKey());
            si = segment_.iterator();
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
    //        case NotifieeConst::cell__: str = "cell"; break;
    //        default: str = Fwk::RootNotifiee::attributeString(a);
    //        }
    //        return str;
    //}

    void ShippingNetwork::explore(Location* loc, Location* dst, Mile max_dist, Cost max_cost, Time max_time, bool expedited, bool explore) {
        typedef struct Node {
            Location *loc;
            string *path;
            Mile dist;
            Cost cost;
            Time time;
        } Node_T;
        map<string, bool> nodes_traversed;
        queue<Node_T> search_queue;
        Node_t node;
        node.loc = loc;
        node.path = new string(loc->name());
        node.distance = 0;
        node.cost = 0;
        node.time = 0;
        search_queue.push(Node_T);
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
                        cout << current_cost << ' ' << current_time << " no; " << *current_path << endl;
                    }
                    else { //expedited
                        cout << current_cost << ' ' << current_time << " yes; " << *current_path << endl;
                    }
                }

                bool newNode = false;
                for (int i = 0; i < current_loc->segment_->size(); i++) {
                    Segment *seg = current_loc->segment_[i];
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
                        node->loc = seg_end;
                        node->path = new string(*current_path);
                        node->path->append("("+seg->name()+":"+seg->length()+":"+seg->name()+") "+seg_end->name());
                        node->dist = current_dist + seg->length();
                        if (pass == 0) { // non-expedited
                            node->cost = current_cost + segTravCost(seg, false);
                            node->time = current_time + segTravTime(seg, false);
                        }
                        else { // expedited
                            node->cost = current_cost + segTravCost(seg, true);
                            node->time = current_time + segTravTime(seg, true);
                        }
                        search_queue.push(seg_end);
                        new_node = true;
                    }
                }

                if (explore && !new_node) { // print path
                    cout << *current_path << endl;
                }
                delete current_path;
            }
        }
    }

Cost segTravCost(Segment* seg, bool expedited) {
    if (seg->expediteSupport() && expedited) {
        return seg->length() * (fleet()->cost(seg->mode()) * 1.5);
    }
    else {
        return seg->length() * fleet()->cost(seg->mode());
    }
}

Time segTravTime(Segment* seg, bool expedited) {
    if (seg->expediteSupport() && expedited) {
        return seg->length() / (fleet()->speed(seg->mode()) * 1.3);
    }
    else {
        return seg->length() / fleet()->speed(seg->mode());
    }
}

}
