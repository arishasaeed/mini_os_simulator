#ifndef OBSERVER_H
#define OBSERVER_H

namespace CustomDS {

// Interface for Observer in the Observer Pattern
class Observer {
public:
    virtual ~Observer() = default;
    
    // Called by the subject when its state changes
    virtual void update() = 0;
};

} // namespace CustomDS

#endif // OBSERVER_H
