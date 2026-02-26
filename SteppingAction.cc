#include "SteppingAction.hh"
#include "EventAction.hh"      // <-- THIS IS IMPORTANT!
#include "G4Step.hh"
#include "G4Event.hh"
#include "G4Track.hh"
#include "G4SystemOfUnits.hh"

SteppingAction::SteppingAction(EventAction* eventAction)
 : G4UserSteppingAction(),
   fEventAction(eventAction)
{}

SteppingAction::~SteppingAction() {}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  G4double edepStep = step->GetTotalEnergyDeposit();

  if (edepStep > 0.) {
    fEventAction->AddEdep(edepStep);   // ✅ Now compiler knows AddEdep exists
  }
}
