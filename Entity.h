#ifndef ENTITY_H
#define ENTITY_H
#include <vector>
#include <queue>

#include "Exception.h"
#include "String.h"
#include "Ptr.h"
#include "PtrInterface.h"
#include "Nominal.h"
#include "Notifiee.h"
#include "Activity.h"

namespace Shipping{
    class Capacity : public Ordinal<Capacity, int> {
    public:
        Capacity(int num) : Ordinal<Capacity, int>(num) {
            if (num < 0) {
                cerr << "Invalid Capacity value: " << num << endl;
                throw Fwk::RangeException("Capacity");
            }
        };
    };
    class Difficulty : public Ordinal<Difficulty,float> {
    public:
        Difficulty(float num = 1.0) : Ordinal<Difficulty, float>(num){
            if ( num < 1.0 || num > 5.0 ){
                cerr << "Invalid Difficulty value: " << num << endl;
                throw Fwk::RangeException("Difficulty");
            }
        }
    };
    class Mile : public Ordinal<Mile, float> {
    public:
        Mile(float num) : Ordinal<Mile,float>(num) {
            if (num < 0.f) {
                cerr << "Invalid Mile value: " << num << endl;
                throw Fwk::RangeException("Mile");
            }
        };
    };
    class Speed : public Ordinal<Speed, float> {
    public:
        Speed(float num) : Ordinal<Speed, float>(num){
            if (num < 0.f ) {
                cerr << "Invalid Speed value: " << num << endl;
                throw Fwk::RangeException("Speed");
            }
        }
    };
    class Cost : public Ordinal<Cost, float> {
    public:
        Cost(float num) : Ordinal<Cost, float>(num){
            if (num < 0.f ) {
                cerr << "Invalid Cost value: " << num << endl;
                throw Fwk::RangeException("Cost");
            }
        }
        Cost expedited() { return (value() * 1.5); }

    };
    using namespace std;

    enum ShippingMode { Truck_, Boat_, Plane_, 
        MODE_COUNT //DO NOT MOVE 
    };

    class Location;

    struct Shipment {
        Location* src;
        Location* dest;
        Capacity packages;
        Time totalTime;
        Cost totalCost;
        Shipment( Location* src, Location* _dest, Capacity _packages)
            : dest(_dest), packages(_packages), totalTime(0.0), totalCost(0.0) {}
    };

    class Segment : public Fwk::PtrInterface<Segment> {
    public:
        typedef Fwk::Ptr<Segment> Ptr;
        static Segment::Ptr SegmentNew(Fwk::String _name, ShippingMode _mode){
            Ptr m = new Segment(_name, _mode);
            return m;
        }
        ~Segment() { }

        ShippingMode mode() const { return mode_; };
        Location * source() const { return source_; }
        void sourceIs( Location* _loc ) { source_ = _loc; }
        Segment::Ptr returnSegment() const { return returnSegment_; }
        void returnSegmentIs( Segment* rs ) { returnSegment_ = rs; }

        Mile length() const { return length_; }
        void lengthIs( Mile newLength ) { length_ = newLength; }

        Difficulty difficulty() const { return difficulty_; }
        void difficultyIs(Difficulty d)  { difficulty_ = d; }

        bool expediteSupport() const { return expediteSupport_; }
        void expediteSupportIs( bool support ) {
            expediteSupport_ = support;
        }

        // SHIPMENT SUPPORT
        Capacity receivedShipments() const { return recievedShip_; }
        Capacity refusedShipments() const { return refusedShip_; }
        Capacity shipmentCapacity() const { return shipmentCap_; }
        void shipmentCapacityIs( Capacity c) { shipmentCap_ = c; }

        bool shipmentNew( Shipment * s){
            if (shipmentQ_.size() >= shipmentCap_.value()){
                ++refusedShip_;
                return false;
            }
            ++recievedShip_;
            if (notifiee())
                notifiee()->onShipmentRecieved(s);
        }
        virtual string name() const { return name_; }

        class Notifiee : public Fwk::BaseNotifiee<Segment> {
        public:
            typedef Fwk::Ptr<Notifiee> Ptr;
            Notifiee(Segment* s) : Fwk::BaseNotifiee<Segment>(s) {}
            virtual void onShipmentRecieved(Shipment*) {}
        };

        virtual Segment::Notifiee::Ptr notifiee() const { return notifiee_; }
        virtual void lastNotifieeIs(Notifiee* n) {
            Segment* me = const_cast<Segment*>(this);
            me->notifiee_ = n;
        }
    protected:
        Segment (const Segment&);
        Segment(Fwk::String _name, ShippingMode _mode) : name_(_name), mode_(_mode), 
            notifiee_(NULL), source_(NULL), returnSegment_(NULL),
            length_(0.f),difficulty_(1.f), expediteSupport_(false),
            shipmentCap_(10), recievedShip_(0), refusedShip_(0) {}
        Fwk::String name_;

        ShippingMode mode_;
        Segment::Notifiee::Ptr notifiee_;
        Location * source_;
        Segment* returnSegment_;

        Mile length_;
        Difficulty difficulty_;
        bool expediteSupport_;

        Capacity shipmentCap_;
        Capacity recievedShip_;
        Capacity refusedShip_;
        queue<Shipment*> shipmentQ_;
    };

    // START LOCATION CLASSES ==========================================
    class Location : public Fwk::PtrInterface<Location> {
    public:
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
        Segment::Ptr segment( unsigned int num ) const {
            if ( !num || num > segment_.size()) return NULL;
            return segment_[num - 1];
        }
        unsigned int segments() const { return segment_.size(); }
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

        class Notifiee : public Fwk::BaseNotifiee<Location> {
        public:
            typedef Fwk::Ptr<Notifiee> Ptr;
            Notifiee(Location* s) : Fwk::BaseNotifiee<Location>(s) {}
            virtual void onShipmentRecieved(Shipment* ) {}
        };
        virtual Location::Notifiee::Ptr notifiee() const { return notifiee_; }
        virtual void lastNotifieeIs(Notifiee* n) {
            Location* me = const_cast<Location*>(this);
            me->notifiee_ = n;
        }

        virtual string name() const { return name_; }
        virtual void shipmentNew(Shipment * s){
            if (notifiee_) notifiee_->onShipmentRecieved(s);
        }
    protected:
        Location ( const Location&);
        explicit Location(Fwk::String name, LocationType _type) : name_(name), type_(_type) {}
        mutable Location::Ptr fwkHmNext_;
        Location::Notifiee * notifiee_;

        LocationType type_;
        std::vector<Segment::Ptr> segment_;
        Fwk::String name_;
    };

    // LOCATION SUBCLASSES =============================================
    class CustomerLocation : public Location {
    public:
        typedef Fwk::Ptr<CustomerLocation const> PtrConst;
        typedef Fwk::Ptr<CustomerLocation> Ptr;
        static CustomerLocation::Ptr CustomerLocationNew(Fwk::String _name ) {
            Ptr m = new CustomerLocation(_name);
            return m;
        }

        class Notifiee : public Location::Notifiee{
        public:
            typedef Fwk::Ptr<Notifiee> Ptr;
            Notifiee(CustomerLocation* l) : Location::Notifiee(l) {}
            virtual void onTransferRate() {}
            virtual void onSize() {}
            virtual void onDestination() {}
        };

        virtual void lastNotifieeIs(CustomerLocation::Notifiee* n) {
            CustomerLocation* me = const_cast<CustomerLocation*>(this);
            me->notifiee_ = n;
        }

        Capacity transferRate() const { return rate_; }
        void transferRateIs( Capacity rate ) {
            if (notifiee_) notifiee_->onTransferRate();
            rate_ = rate;
        }

        Capacity shipmentSize() const { return size_; }
        void shipmentSizeIs( Capacity size ) {
            if (size_ == size) return;
            if (notifiee_) notifiee_->onSize();
            size_ = size;
        }

        Location* destination() const { return dest_; }
        void destinationIs( Location * dest ) {
            if (dest == NULL && dest_ == NULL) return;
            if (dest->name().compare(dest_->name()) == 0) return;
            if (notifiee_) notifiee_->onDestination();
            dest_ = dest;
        }

        Capacity recieved() const { return recieved_; }
        Time latency() const { return totalTime_.value()/recieved_.value(); }
        Cost totalCost() const { return totalCost_; }
        
        void shipmentNew(Shipment * s){
            if (!s->dest->name().compare(name_)) {
                ++recieved_;
                totalTime_ = totalTime_.value() + (s->totalTime).value();
                totalCost_ = totalCost_.value() + (s->totalCost).value();
            } else {
                if (notifiee_) notifiee_->onShipmentRecieved(s);
            }
        }
    protected:
        CustomerLocation::Notifiee::Ptr notifiee_;
        Capacity rate_;
        Capacity size_;
        Location * dest_;

        Capacity recieved_;
        Time totalTime_;
        Cost totalCost_; 

        CustomerLocation (const CustomerLocation& );
        CustomerLocation( Fwk::String _name) : Location(_name, Location::customer_), 
            rate_(0), size_(0), dest_(NULL),
            recieved_(0), totalTime_(0.0), totalCost_(0.f){}
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

    class Fleet : public Fwk::PtrInterface<Fleet> {
    public:
        static const ShippingMode truck() {return Truck_;}
        static const ShippingMode boat() {return Boat_;}
        static const ShippingMode plane() {return Plane_;}

        ~Fleet() {}
        Speed speed( ShippingMode m ) const { return fleetmode[m].speed_; }
        void speedIs (ShippingMode m, Speed _speed) { fleetmode[m].speed_ = _speed; }

        Capacity capacity( ShippingMode m ) const{ return fleetmode[m].capacity_; }
        void capacityIs (ShippingMode m, Capacity _capacity) { fleetmode[m].capacity_ = _capacity; }

        Cost cost( ShippingMode m ) const { return fleetmode[m].cost_; }
        void costIs (ShippingMode m, Cost _cost) { fleetmode[m].cost_ = _cost; }

        virtual string name() const { return name_; }
        typedef Fwk::Ptr<Fleet const> PtrConst;
        typedef Fwk::Ptr<Fleet> Ptr;
        static Fleet::Ptr FleetNew(Fwk::String _name) {
            Ptr m = new Fleet( _name );
            return m;
        }
    protected:
        Fleet (const Fleet&);
        explicit Fleet( Fwk::String name): name_(name){ }

        struct FleetMode {
            Speed speed_;
            Capacity capacity_;
            Cost cost_;
            FleetMode():speed_(50.f),capacity_(10),cost_(1.f) {}
        };
        FleetMode fleetmode[MODE_COUNT];
        Fwk::String name_;
    };

} /* end namespace */
#endif
