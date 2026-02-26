#ifndef EVENTACTION_HH
#define EVENTACTION_HH

#include "G4UserEventAction.hh"
#include "globals.hh"

class G4Event;
class RunAction;

class EventAction : public G4UserEventAction
{
  public:
    EventAction(RunAction* runAction);
    virtual ~EventAction();

    virtual void BeginOfEventAction(const G4Event* event);
    virtual void EndOfEventAction(const G4Event* event);

    void AddEdep(G4double edep) { fEdep += edep; }

  private:
    void ResolveCollectionIDs(); // lazy once

    RunAction* fRunAction;   // pointer to RunAction
    G4double   fEdep;

    // IDs for primitive scorers from your DetectorConstruction:
    // "EcalMFD/Edep", "TailMFD/Edep", "TrackerMFD/Edep"
    G4int fCID_EcalEdep = -1;
    G4int fCID_TailEdep = -1;
    G4int fCID_TrkEdep  = -1;
};

#endif
