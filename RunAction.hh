#ifndef RUNACTION_HH
#define RUNACTION_HH

#include "G4UserRunAction.hh"
#include "globals.hh"
#include <fstream>

class G4Run;

class RunAction : public G4UserRunAction
{
  public:
    RunAction();
    virtual ~RunAction();

    virtual void BeginOfRunAction(const G4Run* run);
    virtual void EndOfRunAction(const G4Run* run);

    void AddEdep(G4double edep) { fTotalEdep += edep; }

    // --- CSV helpers for EventAction ---
    void WriteEcalHit(G4int event, G4int layer, G4double edepMeV);
    void WriteEventSummary(G4int event, G4double ecalSumMeV,
                           G4double tailEdepMeV, G4double trkEdepMeV);

    // --- CSV helper for PhaseSpaceSD (one row per crossing track) ---
    void WritePhaseRow(G4int event, G4int track, G4int parent, G4int pdg,
                       G4double charge_e, G4double Ek_MeV, G4double Etot_MeV,
                       G4double px_MeV, G4double py_MeV, G4double pz_MeV,
                       G4double theta_rad, G4double phi_rad,
                       G4double x_mm, G4double y_mm, G4double z_mm,
                       G4double t_ns) const;

  private:
    G4double              fTotalEdep;
    std::ofstream         fEcalCSV;    // ecal.csv
    std::ofstream         fEventCSV;   // event.csv
    mutable std::ofstream fPhaseCSV;   // phase.csv
};

#endif
