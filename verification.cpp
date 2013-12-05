#include <iostream>
#include <vector>
#include "Exception.h"
#include "Instance.h"
#include "ActivityImpl.h"

using namespace std;
using Fwk::Exception;

int main(int argc, char *argv[]) {
  try
  {
    Ptr<Instance::Manager> manager = shippingInstanceManager();
	vector< Ptr<Instance> > loc;
	vector< Ptr<Instance> > seg;

	// Locations
    loc.push_back( manager->instanceNew("customer1", "Customer") );
    loc.push_back( manager->instanceNew("customer2", "Customer") );
    loc.push_back( manager->instanceNew("port1", "Port") );
    loc.push_back( manager->instanceNew("tt1", "Truck terminal") );
    loc.push_back( manager->instanceNew("tt2", "Truck terminal") );
    loc.push_back( manager->instanceNew("customer3", "Customer") );
    loc[0]->attributeIs("transfer rate", "2");
	// Segments
    seg.push_back( manager->instanceNew("ps1", "Plane segment") );
    seg.push_back( manager->instanceNew("ps2", "Plane segment") );
    seg.push_back( manager->instanceNew("bs1", "Boat segment") );
    seg.push_back( manager->instanceNew("bs2", "Boat segment") );
    seg.push_back( manager->instanceNew("ts1", "Truck segment") );
    seg.push_back( manager->instanceNew("ts2", "Truck segment") );
    seg.push_back( manager->instanceNew("ts3", "Truck segment") );
    seg.push_back( manager->instanceNew("ts4", "Truck segment") );
    seg.push_back( manager->instanceNew("ts5", "Truck segment") );
    seg.push_back( manager->instanceNew("ts6", "Truck segment") );
    seg.push_back( manager->instanceNew("ts7", "Truck segment") );
    seg.push_back( manager->instanceNew("ts8", "Truck segment") );

	
	seg[0]->attributeIs("source", "customer1");
	seg[1]->attributeIs("source", "port1");
	seg[1]->attributeIs("return segment", "ps1");

	seg[2]->attributeIs("source", "customer2");
	seg[2]->attributeIs("source", "port1");
	seg[3]->attributeIs("return segment", "ps2");


    Ptr<Instance> fleet = manager->instanceNew("Fleet", "Fleet");
    fleet->attributeIs("Truck, speed", "1");
    fleet->attributeIs("Truck, capacity", "1");

	// Set some segment (shipment) capacities
	seg[4]->attributeIs("capacity", "1");
	seg[6]->attributeIs("capacity", "1");

	Ptr<Instance> conn = manager->instanceNew("myConn", "Conn");
	
    Activity::Manager::Ptr activityManager = activityManagerInstance();
    activityManager->nowIs(6.0);

//  RealTimeManager::Ptr realTimeManager = realTimeManagerInstance();
//  realTimeManager->realTimePassedIs(6.0);

	// Stop injection activity from the first two customers
    // loc[0]->attributeIs("Transfer Rate", "0");
    // loc[1]->attributeIs("Transfer Rate", "0");

    activityManager->nowIs(24.0);
//  realTimeManager->realTimePassedIs(18.0);

	/* Print simulation statistics for analysis 
	 * You'll need some numbers to put into your README and for analysis. 
	 * The format is completely up to you, but make sure it is easily 
	 * readable, otherwise we might misunderstand your analysis
	 */
	//printSimStats();
  }
  catch(Exception e)
  {
	cout << e.what() << endl;
  }
  catch(...)
  {
  }

  return 0;
}

