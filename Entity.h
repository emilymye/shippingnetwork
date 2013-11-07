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
    };
    class Capacity : public Ordinal<Capacity, unsigned int> {
    public:
        Capacity(unsigned int num) : Ordinal<Capacity,unsigned int>(num) {};
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
        //HASHMAP
        Segment const * fwkHmNext() const { return fwkHmNext_.ptr(); }
        Segment * fwkHmNext() { return fwkHmNext_.ptr(); }
        void fwkHmNextIs(Segment * _fwkHmNext) const {fwkHmNext_ = _fwkHmNext;}
        Fwk::String fwkKey() const { return name(); }

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
        void expediteSupportIs( bool support ) { expediteSupport_ = support; }
    protected:
        Segment (const Segment&);
        Segment(Fwk::String _name, ShippingMode _mode) : 
            Fwk::NamedInterface(_name), mode_(_mode), length_(0.f),difficulty_(0.f) {}
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
            terminal_,
            other_
        };
        static inline LocationType customer() { return customer_; }
        static inline LocationType port() { return port_; }
        static inline LocationType terminal() { return terminal_; }
        static inline LocationType other() { return terminal_; }
        LocationType type() const { return type_; }
        void typeIs( LocationType _type ) { type_ = _type; }

        static Location::Ptr LocationNew(Fwk::String _name, LocationType _type){
            Ptr m = new Location(_name, _type);
            return m;
        }
        //HASHMAP
        Location const * fwkHmNext() const { return fwkHmNext_.ptr(); }
        Location * fwkHmNext() { return fwkHmNext_.ptr(); }
        void fwkHmNextIs(Location * _fwkHmNext) const {fwkHmNext_ = _fwkHmNext;}
        Fwk::String fwkKey() const { return name(); }

        ~Location();
        typedef std::vector<Segment::Ptr> SegmentList;
        Segment::PtrConst segment( int num ) const { return segment_[num]; }
        Segment::Ptr segment( int num ) { return segment_[num]; }
        void segmentIs( Fwk::String segmentName ); //TODO
    protected:
        Location ( const Location&);
        explicit Location(Fwk::String _name, LocationType _type) : Fwk::NamedInterface(_name), type_(_type) {}
        mutable Location::Ptr fwkHmNext_; 
        LocationType type_;
        SegmentList segment_;
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
        PortLocation( Fwk::String _name) : Location(_name, Location::port_) {}
    };
    class Terminal : public Location {
    public:
        typedef Fwk::Ptr<Terminal const> PtrConst;
        typedef Fwk::Ptr<Terminal> Ptr;
        static Terminal::Ptr TerminalNew( Fwk::String _name , ShippingMode _mode ) {
            Ptr m = new Terminal(_name, _mode);
            return m;
        }
        ShippingMode mode() { return mode_; }
    protected:
        Terminal (const Terminal& );
        Terminal( Fwk::String _name, ShippingMode _mode) : mode_(_mode), Location(_name, Location::port_) {}
        ShippingMode mode_;
    };
    // Terminal subclasses
    class TruckTerminal : public Terminal {
    public:
        typedef Fwk::Ptr<TruckTerminal const> PtrConst;
        typedef Fwk::Ptr<TruckTerminal> Ptr;
        static TruckTerminal::Ptr TruckTerminalNew( Fwk::String _name) {
            Ptr m = new TruckTerminal(_name);
            return m;
        }
    private:
        TruckTerminal (const TruckTerminal& );
        TruckTerminal( Fwk::String _name) : Terminal (_name,Shipping::Truck_){}
    };
    class BoatTerminal : public Terminal {
    public:
        typedef Fwk::Ptr<BoatTerminal const> PtrConst;
        typedef Fwk::Ptr<BoatTerminal> Ptr;
        static BoatTerminal::Ptr BoatTerminalNew( Fwk::String _name) {
            Ptr m = new BoatTerminal(_name);
            return m;
        }
    private:
        BoatTerminal (const BoatTerminal& );
        BoatTerminal( Fwk::String _name) : Terminal (_name,Shipping::Boat_){}
    };
    class PlaneTerminal : public Terminal {
    public:
        typedef Fwk::Ptr<PlaneTerminal const> PtrConst;
        typedef Fwk::Ptr<PlaneTerminal> Ptr;
        static PlaneTerminal::Ptr PlaneTerminalNew( Fwk::String _name) {
            Ptr m = new PlaneTerminal(_name);
            return m;
        }
    private:
        PlaneTerminal (const PlaneTerminal& );
        PlaneTerminal( Fwk::String _name) : Terminal(_name,Shipping::Plane_){}
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
    } 

    */

} /* end namespace */
#endif
