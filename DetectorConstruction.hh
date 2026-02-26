// DetectorConstruction.hh
#ifndef DetectorConstruction_hh
#define DetectorConstruction_hh

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4LogicalVolume;
class G4FieldManager;

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
  DetectorConstruction();
  ~DetectorConstruction() override;

  G4VPhysicalVolume* Construct() override;
  void ConstructSDandField() override;

  // getter used by RunAction (scoring volume)
  G4LogicalVolume* GetScoringVolume() const { return fScoringVolume; }
  
private:
  // World
  G4double fWorldSize;

  // Target (Tungsten)
  G4double fTargetX, fTargetY, fTargetZ;

  //Phase space plane
  
G4LogicalVolume* fPhasePlaneLV;  // new
G4double         fPhaseDz;       // new


  // Vacuum decay chamber
  G4double fVacRadius;
  G4double fVacLength;

  // Gamma converter (Tungsten)
  G4int    fNConvLayers;
  G4double fConvTotalX0;

  // Tracker (Silicon planes)
  G4int    fNTrackerLayers;
  G4double fTrackerSizeXY;
  G4double fTrackerThickness;
  G4double fBFieldTesla;

  // ECAL (CsI) and Tail
  G4int    fNEcalLayers;
  G4double fEcalTileXY;
  G4double fEcalLayerThickness;
  G4double fTailThickness;

  // Cached LVs for assigning SDs and scoring
  G4LogicalVolume* fTrackerLV;
  G4LogicalVolume* fEcalLayerLV;
  G4LogicalVolume* fTailLV;
  G4LogicalVolume* fScoringVolume;

  // Magnetic field
  G4FieldManager*  fFieldMgr;

  void DefineMaterials();
};

#endif // DetectorConstruction_hh
