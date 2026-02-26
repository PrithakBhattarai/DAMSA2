// PrimaryGeneratorAction.hh
#ifndef PrimaryGeneratorAction_hh
#define PrimaryGeneratorAction_hh

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
  PrimaryGeneratorAction();
  ~PrimaryGeneratorAction() override;
  void GeneratePrimaries(G4Event* event) override;

  // expose the gun so RunAction (or others) can inspect it
  G4ParticleGun* GetParticleGun() const { return fGun; }

private:
  G4ParticleGun* fGun;
};

#endif // PrimaryGeneratorAction_hh
