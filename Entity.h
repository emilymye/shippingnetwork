#ifndef ENTITY_H
#define ENTITY_H

#include "fwk/Exception.h"
#include <string>
#include "fwk/BaseNotifiee.h"
#include "fwk/NamedInterface.h"
#include "fwk/HashMap.h"
#include "fwk/LinkedList.h"
#include "fwk/String.h"
#include "fwk/Ptr.h"
#include "fwk/PtrInterface.h"
#include "Instance.h"

namespace Shipping{
    /*ORDINAL VALUE TYPES*/
    class Capacity : public Ordinal<Capacity, unsigned int> {
    public:
        Capacity(unsigned int num) : Ordinal<Capacity,unsigned int>(num) {};
    };
    class Difficulty : public Ordinal<Difficulty,float> {
    public:
        Difficulty(float num) : Ordinal<Difficulty, float>(num){
            if (num < min_ || num > max_ ){
                throw Fwk::RangeException("Difficulty");
            }
        }
        Difficulty min() const { return min_; };
        Difficulty max() const { return max_; }
    protected:
        const float min_ = 1.0;
        const float max_ = 5.0;
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
    class Time : public Ordinal<Cost, float> {
    public:
        Time(float num) : Ordinal<Time, float>(num){
            if (num < 0 ) throw Fwk::RangeException("Time");
        }
    };

    class ShippingMode {
        enum Mode {
            truck,
            boat,
            plane,
            MODE_COUNT //DO NOT MOVE
        };
        static inline Mode truck() { return truck; }
        static inline Mode boat() { return boat; }
        static inline Mode plane() { return plane; }
        static inline unsigned int types() { return MODE_COUNT; }
    };

    class Location : Fwk::NamedInterface {
    public:
        typedef Fwk::Ptr<Location const> PtrConst;
        typedef Fwk::Ptr<Location> Ptr;
        Fwk::String fwkKey() const { return name(); }

        enum LocationType{
            customer_,
            port_,
            terminal_,
        };
        LocationType type();
        void typeIs( LocationType type );
        static inline LocationType customer() { return customer_; };
        static inline LocationType port() { return port_; };
        static inline LocationType terminal() { return terminal_; };

        Segment::Ptr segment( int segmentN );
        void segmentIs( Fwk::String segmentName );
        typedef Fwk::LinkedList<Segment::Ptr> SegmentList;
    private:
        LocationType type_;
        SegmentList segments_;
    };

    class CustomerLocation : public Location {
    public:
        typedef Fwk::Ptr<CustomerLocation const> PtrConst;
        typedef Fwk::Ptr<CustomerLocation> Ptr;
    };

    class PortLocation : public Location{
    public:
    };

    class TerminalLocation : public Location {
    public:
        enum Mode {
            truck_,
            boat_,
            plane_
        };
        Mode mode() { return mode_; };
        void modeIs( Mode m) { mode_ = m; }
    private:
        Mode mode_;
    };



    class Segment : public Fwk::NamedInterface {
    public: 
        typedef Fwk::Ptr<Segment const> PtrConst;
        typedef Fwk::Ptr<Segment> Ptr;
        Segment::Ptr SegmentNew( Fwk::String _name, Mode_mode);

        Location::Ptr source();
        void sourceIs( Location::Ptr source ); 

        Mile length() { return length_; };
        void lengthIs( Mile newLength ) { length_ = newLength; };

        Segment::Ptr returnSegment();
        void returnSegmentIs( string & segmentName );

        Difficulty difficulty();

        bool expediteSupport();
        void expediteSupportIs( bool support );
    protected:
        Segment( Fwk::String _name, Mode _mode ) : Fwk::NamedInterface(_name), length_(1), mode_(_mode){}
    private:
        Mode mode_;
        Location * source;
        Mile length_;
        Segment* returnSegment_;
        Difficulty difficulty_;
        bool expediteSupport_;
    };

    class Fleet : public Fwk::NamedInterface {
    public:
        typedef Fwk::Ptr<Fleet const> PtrConst;
        typedef Fwk::Ptr<Fleet> Ptr;
        Fleet::Ptr FleetNew(Fwk::String name);

        Speed speed( Mode m );
        void speedIs (Mode m, Speed s);
        Capacity capacity( Mode m );
        void capacityIs (Mode m, Speed s);
        Cost cost( Mode m );
        void costIs (Mode m, Speed s);  
    private:
        struct FleetMode {
            Speed speeds_;
            Capacity capacity_;
            Cost cost_;
        };
        FleetMode modes [MODE_COUNT];
    };

    //For assignment 3 - disregard for now
    class Shipment : public Fwk::NamedInterface {
    public:
        Shipment::Ptr ShipmentNew( Fwk::String name );

        Capacity packages();
        void packagesIs( unsigned int _packages );
        Location::Ptr source();
        void sourceIs(Location::Ptr s)
            Location::Ptr destination();
        void destinationIs(Location::Ptr l)
    private:
        Capacity packages_;
        Location* source;
        Location* destination;

    }

#endif