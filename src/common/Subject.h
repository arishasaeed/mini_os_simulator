#ifndef SUBJECT_H
#define SUBJECT_H

#include <vector>
#include "Observer.h"

namespace CustomDS {

// Base class for Subject in the Observer Pattern
class Subject {
private:
    std::vector<Observer*> observers;

public:
    virtual ~Subject() = default;

    // Attach an observer
    void addObserver(Observer* observer) {
        if (observer) {
            observers.push_back(observer);
        }
    }

    // Detach an observer
    void removeObserver(Observer* observer) {
        for (auto it = observers.begin(); it != observers.end(); ++it) {
            if (*it == observer) {
                observers.erase(it);
                break;
            }
        }
    }

    // Notify all attached observers of a state change
    void notifyObservers() {
        for (auto* observer : observers) {
            if (observer) {
                observer->update();
            }
        }
    }
};

} // namespace CustomDS

#endif // SUBJECT_H
