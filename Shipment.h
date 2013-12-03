#ifndef SHIPMENT_H
#define SHIPMENT_H

#include "Nominal.h"
#include "fwk/Exception.h"
#include "fwk/NamedInterface.h"
#include "fwk/String.h"
#include "fwk/Ptr.h"
#include "fwk/PtrInterface.h"
#include "Instance.h"
#include <vector>

namespace Shipping{
	class Time : public Ordinal<Time,double> {
	public:
		Time(double time) : Ordinal<Time,double>(time){}
	};

    class Shipment : public Fwk::NamedInterface {
    public:
    	typedef Fwk::Ptr<Shipment const> PtrConst;
    	typedef Fwk::Ptr<Shipment> Ptr;
    	Shipment::Ptr ShipmentNew( Fwk::String name );

		Capacity packages() const { return packages_; }
		void packagesIs( unsigned int _packages ) {}

		Location::Ptr source();
		void sourceIs(Location::Ptr s);

		Location::Ptr destination();
		void destinationIs(Location::Ptr l);
    protected:
		Shipment( const Shipment&);
		explicit Shipment(Fwk::String _name) :
				Fwk::NamedInterface(_name), packages_(0), source_(0), destination_(0){}
		Capacity packages_;
		Location* source_;
		Location* destination_;
    };
}
#endif


