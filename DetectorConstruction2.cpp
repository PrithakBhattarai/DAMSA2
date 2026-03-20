
#include "DetectorConstruction.hh"

#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4RotationMatrix.hh"

#include "G4UniformMagField.hh"
#include "G4FieldManager.hh"
#include "G4ChordFinder.hh"

#include "G4SDManager.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4PSEnergyDeposit.hh"
#include "G4PSNofSecondary.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "PhaseSpaceSD.hh"

DetectorConstruction::DetectorConstruction()
: fTrackerLV(nullptr), fEcalLayerLV(nullptr), fTailLV(nullptr), fFieldMgr(nullptr),
  fPhasePlaneLV(nullptr), fPhaseDz(1.*um)
{
  // World
  fWorldSize = 6.0*m;

  // Target (from slide: 0.05 x 0.05 x 0.10 m^3 W)
  fTargetX = 5.0*cm; fTargetY = 5.0*cm; fTargetZ = 10.0*cm; 

  // Vacuum decay chamber (R=0.1 m, L=0.3 m)  
  fVacRadius = 10.0*cm; fVacLength = 30.0*cm;

  // Gamma converter: 4 W layers totaling ~3 X0  
  fNConvLayers = 4; fConvTotalX0 = 3.0;

  // Tracker: 6 Si planes, 10x10 cm^2, 1T field  
  fNTrackerLayers   = 6;
  fTrackerSizeXY    = 10.0*cm;
  fTrackerThickness = 0.03*cm;    // 300 μm sensor
  fBFieldTesla      = 1.0;

  // ECAL: 44 layers, each 12x12x1 cm^3 (CsI) 
  fNEcalLayers        = 44;
  fEcalTileXY         = 12.0*cm;
  fEcalLayerThickness = 1.0*cm;

  // Tail catcher: ~10 cm CsI (≈5 X0)  
  fTailThickness = 10.0*cm;
}

DetectorConstruction::~DetectorConstruction() {}

// Materials

void DetectorConstruction::DefineMaterials()
{
  auto* nist = G4NistManager::Instance();

  nist->FindOrBuildMaterial("G4_AIR");
  nist->FindOrBuildMaterial("G4_Fe");
  nist->FindOrBuildMaterial("G4_W");
  nist->FindOrBuildMaterial("G4_Si");
  nist->FindOrBuildMaterial("G4_Galactic");

  // Undoped CsI 
  auto* elCs = nist->FindOrBuildElement("Cs");
  auto* elI  = nist->FindOrBuildElement("I");
  auto* CsI  = new G4Material("CsI", 4.51*g/cm3, 2);
  CsI->AddElement(elCs, 1);
  CsI->AddElement(elI,  1);

  (void)CsI; 
}
// Construct geometry

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  DefineMaterials();
  auto* nist = G4NistManager::Instance();

  // World
  auto* solidWorld = new G4Box("World", fWorldSize/2, fWorldSize/2, fWorldSize/2);
  auto* logicWorld = new G4LogicalVolume(solidWorld, nist->FindOrBuildMaterial("G4_AIR"), "WorldLV");
  auto* physWorld  = new G4PVPlacement(nullptr, {}, logicWorld, "World", nullptr, false, 0);

  auto* worldVis = new G4VisAttributes(G4Colour(0.8,0.9,1.0,0.05));
  worldVis->SetForceSolid(false);
  logicWorld->SetVisAttributes(worldVis);

  // Beamline along +Z
  G4double z = -2.0*m;

  // --- Lead target: 5×5×10 cm^3 ---
  {
    auto* s = new G4Box("Target", fTargetX/2, fTargetY/2, fTargetZ/2);
    auto* l = new G4LogicalVolume(s, nist->FindOrBuildMaterial("G4_W"), "TargetLV");
    new G4PVPlacement(nullptr, {0,0,z + fTargetZ/2}, l, "Target", logicWorld, false, 0);
    auto* v = new G4VisAttributes(G4Colour(0.45,0.45,0.45));
    v->SetForceSolid(true);
    l->SetVisAttributes(v);
    z += fTargetZ;
  }

  //  Phase-space plane
{
  const G4double dz = fPhaseDz; // 1 um
  auto* s = new G4Box("PhasePlane", fTrackerSizeXY/2, fTrackerSizeXY/2, dz/2);
  // vacuum material to avoid interactions
  fPhasePlaneLV = new G4LogicalVolume(s, G4Material::GetMaterial("G4_Galactic"), "PhasePlaneLV");
  auto* v = new G4VisAttributes(G4Colour(0.0,1.0,0.0,0.2));
  v->SetForceSolid(false);
  fPhasePlaneLV->SetVisAttributes(v);

  new G4PVPlacement(nullptr, {0,0,z + fPhaseDz/2}, fPhasePlaneLV, "PhasePlane", logicWorld, false, 0, true);
  z += fPhaseDz; 
}


  // Vacuum decay chamber: 1  (Fe) wall + inner vacuum 
{
  const G4double wallThick = 1.0*mm;
  const G4double rVac      = fVacRadius;              // inner (vacuum) radius
  const G4double rWallOut  = rVac + wallThick;        // outer radius of Al shell
  const G4double hz        = fVacLength/2;            // half length

  // Aluminium cylindrical shell 
  auto* sShell = new G4Tubs("VDC_FeShell", rVac, rWallOut, hz, 0.*deg, 360.*deg);
  auto* lShell = new G4LogicalVolume(sShell, nist->FindOrBuildMaterial("G4_Fe"), "VDC_FeShellLV");
  new G4PVPlacement(nullptr, {0,0,z + hz}, lShell, "VDC_FeShell", logicWorld, false, 0, true);

  auto* vShell = new G4VisAttributes(G4Colour(0.7,0.7,0.8,0.4));
  vShell->SetForceSolid(false);  
  lShell->SetVisAttributes(vShell);

  // Inner vacuum volume
  auto* sVac = new G4Tubs("VDC_Vacuum", 0.*mm, rVac, hz, 0.*deg, 360.*deg);
  auto* lVac = new G4LogicalVolume(sVac, nist->FindOrBuildMaterial("G4_Galactic"), "VDC_VacuumLV");
  new G4PVPlacement(nullptr, {}, lVac, "VDC_Vacuum", lShell, false, 0, true);

  auto* vVac = new G4VisAttributes(G4Colour(0.95,0.95,0.99,0.05));
  vVac->SetForceSolid(false);
  lVac->SetVisAttributes(vVac);

  
  z += fVacLength;
}


  G4LogicalVolume* trackerRegionLV = nullptr;
  {
    const G4double regionL = 11.0*cm;
    auto* s = new G4Box("TrackerRegion", fTrackerSizeXY/1.5, fTrackerSizeXY/1.5, regionL/2);
    trackerRegionLV = new G4LogicalVolume(s, nist->FindOrBuildMaterial("G4_AIR"), "TrackerRegionLV");
    new G4PVPlacement(nullptr, {0,0,z + regionL/2}, trackerRegionLV, "TrackerRegion", logicWorld, false, 0);
    auto* v = new G4VisAttributes(G4Colour(0.8,0.8,0.2,0.1));
    v->SetForceSolid(false);
    trackerRegionLV->SetVisAttributes(v);

    // Magnetic field (uniform 1 T along +Y)
    auto* bfield = new G4UniformMagField(G4ThreeVector(0., fBFieldTesla*tesla, 0.));
    fFieldMgr = new G4FieldManager(bfield);
    fFieldMgr->CreateChordFinder(bfield);
    trackerRegionLV->SetFieldManager(fFieldMgr, true);

    // Silicon plane LV (300 μm)
    fTrackerLV = new G4LogicalVolume(
      new G4Box("SiPlane", fTrackerSizeXY/2, fTrackerSizeXY/2, fTrackerThickness/2),
      nist->FindOrBuildMaterial("G4_Si"),
      "SiPlaneLV"
    );
    auto* sv = new G4VisAttributes(G4Colour(0.95,0.2,0.2));
    sv->SetForceSolid(true);
    fTrackerLV->SetVisAttributes(sv);

    // Place 6 planes evenly spaced within region
    for (int i=0; i<fNTrackerLayers; ++i) {
      const G4double localZ = -regionL/2 + (i+1)*regionL/(fNTrackerLayers+1);
      new G4PVPlacement(nullptr, {0,0,localZ}, fTrackerLV, "SiPlane", trackerRegionLV, false, i);
    }
    z += regionL;
  }

 
{
  // ECAL mother (air)
  const G4double ecalX = fEcalTileXY;                         // 12 cm
  const G4double ecalY = fEcalTileXY;                         // 12 cm
  const G4double ecalZ = fNEcalLayers * fEcalLayerThickness;  // 44 cm

  auto* sECAL = new G4Box("ECAL", ecalX/2, ecalY/2, ecalZ/2);
  auto* lECAL = new G4LogicalVolume(sECAL,
                                    nist->FindOrBuildMaterial("G4_AIR"),
                                    "ECALLV");

  // place ECAL on the beamline; its 12×12 face is normal to +Z
  new G4PVPlacement(nullptr, {0, 0, z + ecalZ/2}, lECAL, "ECAL",
                    logicWorld, false, 0, true);

  auto* vECAL = new G4VisAttributes(G4Colour(0., 0., 1., 0.10));
  vECAL->SetForceSolid(false);
  lECAL->SetVisAttributes(vECAL);

  // One CsI layer: thickness along Z (1 cm), face 12×12 cm² in X–Y
  fEcalLayerLV = new G4LogicalVolume(
      new G4Box("EcalLayer", ecalX/2, ecalY/2, fEcalLayerThickness/2),
      G4Material::GetMaterial("CsI"),
      "EcalLayerLV");

  auto* vChip = new G4VisAttributes(G4Colour(0.2, 0.4, 0.9));
  vChip->SetForceSolid(true);
  fEcalLayerLV->SetVisAttributes(vChip);

  // Stack 44 layers along Z with alternating orientation
  const G4double z0 = -ecalZ/2 + fEcalLayerThickness/2;
  for (int i = 0; i < fNEcalLayers; ++i) {
    const G4double zi = z0 + i * fEcalLayerThickness;

    // Alternate 0° / 90° around Z so neighboring layers are perpendicular
    auto* rot = new G4RotationMatrix();
    if (i % 2 == 1) rot->rotateZ(90*deg);

    new G4PVPlacement(rot, {0., 0., zi},
                      fEcalLayerLV, "EcalLayer",
                      lECAL, false, i, true);
  }

  // ECAL depth (44 cm is along Z)
  z += ecalZ ;
}


  // Tail catcher: ~10 cm CsI 
  {
    fTailLV = new G4LogicalVolume(
      new G4Box("TailCatcher", fEcalTileXY/2, fEcalTileXY/2, fTailThickness/2),
      G4Material::GetMaterial("CsI"),
      "TailCatcherLV"
    );
    auto* v = new G4VisAttributes(G4Colour(0.1,0.2,0.5));
    v->SetForceSolid(true);
    fTailLV->SetVisAttributes(v);

    new G4PVPlacement(nullptr, {0,0,z + fTailThickness/2}, fTailLV, "TailCatcher", logicWorld, false, 0);
  }

  return physWorld;
}


// Sensitive detectors & field scoring
void DetectorConstruction::ConstructSDandField()
{
  auto* sdm = G4SDManager::GetSDMpointer();

  // Tracker scoring
  {
    auto* mfd = new G4MultiFunctionalDetector("TrackerMFD");
    sdm->AddNewDetector(mfd);
    mfd->RegisterPrimitive(new G4PSEnergyDeposit("Edep"));
    mfd->RegisterPrimitive(new G4PSNofSecondary("Nsec"));
    SetSensitiveDetector(fTrackerLV, mfd);
  }

  // ECAL scoring (per layer)
  {
    auto* mfd = new G4MultiFunctionalDetector("EcalMFD");
    sdm->AddNewDetector(mfd);
    mfd->RegisterPrimitive(new G4PSEnergyDeposit("Edep"));
    SetSensitiveDetector(fEcalLayerLV, mfd);
  }

  // Tail catcher scoring
  {
    auto* mfd = new G4MultiFunctionalDetector("TailMFD");
    sdm->AddNewDetector(mfd);
    mfd->RegisterPrimitive(new G4PSEnergyDeposit("Edep"));
    SetSensitiveDetector(fTailLV, mfd);
  }

  {
    auto* psd = new PhaseSpaceSD("PhaseSD");
    sdm->AddNewDetector(psd);
    SetSensitiveDetector(fPhasePlaneLV, psd);
  }
}
