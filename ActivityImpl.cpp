#include <iostream>
#include <time.h>
//#include "unistd.h"
#include "ActivityImpl.h"

namespace ActivityImpl {

    // == | ManagerImpl |========================================

    // Definition of static member
	Fwk::Ptr<Activity::Manager> ManagerImpl::managerInstance_ = NULL;
    //Gets the singleton instance of ManagerImpl
	Fwk::Ptr<Activity::Manager> ManagerImpl::activityManagerInstance() {
		if (managerInstance_ == NULL) {
			managerInstance_ = new ManagerImpl();
		}
		return managerInstance_;
	}

	Activity::Ptr ManagerImpl::activityNew(const string& name) {
		Activity::Ptr activity = activities_[name];
		if (activity != NULL) {
			cerr << "Activity already exists!" << endl;
			return NULL;
		}

		activity = new ActivityImpl(name, this);
		activities_[name] = activity;
		return activity;
	}

	Activity::Ptr ManagerImpl::activity(const string& name) const {
		map<string, Activity::Ptr>::const_iterator it = activities_.find(name);
		if(it != activities_.end() ) {
			return (*it).second;
		} // dont throw an exception (accessor)
		return NULL; 
	}

	void ManagerImpl::activityDel(const string& name) {
		activities_.erase(name);
	}

	void ManagerImpl::lastActivityIs(Activity::Ptr activity) {
		scheduledActivities_.push(activity);
	}

	void ManagerImpl::nowIs(Time t) {
		while (!scheduledActivities_.empty()) {
			Activity::Ptr nextAct = scheduledActivities_.top();
            if (nextAct->nextTime() > t) break;
	    
			now_ = nextAct->nextTime();
			scheduledActivities_.pop();
            nextAct->statusIs(Activity::executing);
            nextAct->statusIs(Activity::free);
		}
	    //syncrhonize the time
		now_ = t;
	}

    // == | RealTimeManagerImpl |========================================
	Fwk::Ptr<RealTimeManager> RealTimeManager::managerInstance_ = NULL;

    //Gets the singleton instance of RealTimeManagerImpl
	Fwk::Ptr<RealTimeManager> RealTimeManager::realTimeManagerInstance() {
		if (managerInstance_ == NULL) {
			managerInstance_ = new RealTimeManager();
		}
		return managerInstance_;
	}

	void RealTimeManager::realTimePassedIs(Time t) {
        //usleep(t * 1000000);
		virtualManager_->nowIs(now().value + t.value());
	}

} //end namespace ActivityImpl
