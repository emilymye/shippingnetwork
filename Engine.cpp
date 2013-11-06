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



}