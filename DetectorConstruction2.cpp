#include "DAMSADetectorConstruction.hpp"
#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "G4SDManager.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4VPrimitiveScorer.hh"
#include "G4PSEnergyDeposit.hh"
#include "G4PSTrackLength.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4SystemOfUnits.hh"
#include "G4UniformMagField.hh"
#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"
#include "G4SubtractionSolid.hh"
#include "DAMSASensitiveDetector.hpp"

DAMSADetectorConstruction::DAMSADetectorConstruction()
: G4VUserDetectorConstruction(),
  fTungstenTargetLV(nullptr),
  fDecayChamberLV(nullptr),
  fCsICalLV(nullptr),
  fScoringVolumeLV(nullptr),
  fTungsten(nullptr),
  fVacuum(nullptr),
  fSilicon(nullptr),
  fCsI(nullptr),
  fAir(nullptr),
  fMagField(nullptr),
  fFieldMgr(nullptr),
  fCsIPositionZ(-6*cm),
  fTargetX(5*cm),
  fTargetY(5*cm),
  fTargetZ(10*cm)
{
    for (int i = 0; i < 6; i++) {
        fSiTrackerLV[i] = nullptr;
    }
    DefineMaterials();
}

DAMSADetectorConstruction::~DAMSADetectorConstruction()
{
    delete fMagField;
}

void DAMSADetectorConstruction::DefineMaterials()
{
    G4NistManager* nist = G4NistManager::Instance();
    fTungsten = nist->FindOrBuildMaterial("G4_W");
    fVacuum   = nist->FindOrBuildMaterial("G4_Galactic");
    fSilicon  = nist->FindOrBuildMaterial("G4_Si");
    fCsI      = nist->FindOrBuildMaterial("G4_CESIUM_IODIDE");
    fAir      = nist->FindOrBuildMaterial("G4_AIR");
}

G4VPhysicalVolume* DAMSADetectorConstruction::Construct()
{
    // World
    G4Box* solidWorld = new G4Box("World", 1.0*m, 1.0*m, 2.0*m);
    G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, fAir, "World");
    G4VPhysicalVolume* physWorld =
        new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, 0);

    // Geometry positions
    const G4double targetCenterZ       = -80.0*cm;
    // VDC: front=-75cm (target exit), back=-40cm (flush against first Si layer)
    // center=(-75+-40)/2=-57.5cm, half-length=17.5cm
    const G4double decayChamberCenterZ = -57.5*cm;
    const G4double decayChamberHalfZ   =  17.5*cm;
    const G4double targetExitScoringZ  = -75.0*cm;

    // ---- Tungsten target (solid block, no hole) ----
    G4Box* targetSolid = new G4Box("Target", 2.5*cm, 2.5*cm, 5.0*cm);
    fTungstenTargetLV = new G4LogicalVolume(targetSolid, fTungsten, "TungstenTarget");
    new G4PVPlacement(0, G4ThreeVector(0,0,targetCenterZ),
                      fTungstenTargetLV, "TungstenTarget", logicWorld, false, 0);

    // ---- Vacuum decay chamber (hole only here, for beam pipe representation) ----
    G4Tubs* dcOuter = new G4Tubs("DCOuter", 0, 10*cm, decayChamberHalfZ, 0, 360*deg);
    
    fDecayChamberLV = new G4LogicalVolume(dcOuter, fVacuum, "DecayChamber");
    new G4PVPlacement(0, G4ThreeVector(0,0,decayChamberCenterZ),
                      fDecayChamberLV, "DecayChamber", logicWorld, false, 0);

    // ---- Silicon tracker layers (SOLID — no holes) ----
    for (int i = 0; i < 6; i++) {
        G4Box* siSolid = new G4Box("SiLayer", 5*cm, 5*cm, 0.1*cm);
        fSiTrackerLV[i] = new G4LogicalVolume(siSolid, fSilicon, "SiTracker");
        G4double zPos = -40*cm + i*2*cm;
        new G4PVPlacement(0, G4ThreeVector(0,0,zPos),
                          fSiTrackerLV[i], "SiTracker", logicWorld, false, i);
    }

    // ---- CsI calorimeter (SOLID — no hole) ----
    G4Box* csiSolid = new G4Box("CsICalorimeter", 6*cm, 6*cm, 22*cm);
    fCsICalLV = new G4LogicalVolume(csiSolid, fCsI, "CsICalorimeter");
    new G4PVPlacement(0, G4ThreeVector(0,0,fCsIPositionZ),
                      fCsICalLV, "CsICalorimeter", logicWorld, false, 0);

    // ---- Scoring volumes (invisible) ----
    G4VisAttributes* invisVis = new G4VisAttributes();
    invisVis->SetVisibility(false);

    G4Box* scoringBox = new G4Box("ScoringBox", 15*cm, 15*cm, 0.1*mm);
    fScoringVolumeLV = new G4LogicalVolume(scoringBox, fVacuum, "ScoringVolume");
    new G4PVPlacement(0, G4ThreeVector(0,0,-28*cm),
                      fScoringVolumeLV, "ScoringVolume", logicWorld, false, 0);
    fScoringVolumeLV->SetVisAttributes(invisVis);

    G4Box* targetExitBox = new G4Box("TargetExitBox", 15*cm, 15*cm, 0.1*mm);
    G4LogicalVolume* targetExitLV = new G4LogicalVolume(targetExitBox, fVacuum, "TargetExitVolume");
    new G4PVPlacement(0, G4ThreeVector(0,0,targetExitScoringZ),
                      targetExitLV, "TargetExitVolume", logicWorld, false, 0);
    targetExitLV->SetVisAttributes(invisVis);

    // ---- Visualization colors ----
    fTungstenTargetLV->SetVisAttributes(new G4VisAttributes(G4Colour(0.7, 0.7, 0.7)));      // grey
    fDecayChamberLV->SetVisAttributes(new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 0.3)));  // blue, transparent
    G4VisAttributes* siVis = new G4VisAttributes(G4Colour(0.0, 1.0, 1.0));                  // cyan
    for (int i = 0; i < 6; i++) fSiTrackerLV[i]->SetVisAttributes(siVis);
    fCsICalLV->SetVisAttributes(new G4VisAttributes(G4Colour(1.0, 0.0, 1.0)));             // magenta

    return physWorld;
}

void DAMSADetectorConstruction::SetupMagneticField()
{
    fMagField = new G4UniformMagField(G4ThreeVector(0., 1.0*tesla, 0.));
    fFieldMgr = new G4FieldManager();
    fFieldMgr->SetDetectorField(fMagField);
    fFieldMgr->CreateChordFinder(fMagField);
    G4cout << "1T magnetic field applied in y-direction" << G4endl;
}

void DAMSADetectorConstruction::ConstructSDandField()
{
    G4SDManager* SDman = G4SDManager::GetSDMpointer();

    DAMSASensitiveDetector* siSD =
        new DAMSASensitiveDetector("SiTrackerSD", "SiTrackerHitsCollection");
    SDman->AddNewDetector(siSD);
    for (int i = 0; i < 6; i++) fSiTrackerLV[i]->SetSensitiveDetector(siSD);

    DAMSASensitiveDetector* csiSD =
        new DAMSASensitiveDetector("CsICalSD", "CsICalHitsCollection");
    SDman->AddNewDetector(csiSD);
    fCsICalLV->SetSensitiveDetector(csiSD);

    G4cout << "Sensitive detectors configured for Si trackers and CsI calorimeter" << G4endl;
}
