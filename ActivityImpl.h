#ifndef __ACTIVITY_IMPL_H__
#define __ACTIVITY_IMPL_H__

#include <map>
#include <string>
#include <queue>
#include <string>
#include "PtrInterface.h"
#include "Ptr.h"
#include "Activity.h"

Fwk::Ptr<Activity::Manager> activityManagerInstance();

namespace ActivityImpl {
    class ActivityComp : public binary_function<Activity::Ptr, Activity::Ptr, bool> {
    public:
        ActivityComp() {}

        bool operator()(Activity::Ptr a, Activity::Ptr b) const {
            return (a->nextTime() > b->nextTime());
        }
    };

    class ActivityImpl : public Activity {
    protected:
        ActivityImpl(const string& name, Fwk::Ptr<class ManagerImpl> manager)
            : Activity(name), status_(free), nextTime_(0.0), notifiee_(NULL), manager_(manager) {}
        Fwk::Ptr<class ManagerImpl> manager() const { return manager_; }

        virtual Status status() const { return status_; }
        virtual void statusIs(Status s) {
            status_ = s;
            if (notifiee_ != NULL) {
                notifiee_->onStatus();
            }
        }

        virtual Time nextTime() const { return nextTime_; }
        virtual void nextTimeIs(Time t) {
            nextTime_ = t;
            if (notifiee_ != NULL) {
                notifiee_->onNextTime();
            }
        }

        virtual Notifiee::Ptr notifiee() const { return notifiee_; }

        virtual void lastNotifieeIs(Notifiee* n) {
            ActivityImpl* me = const_cast<ActivityImpl*>(this);
            me->notifiee_ = n;
        }
    private:
        friend class ManagerImpl;
        Status status_;
        Time nextTime_;
        Notifiee* notifiee_;
        Fwk::Ptr<class ManagerImpl> manager_;
    };

    class ManagerImpl : public Activity::Manager {
    public:
        typedef Fwk::Ptr<ManagerImpl> Ptr;
        virtual Activity::Ptr activityNew(const string& name);
        virtual Activity::Ptr activity(const string& name) const;
        virtual void activityDel(const string& name);
        virtual Time now() const { return now_; }
        virtual void nowIs(Time time);

        static Fwk::Ptr<Activity::Manager> activityManagerInstance();
        virtual void lastActivityIs(Activity::Ptr activity);
    protected:
        ManagerImpl() : now_(0) { }
        Time now_;
        priority_queue<Activity::Ptr, vector<Activity::Ptr>, ActivityComp> scheduledActivities_;
        map<string, Activity::Ptr> activities_; //pool of all activities
        //singleton instance
        static Fwk::Ptr<Activity::Manager> managerInstance_;	
    };
}

class RealTimeManager : public Fwk::PtrInterface<RealTimeManager> {
public:
    typedef Fwk::Ptr<RealTimeManager> Ptr;
    virtual Activity::Ptr activityNew(const string& name) { 
        return virtualManager_->activityNew(name);
    }
    virtual Activity::Ptr activity(const string& name) const{ 
        return virtualManager_->activity(name);
    }
    virtual void activityDel(const string& name){ 
        virtualManager_->activityDel(name);
    }
    virtual void lastActivityIs(Activity::Ptr activity) {
        virtualManager_->lastActivityIs(activity);
    }
    static Fwk::Ptr<RealTimeManager> realTimeManagerInstance();
    virtual void realTimePassedIs(Time t);
protected:
    RealTimeManager() { virtualManager_ = activityManagerInstance(); }
    Fwk::Ptr<Activity::Manager> virtualManager_;
    static Fwk::Ptr<RealTimeManager> managerInstance_;
};


extern Fwk::Ptr<RealTimeManager> realTimeManagerInstance();

#endif /* __ACTIVITY_IMPL_H__ */

