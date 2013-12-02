#ifndef ENTITY_H
#define ENTITY_H

#include "Nominal.h"
#include "fwk/Exception.h"
#include "fwk/NamedInterface.h"
#include "fwk/HashMap.h"
#include "fwk/LinkedList.h"
#include "fwk/String.h"
#include "fwk/Ptr.h"
#include "fwk/PtrInterface.h"
#include "Instance.h"
#include <vector>

namespace Shipping{
    class Difficulty : public Ordinal<Difficulty,float> {
    public:
        Difficulty(float num = 1.0) : Ordinal<Difficulty, float>(num){
            if ( num < 1.0 || num > 5.0 ){
                throw Fwk::RangeException("Difficulty");
            }
        }
    };
    class Mile : public Ordinal<Mile, float> {
    public:
        Mile(float num) : Ordinal<Mile,float>(num) {
            if (num < 0.f) throw Fwk::RangeException("Mile");
        };
    };
    class Speed : public Ordinal<Speed, float> {
    public:
        Speed(float num) : Ordinal<Speed, float>(num){
            if (num < 0 ) throw Fwk::RangeException("Speed");
        }
    };
    class Cost : public Ordinal<Cost, float> {
    public:
        Cost(float num) : Ordinal<Cost, float>(num){
            if (num < 0 ) throw Fwk::RangeException("Cost");
        }
        Cost expedited() { return (value() * 1.5); }
             
    };
    class Capacity : public Ordinal<Capacity, int> {
    public:
        Capacity(int num) : Ordinal<Capacity, int>(num) {
            if (num < 0) throw Fwk::RangeException("Capacity");
        };
    };

    enum ShippingMode {
        Truck_,
        Boat_,
        Plane_,
        MODE_COUNT //DO NOT MOVE
    };

    class Location; 

    class Segment : public Fwk::NamedInterface {
    public: 
        typedef Fwk::Ptr<Segment const> PtrConst;
        typedef Fwk::Ptr<Segment> Ptr;
        static Segment::Ptr SegmentNew(Fwk::String _name, ShippingMode _mode){
            Ptr m = new Segment(_name, _mode);
            return m;
        }
        ~Segment() { }

        class NotifieeConst : public virtual Fwk::NamedInterface::NotifieeConst {
        public:
            typedef Fwk::Ptr<NotifieeConst const> PtrConst;
            typedef Fwk::Ptr<NotifieeConst> Ptr;
            Fwk::String name() const { return (notifier_?notifier_->name():Fwk::String()); }
            Segment::PtrConst notifier() const { return notifier_; }
            void notifierIs(const Segment::PtrConst& _notifier) {
                Segment::Ptr notifierSave(const_cast<Segment *>(notifier_.ptr()));
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
            bool isNonReferencing() const { return isNonReferencing_; }
            ~NotifieeConst(){
                if(notifier_&&isNonReferencing()) notifier_->newRef();
            }
            virtual void onExpediteChange( bool isExpedited ) {};

            // Constructors ====================================================
        protected:
            Segment::PtrConst notifier_;
            bool isNonReferencing_;
            Fwk::String tacKeyForLocation_;
            NotifieeConst(): Fwk::NamedInterface::NotifieeConst(),
                isNonReferencing_(false){}
        };
        class Notifiee : public virtual NotifieeConst, public virtual Fwk::NamedInterface::Notifiee {
        public:
            typedef Fwk::Ptr<Notifiee const> PtrConst;
            typedef Fwk::Ptr<Notifiee> Ptr;
            Segment::PtrConst notifier() const { return NotifieeConst::notifier(); }
            Segment::Ptr notifier() { return const_cast<Segment *>(NotifieeConst::notifier().ptr()); }
            static Notifiee::Ptr NotifieeIs() {
                Ptr m = new Notifiee();
                return m;
            }
        protected:
            Notifiee(): Fwk::NamedInterface::Notifiee() {}
        };
        Segment::NotifieeConst* notifiee() const {return notifiee_;}

        ShippingMode mode() const { return mode_; };

        Location const * source() const { return source_; }
        Location * source() { return source_; }
        void sourceIs( Location* _loc ) { source_ = _loc; }

        Segment::PtrConst returnSegment() const { return returnSegment_; }
        Segment::Ptr returnSegment() { return returnSegment_; }
        void returnSegmentIs( Segment::Ptr returnSeg ) { returnSegment_ = returnSeg.ptr(); }

        Mile length() const { return length_; }
        void lengthIs( Mile newLength ) { length_ = newLength; }

        Difficulty difficulty() const { return difficulty_; }
        void difficultyIs(Difficulty d)  { difficulty_ = d; }

        bool expediteSupport() const { return expediteSupport_; }
        void expediteSupportIs( bool support ) { 
            if (support == expediteSupport_) return;
            expediteSupport_ = support; 
            if (notifiee_) notifiee_->onExpediteChange(support);
        }
    protected:
        Segment::NotifieeConst * notifiee_;
        void newNotifiee( Segment::NotifieeConst * n ) const {
            Segment* me = const_cast<Segment*>(this);
            me->notifiee_ = n;
        }
        void deleteNotifiee( Segment::NotifieeConst * n ) const {
            Segment* me = const_cast<Segment*>(this);
            me->notifiee_ = 0;
        }
        void notifieeIs( Segment::Notifiee * n) const {
            Segment* me = const_cast<Segment*>(this);
            me->notifiee_ = n;
        }
        Segment (const Segment&);
        Segment(Fwk::String _name, ShippingMode _mode) : 
            Fwk::NamedInterface(_name), mode_(_mode), length_(0.f),difficulty_(1.f) {}
        mutable Segment::Ptr fwkHmNext_; 

        ShippingMode mode_;
        Location * source_; 
        Segment* returnSegment_;
        Mile length_;
        Difficulty difficulty_;
        bool expediteSupport_;
    };

    // START LOCATION CLASSES ===============================================
    class Location : public Fwk::NamedInterface {
    public:
        typedef Fwk::Ptr<Location const> PtrConst;
        typedef Fwk::Ptr<Location> Ptr;

        enum LocationType{
            customer_,
            port_,
            truckTerminal_,
            boatTerminal_,
            planeTerminal_,
            other_
        };
        static inline LocationType customer() { return customer_; }
        static inline LocationType port() { return port_; }
        static inline LocationType terminalIdx() { return truckTerminal_;}
        static inline LocationType truckTerminal() { return truckTerminal_; }
        static inline LocationType boatTerminal() { return boatTerminal_; }
        static inline LocationType planeTerminal() { return planeTerminal_; }
        static inline LocationType other() { return customer_; }
        LocationType type() const { return type_; }
        void typeIs( LocationType _type ) { type_ = _type; }

        static Location::Ptr LocationNew(Fwk::String _name, LocationType _type){
            Ptr m = new Location(_name, _type);
            return m;
        }
        ~Location() { }
        Segment::PtrConst segment( unsigned int num ) const { 
            if ( !num  || num > segment_.size()) return 0;
            return segment_[num - 1]; 
        }
        Segment::Ptr segment( unsigned int num ) { 
            if ( !num || num > segment_.size()) return 0;
            return segment_[num - 1]; 
        }
        unsigned int segments() { return segment_.size(); }
        Segment::Ptr segmentNew ( Segment::Ptr seg ){ 
            segment_.push_back(seg); 
            return seg;
        }
        void segmentDel( Segment::Ptr seg ) { 
            for (unsigned int i = 0; i < segment_.size(); i++){
                if (!segment_[i]->name().compare(seg->name())){
                    segment_.erase( segment_.begin() + i);
                    return;
                }
            }
        }
    protected:
        Location ( const Location&);
        explicit Location(Fwk::String _name, LocationType _type) : Fwk::NamedInterface(_name), type_(_type) {}
        mutable Location::Ptr fwkHmNext_; 
        LocationType type_;
        std::vector<Segment::Ptr> segment_;
    };

    // LOCATION SUBCLASSES ==============================================
    class CustomerLocation : public Location {
    public:
        typedef Fwk::Ptr<CustomerLocation const> PtrConst;
        typedef Fwk::Ptr<CustomerLocation> Ptr;
        static CustomerLocation::Ptr CustomerLocationNew(Fwk::String _name ) {
            Ptr m = new CustomerLocation(_name);
            return m;
        }
    protected:
        CustomerLocation (const CustomerLocation& );
        CustomerLocation( Fwk::String _name) : Location(_name, Location::customer_) {}
    };
    class PortLocation : public Location {
    public:
        typedef Fwk::Ptr<PortLocation const> PtrConst;
        typedef Fwk::Ptr<PortLocation> Ptr;
        static PortLocation::Ptr PortLocationNew( Fwk::String _name ) {
            Ptr m = new PortLocation(_name);
            return m;
        }
    protected:
        PortLocation (const PortLocation& );
        PortLocation( Fwk::String _name) : Location(_name,port_) {}
    };

    // Terminal subclasses
    class TruckTerminal : public Location {
    public:
        typedef Fwk::Ptr<TruckTerminal const> PtrConst;
        typedef Fwk::Ptr<TruckTerminal> Ptr;
        static TruckTerminal::Ptr TruckTerminalNew( Fwk::String _name) {
            Ptr m = new TruckTerminal(_name);
            return m;
        }
    private:
        TruckTerminal (const TruckTerminal& );
        TruckTerminal( Fwk::String _name) : Location (_name,truckTerminal_){}
    };
    class BoatTerminal : public Location {
    public:
        typedef Fwk::Ptr<BoatTerminal const> PtrConst;
        typedef Fwk::Ptr<BoatTerminal> Ptr;
        static BoatTerminal::Ptr BoatTerminalNew( Fwk::String _name) {
            Ptr m = new BoatTerminal(_name);
            return m;
        }
    private:
        BoatTerminal (const BoatTerminal& );
        BoatTerminal( Fwk::String _name) : Location (_name,boatTerminal_){}
    };
    class PlaneTerminal : public Location {
    public:
        typedef Fwk::Ptr<PlaneTerminal const> PtrConst;
        typedef Fwk::Ptr<PlaneTerminal> Ptr;
        static PlaneTerminal::Ptr PlaneTerminalNew( Fwk::String _name) {
            Ptr m = new PlaneTerminal(_name);
            return m;
        }
    private:
        PlaneTerminal (const PlaneTerminal& );
        PlaneTerminal( Fwk::String _name) : Location(_name,planeTerminal_){}
    };

    // SEGMENT SUBCLASSES ==============================================
    class TruckSegment : public Segment {
    public:
        typedef Fwk::Ptr<TruckSegment const> PtrConst;
        typedef Fwk::Ptr<TruckSegment> Ptr;
        static TruckSegment::Ptr TruckSegmentNew( Fwk::String _name) {
            Ptr m = new TruckSegment(_name);
            return m;
        }
    protected:
        TruckSegment (const TruckSegment& );
        TruckSegment( Fwk::String _name) : Segment(_name, Shipping::Truck_) {}
    };
    class BoatSegment : public Segment {
    public:
        typedef Fwk::Ptr<BoatSegment const> PtrConst;
        typedef Fwk::Ptr<BoatSegment> Ptr;
        static BoatSegment::Ptr BoatSegmentNew( Fwk::String _name) {
            Ptr m = new BoatSegment(_name);
            return m;
        }
    protected:
        BoatSegment (const TruckSegment& );
        BoatSegment( Fwk::String _name) : Segment(_name, Shipping::Boat_) {}
    };
    class PlaneSegment : public Segment {
    public:
        typedef Fwk::Ptr<PlaneSegment const> PtrConst;
        typedef Fwk::Ptr<PlaneSegment> Ptr;
        static PlaneSegment::Ptr PlaneSegmentNew( Fwk::String _name) {
            Ptr m = new PlaneSegment(_name);
            return m;
        }
    protected:
        PlaneSegment (const TruckSegment& );
        PlaneSegment( Fwk::String _name) : Segment(_name, Shipping::Plane_) {}
    };

    class Fleet : public Fwk::NamedInterface {
    public:
        static const ShippingMode truck() {return Truck_;}
        static const ShippingMode boat() {return Boat_;}
        static const ShippingMode plane() {return Plane_;}

        typedef Fwk::Ptr<Fleet const> PtrConst;
        typedef Fwk::Ptr<Fleet> Ptr;
        static Fleet::Ptr FleetNew(Fwk::String _name) {
            Ptr m = new Fleet( _name );
            return m;
        }

        ~Fleet() {}
        Speed speed( ShippingMode m ) const { return fleetmode[m].speed_; }
        void speedIs (ShippingMode m, Speed _speed) { fleetmode[m].speed_ = _speed; }

        Capacity capacity( ShippingMode m ) const{ return fleetmode[m].capacity_; }
        void capacityIs (ShippingMode m, Capacity _capacity) { fleetmode[m].capacity_ = _capacity; }

        Cost cost( ShippingMode m ) const { return fleetmode[m].cost_; }
        void costIs (ShippingMode m, Cost _cost) { fleetmode[m].cost_ = _cost; }

    protected:
        struct FleetMode {
            Speed speed_;
            Capacity capacity_;
            Cost cost_;
            FleetMode():speed_(50),capacity_(10),cost_(1) {}
        };
        Fleet (const Fleet&);
        explicit Fleet( Fwk::String _name): Fwk::NamedInterface(_name){ }
        FleetMode fleetmode[MODE_COUNT];
    };

    /******** ASSIGNMENT 3 CODE - disregard for now
    class Shipment : public Fwk::NamedInterface {
    public:
    typedef Fwk::Ptr<Shipment const> PtrConst;
    typedef Fwk::Ptr<Shipment> Ptr;
    Shipment::Ptr ShipmentNew( Fwk::String name );

    Capacity packages() const { return packages_; }
    void packagesIs( unsigned int _packages ) {}
    Location::Ptr source();
    void sourceIs(Location::Ptr s)
    Location::Ptr destination();
    void destinationIs(Location::Ptr l)
    protected:
    Shipment( const Shipment&);
    explicit Shipment(Fwk::String _name) : Fwk::NamedInterface(_name), packages_(0){}
    Capacity packages_;
    Location* source;
    Location* destination;
    } */

} /* end namespace */
#endif
