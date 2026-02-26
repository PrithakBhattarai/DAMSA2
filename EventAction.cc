#include "EventAction.hh"
#include "RunAction.hh"
#include "G4Event.hh"
#include "G4SDManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4THitsMap.hh"

EventAction::EventAction(RunAction* runAction)
: G4UserEventAction(),
  fRunAction(runAction),
  fEdep(0.)
{}

EventAction::~EventAction() {}

void EventAction::ResolveCollectionIDs()
{
  if (fCID_EcalEdep >= 0) return;
  auto* sdm = G4SDManager::GetSDMpointer();
  fCID_EcalEdep = sdm->GetCollectionID("EcalMFD/Edep");
  fCID_TailEdep = sdm->GetCollectionID("TailMFD/Edep");
  fCID_TrkEdep  = sdm->GetCollectionID("TrackerMFD/Edep");
}

void EventAction::BeginOfEventAction(const G4Event* /*event*/)
{
  fEdep = 0.;
  if (fCID_EcalEdep < 0) ResolveCollectionIDs();
}

void EventAction::EndOfEventAction(const G4Event* evt)
{
  auto* HCE = evt->GetHCofThisEvent();
  const G4int evID = evt->GetEventID();

  G4double ecalSum = 0., tailE = 0., trkE = 0.;

  auto getMap = [&](G4int cid) -> G4THitsMap<G4double>* {
    return (HCE && cid >= 0) ? dynamic_cast<G4THitsMap<G4double>*>(HCE->GetHC(cid)) : nullptr;
  };

  // ECAL: write one CSV row per layer hit
  if (auto* m = getMap(fCID_EcalEdep)) {
    for (const auto& kv : *m->GetMap()) {
      const G4int    layerCopyNo = kv.first;
      const G4double edep        = *(kv.second);  // in MeV
      ecalSum += edep;
      fRunAction->WriteEcalHit(evID, layerCopyNo, edep);
    }
  }

  // Tail and Tracker sums
  if (auto* m = getMap(fCID_TailEdep))
    for (const auto& kv : *m->GetMap()) tailE += *(kv.second);

  if (auto* m = getMap(fCID_TrkEdep))
    for (const auto& kv : *m->GetMap()) trkE += *(kv.second);

  // Per-event summary CSV
  fRunAction->WriteEventSummary(evID, ecalSum, tailE, trkE);

  // keep your total accumulator
  fRunAction->AddEdep(fEdep + ecalSum + tailE + trkE);
}
