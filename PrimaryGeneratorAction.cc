// PrimaryGeneratorAction.cc
#include "PrimaryGeneratorAction.hh"

#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4Event.hh"
#include "G4ThreeVector.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
  fGun = new G4ParticleGun(1);
  auto* particle = G4ParticleTable::GetParticleTable()->FindParticle("e-");
  fGun->SetParticleDefinition(particle);
  fGun->SetParticleEnergy(3.*GeV);               // tweak via macro if desired
  fGun->SetParticleMomentumDirection(G4ThreeVector(0.,0.,1.));
  fGun->SetParticlePosition(G4ThreeVector(0.,0.,-4.95*m));
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
  fGun->GeneratePrimaryVertex(event);
}
