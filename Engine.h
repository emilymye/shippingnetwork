#ifndef ENGINE_H
#define ENGINE_H

#include <string>

#include "Ptr.h"
#include "PtrInterface.h"
#include "Instance.h"

namespace Shipping {

//Ordinal Classes: listed mostly for reference, all are at least non-negative
class Difficulty : public Ordinal<Difficulty,float> {}; //1.0 to 5.0
class Mile : public Ordinal<Mile, unsigned int> {}; 
class Capacity : public Ordinal<Capacity, unsigned int> {};
class Speed : public Ordinal<Speed, float> {};
class Cost : public Ordinal<Cost, float> {};
class Time : public Ordinal<Cost, float> {};

//INTERFACE - public methods. attributes only
class ShippingNetwork{
public:
    ShippingNetwork::Ptr ShippingNetworkNew();
    ~ShippingNetwork();

    // Notifiee implementation excerpt (for virtual methods)
    // Some basic implementation excluded
    class Notifiee { ... };
    class NotifieeConst : public virtual Fwk::NamedInterface::NotifieeConst {
    public:
        virtual void onLocationNew( Cell::Ptr ) {}
        virtual void onSegmentNew( Fwk::String _name ) {}
    ShippingNetwork::Notifiee * notifiee() const {return notifiee_;}

    //LOCATION queries
    Location::Ptr LocationIs( Fwk::String _name);
    Location::Ptr location( Fwk::String _name);
    
    //SEGMENT queries 		*****************************************
    Segment::Ptr SegmentIs( Fwk::String _name);
    Segment::Ptr segment( Fwk::String _name);
    
    //FLEET queries 		****************************************
    Fleet::Ptr FleetIs( Fwk::String _name);
    Fleet::Ptr fleet( Fwk::String _name);

    //CONNECTIVITY queries 	*****************************************
    Fwk::String exploration ( Fwk::String startLocation, Mile maxDist, Cost maxCost, float MaxTime, bool expedited);
    Fwk::String connection ( Fwk::String startLocation, Fwk::String endLocation );

    //Typedefs for different Hashmaps
    typedef Fwk::Hashmap < Location, Fwk::String, Location, Location::PtrConst, Location::Ptr> LocationDict;
    typedef Fwk::Hashmap < Segment, Fwk::String, Segment, Segment::PtrConst, Segment::Ptr> SegmentDict;
    typedef Fwk::Hashmap < Fleet, Fwk::String, Fleet, Fleet::PtrConst, Fleet::Ptr> FleetDict;
protected:
    ShippingNetwork(){}
    ShippingNetwork::Notifiee * notifiee_;
    void notifieeIs( ShippingNetwork::Notifiee *  n) const {
        ShippingNetwork* me = const_cast<ShippingNetwork*>(this);
        me->notifiee_ = n;
    }

    LocationDict locations_;
    SegmentDict segments_;
    FleetDict fleets_
};

class ShippingNetworkReactor : public ShippingNetwork::Notifiee{
public:
    static ShippingNetworkReactor::Ptr ShippingNetworkReactorIs( ShippingNetwork* sn) {}
    void onLocationNew(Location::Ptr loc);
    void onSegmentNew(Segment::Ptr seg);

    enum EntityType{
        customer_,
        port_,
        truckTerminal_,
        boatTerminal_,
        planeTerminal_,
        truckSegment_,
        boatSegment_,
        planeSegment_,
        SHIPPING_ENTITY_COUNT
    }

    //STATS QUERIES 		*****************************************
    int shippingEntities( ShippingEntity type );
    float expeditedPercent();
    
protected:
    ShippingNetworkReactor(ShippingNetwork * sn) : ShippingNetwork::Notifiee() { 
    	notifierIs(t); 
    }
    int expeditedSegments;
    int entityCounts [SHIPPING_ENTITY_COUNT]
};

class Location : Fwk::NamedInterface {
public:
    LocationNew( Fwk::String name ) {}

    Segment::Ptr segment( int segmentN );
    void segmentIs( Fwk::String segmentName );
    
    enum LocationType{
        customer,
        port,
        truckTerminal,
        boatTerminal,
        planeTerminal
    };
    LocationType type();
    typeIs( LocationType type );
    typedef Fwk::LinkedList<Segment::Ptr> SegmentList;
private:
    LocationType type_;
    SegmentList segments_;
};

class Segment : public Fwk::NamedInterface {
public: 
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

enum Mode {
    truck,
    boat,
    plane,
    MODE_COUNT //DO NOT MOVE
};

class Fleet : public Fwk::NamedInterface {
public:
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

//Possible unnecessary for this assignment
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


} /* end namespace */

#endif
