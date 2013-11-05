#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include "fwk/Exception.h"
#include "fwk/BaseNotifiee.h"
#include "fwk/NamedInterface.h"
#include "fwk/HashMap.h"
#include "fwk/String.h"
#include "fwk/Ptr.h"
#include "fwk/PtrInterface.h"
#include "Instance.h"
#include "Entity.h"
#include "Nominal.h"

namespace Shipping {
    //ordinal value types
    class Time : public Ordinal<Time, float> {
    public:
        Time(float num) : Ordinal<Time, float>(num){
            if (num < 0 ) throw Fwk::RangeException("Time");
        }
    };

    class ShippingNetwork : public Fwk::NamedInterface {
    public:
        typedef Fwk::Ptr<ShippingNetwork const> PtrConst;
        typedef Fwk::Ptr<ShippingNetwork> Ptr;
        ShippingNetwork::Ptr ShippingNetworkNew();
        ~ShippingNetwork();

        /* NOTIFIEE IMPLEMENTATION ==============================================*/
        class NotifieeConst : public virtual Fwk::NamedInterface::NotifieeConst {
        public:
            typedef Fwk::Ptr<NotifieeConst const> PtrConst;
            typedef Fwk::Ptr<NotifieeConst> Ptr;
            Fwk::String name() const { return (notifier_?notifier_->name():Fwk::String()); }
            ShippingNetwork::PtrConst notifier() const { return notifier_; }
            bool isNonReferencing() const { return isNonReferencing_; }

            // Non-const interface =============================================
            ~NotifieeConst();
            virtual void notifierIs(const ShippingNetwork::PtrConst& _notifier);
            void isNonReferencingIs(bool _isNonReferencing);
            static NotifieeConst::Ptr NotifieeConstIs() { 
                Ptr m = new NotifieeConst();
                return m;
            }
            virtual void onLocationNew( Location::Ptr ) {};
            virtual void onSegmentNew( Segment::Ptr ) {};
            // Constructors ====================================================
        protected:
            ShippingNetwork::PtrConst notifier_;
            bool isNonReferencing_;
            Fwk::String tacKeyForLocation_;
            NotifieeConst(): Fwk::NamedInterface::NotifieeConst(),
                isNonReferencing_(false){}
        };

        class Notifiee : public virtual NotifieeConst, public virtual Fwk::NamedInterface::Notifiee {
        public:
            typedef Fwk::Ptr<Notifiee const> PtrConst;
            typedef Fwk::Ptr<Notifiee> Ptr;
            ShippingNetwork::PtrConst notifier() const { return NotifieeConst::notifier(); }
            ShippingNetwork::Ptr notifier() { return const_cast<ShippingNetwork *>(NotifieeConst::notifier().ptr()); }
            // Non-const interface =============================================

            static Notifiee::Ptr NotifieeIs() {
                Ptr m = new Notifiee();
                return m;
            }
            // Constructors ====================================================
        protected:
            Notifiee(): Fwk::NamedInterface::Notifiee() {}
        };
        ShippingNetwork::Notifiee * notifiee() const {return notifiee_;}
        /* END NOTIFIEE IMPLEMENTATION ==============================================*/     

        //LOCATION ==============================================
        typedef Fwk::HashMap<Location, Fwk::String, Location, Location::PtrConst, Location::Ptr> LocationDict;
        Location::Ptr LocationNew( Location::Ptr newLocation);
        Location const* location( Fwk::String _name) const { return location_[_name]; };
        Location* location( Fwk::String _name) { return location_[_name]; };

        //SEGMENT ==============================================
        typedef Fwk::HashMap<Segment, Fwk::String, Segment, Segment::PtrConst, Segment::Ptr> SegmentDict;
        Segment::Ptr SegmentNew( Segment::Ptr newSegment );
        Segment const* segment( Fwk::String _name) const { return segment_[_name]; };
        Segment * segment( Fwk::String _name) { return segment_[_name]; };

        //FLEET ==============================================
        typedef Fwk::HashMap<Fleet, Fwk::String, Fleet, Fleet::PtrConst, Fleet::Ptr> FleetDict;
        Fleet::Ptr FleetNew(  Fleet::Ptr newFleet );
        Fleet const* fleet( Fwk::String _name) const { return fleet_[_name]; };
        Fleet * fleet( Fwk::String _name) { return fleet_[_name]; };

        //CONNECTIVITY ==============================================
        struct ExplorationQuery{
            Mile maxDist;
            Cost maxCost;
            Time maxTime;
            bool expedited;
            ExplorationQuery(float _dist, float _cost, float _time, bool _expedited) : 
                maxDist(_dist),maxCost(_cost), maxTime(_time), expedited(_expedited) {};
        };
        Fwk::String exploration( Fwk::String startLocation,  ExplorationQuery query); 
        Fwk::String connection( Fwk::String startLocation, Fwk::String endLocation );
    protected:
        ShippingNetwork::Notifiee * notifiee_;
        void notifieeIs( ShippingNetwork::Notifiee *  n) const {
            ShippingNetwork* me = const_cast<ShippingNetwork*>(this);
            me->notifiee_ = n;
        }
        LocationDict location_;
        SegmentDict segment_;
        FleetDict fleet_;
    };

    class Percent : Ordinal<Percent,float>{
        Percent(float num):Ordinal<Percent,float>(num){
            if (num < 0) throw Fwk::RangeException("Percent");
        }
    };

    class ShippingNetworkReactor : public ShippingNetwork::Notifiee{
    public:
        static ShippingNetworkReactor::Ptr ShippingNetworkReactorIs( ShippingNetwork* sn) {
            Ptr m = new ShippingNetworkReactor(sn);
            return m;
        }

        void onLocationNew(Location::Ptr loc);
        void onSegmentNew(Segment::Ptr seg);

        enum StatisticType{
            customer_,
            port_,
            truckTerminal_,
            boatTerminal_,
            planeTerminal_,
            truckSegment_,
            boatSegment_,
            planeSegment_,
            SHIPPING_ENTITY_COUNT
        };
        unsigned int shippingEntities( StatisticType type );
        static inline StatisticType customer() { return customer_; };
        static inline StatisticType port() { return port_; };
        static inline StatisticType truckTerminal() { return truckTerminal_; };
        static inline StatisticType boatTerminal() { return boatTerminal_; };
        static inline StatisticType planeTerminal() { return planeTerminal_; };
        static inline StatisticType truckSegment() { return truckSegment_; };
        static inline StatisticType boatSegment() { return boatSegment_; };
        static inline StatisticType planeSegment() { return planeSegment_; };
        static StatisticType StatisticInstance ( Fwk::String );

        Percent expeditedPercent();
    protected:
        ShippingNetworkReactor(ShippingNetwork * sn) : expeditedSegments(0), ShippingNetwork::Notifiee() { notifierIs(sn); }
        unsigned int expeditedSegments;
        unsigned int entityCounts [SHIPPING_ENTITY_COUNT];
    };


}

#endif
