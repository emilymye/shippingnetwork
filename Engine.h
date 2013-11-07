#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include "fwk/Exception.h"
#include "fwk/BaseNotifiee.h"
#include "fwk/NamedInterface.h"
#include "fwk/String.h"
#include "fwk/Ptr.h"
#include "fwk/PtrInterface.h"
#include "Instance.h"
#include "Entity.h"
#include "Nominal.h"
#include <map>
namespace Shipping {
    using namespace std;
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
        static ShippingNetwork::Ptr ShippingNetworkNew(Fwk::String _name) { 
            Ptr m = new ShippingNetwork(_name);
            return m;
        }
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
        Location::Ptr locationNew( Location::Ptr l);
        Location::Ptr locationDel( Fwk::String _name );
        Location* location( Fwk::String _name) { return location_[_name].ptr(); };

        //SEGMENT =============================================
        Segment::Ptr segmentNew( Segment::Ptr s );
        Segment::Ptr segmentDel( Fwk::String _name );
        Segment * segment( Fwk::String _name) { return segment_[_name].ptr(); };

        //FLEET ==============================================
        Fleet::Ptr fleetNew (Fwk::String _name);
        Fleet::Ptr fleetDel (Fwk::String _name); 
        Fleet * fleet() { return fleet_; };


        //CONNECTIVITY ==============================================
        struct ExplorationQuery{
            Mile maxDist;
            Cost maxCost;
            Time maxTime;
            bool expedited;
            ExplorationQuery() : maxDist(FLT_MAX),maxCost(FLT_MAX), maxTime(FLT_MAX), expedited(false) {}
        };
       
        string path(Fwk::String startLocation, Fwk::String endLocation);
        string path(Fwk::String startLocation, ExplorationQuery query);
    protected:
        ShippingNetwork(Fwk::String _name) : Fwk::NamedInterface (_name), fleet_(0) {}
        ShippingNetwork::Notifiee * notifiee_;
        float segTravCost(Segment::Ptr, bool);
        float segTravTime(Segment::Ptr, bool);
        void notifieeIs( ShippingNetwork::Notifiee *  n) const {
            ShippingNetwork* me = const_cast<ShippingNetwork*>(this);
            me->notifiee_ = n;
        }
    private:
        void explore(
            Location* loc, Location* dst,
            Mile max_dist, Cost max_cost, Time max_time, bool expedited, bool exploration);
        map<string,Location::Ptr> location_;
        map<string,Segment::Ptr> segment_;
        Fleet* fleet_;

    };

    class Percent : public Ordinal<Percent,float>{
        Percent(float num):Ordinal<Percent,float>(num){
            if (num < 0) throw Fwk::RangeException("Percent");
        }
    };

    class ShippingNetworkReactor : public ShippingNetwork::Notifiee{
    public:
        typedef Fwk::Ptr<ShippingNetworkReactor const> PtrConst;
        typedef Fwk::Ptr<ShippingNetworkReactor> Ptr;

        static ShippingNetworkReactor::Ptr ShippingNetworkReactorIs( ShippingNetwork* sn) {
            ShippingNetworkReactor::Ptr m = new ShippingNetworkReactor(sn);
            return m;
        }

        void onLocationNew(Location::Ptr loc);
        void onSegmentNew(Segment::Ptr seg);

        enum StatsEntityType{
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
        static inline StatsEntityType customer() { return customer_; };
        static inline StatsEntityType port() { return port_; };
        static inline StatsEntityType truckTerminal() { return truckTerminal_; };
        static inline StatsEntityType boatTerminal() { return boatTerminal_; };
        static inline StatsEntityType planeTerminal() { return planeTerminal_; };
        static inline StatsEntityType truckSegment() { return truckSegment_; };
        static inline StatsEntityType boatSegment() { return boatSegment_; };
        static inline StatsEntityType planeSegment() { return planeSegment_; };

        unsigned int shippingEntities( StatsEntityType type );
        Percent expeditedPercent();
    protected:
        ShippingNetworkReactor(ShippingNetwork * sn) : expeditedSegments(0), ShippingNetwork::Notifiee() { notifierIs(sn); }
        unsigned int expeditedSegments;
        unsigned int entityCounts [SHIPPING_ENTITY_COUNT];
    };

}

#endif
