#ifndef MYTRACKINGACTION_HH
#define MYTRACKINGACTION_HH

#include "G4UserTrackingAction.hh"

class MyTrackingAction : public G4UserTrackingAction {
public:
    MyTrackingAction() = default;
    ~MyTrackingAction() override = default;

    void PreUserTrackingAction(const G4Track* track) override;
};

#endif
