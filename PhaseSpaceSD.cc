#include "PhaseSpaceSD.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4RunManager.hh"
#include "RunAction.hh"
#include "G4SystemOfUnits.hh"
#include <algorithm>
#include <cmath>

PhaseSpaceSD::PhaseSpaceSD(const G4String& name)
: G4VSensitiveDetector(name) {}

G4bool PhaseSpaceSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
  // Log only when entering the thin plane (1 row per crossing)
  if (step->GetPreStepPoint()->GetStepStatus() != fGeomBoundary) return false;

  const auto* runAction =
      dynamic_cast< const RunAction*>(G4RunManager::GetRunManager()->GetUserRunAction());
  if (!runAction) return false;

  const auto* track = step->GetTrack();
  const auto  p     = track->GetMomentum();             // MeV
  const auto  pdir  = track->GetMomentumDirection();
  const auto* def   = track->GetDefinition();

  // Beam assumed along +Z (change here if different)
  const G4ThreeVector beamDir(0.,0.,1.);
  const G4double cosTh = std::clamp(pdir.dot(beamDir), -1.0, 1.0);
  const G4double theta = std::acos(cosTh);
  const G4double phi   = std::atan2(pdir.y(), pdir.x());

  const auto* pre  = step->GetPreStepPoint();
  const auto  pos  = pre->GetPosition();                // mm
  const G4double t = pre->GetGlobalTime();              // ns

  const G4int    evID   = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
  const G4int    trkID  = track->GetTrackID();
  const G4int    parent = track->GetParentID();
  const G4int    pdg    = def->GetPDGEncoding();
  const G4double q_e    = def->GetPDGCharge();          // in e units
  const G4double Ek     = track->GetKineticEnergy() / MeV;
  const G4double Etot   = track->GetTotalEnergy()  / MeV;

  runAction->WritePhaseRow(
      evID, trkID, parent, pdg, q_e,
      Ek, Etot,
      p.x(), p.y(), p.z(),    // MeV
      theta, phi,
      pos.x(), pos.y(), pos.z(),  // mm
      t);                          // ns

  return true;
}
