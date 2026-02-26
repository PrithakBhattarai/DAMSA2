#pragma once
#include "G4VSensitiveDetector.hh"
#include "globals.hh"

class G4Step;

class PhaseSpaceSD : public G4VSensitiveDetector {
public:
  PhaseSpaceSD(const G4String& name);
  ~PhaseSpaceSD() override = default;

  G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override;
};
