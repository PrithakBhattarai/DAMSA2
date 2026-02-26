#include "RunAction.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include <iomanip>

RunAction::RunAction()
: G4UserRunAction(), fTotalEdep(0.) {}

RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run* /*run*/)
{
  fTotalEdep = 0.;

  // Open CSV files (overwrite each run)
  fEcalCSV.open("ecal.csv");
  fEventCSV.open("event.csv");
  fPhaseCSV.open("phase.csv");                 // NEW

  // Write headers
  if (fEcalCSV.is_open())
    fEcalCSV << "event,layer,edepMeV\n";
  if (fEventCSV.is_open())
    fEventCSV << "event,EcalSumMeV,TailEdepMeV,TrackerEdepMeV\n";
  if (fPhaseCSV.is_open())                    // NEW
    fPhaseCSV << "event,track,parent,pdg,charge_e,Ek_MeV,Etot_MeV,"
                 "px_MeV,py_MeV,pz_MeV,theta_rad,phi_rad,"
                 "x_mm,y_mm,z_mm,t_ns\n";
}

void RunAction::EndOfRunAction(const G4Run* /*run*/)
{
  if (fEcalCSV.is_open())  fEcalCSV.close();
  if (fEventCSV.is_open()) fEventCSV.close();
  if (fPhaseCSV.is_open()) fPhaseCSV.close();          // NEW

  G4cout << "Total Energy Deposition in Run: "
         << fTotalEdep / CLHEP::MeV << " MeV" << G4endl;
}

void RunAction::WriteEcalHit(G4int event, G4int layer, G4double edepMeV)
{
  if (fEcalCSV.is_open()) {
    fEcalCSV << event << ',' << layer << ','
             << std::setprecision(10) << edepMeV << '\n';
  }
}

void RunAction::WriteEventSummary(G4int event, G4double ecalSumMeV,
                                  G4double tailEdepMeV, G4double trkEdepMeV)
{
  if (fEventCSV.is_open()) {
    fEventCSV << event << ','
              << std::setprecision(10) << ecalSumMeV << ','
              << tailEdepMeV << ','
              << trkEdepMeV << '\n';
  }
}

// NEW: one row per particle crossing the phase plane
void RunAction::WritePhaseRow(G4int event, G4int track, G4int parent, G4int pdg,
                       G4double charge_e, G4double Ek_MeV, G4double Etot_MeV,
                       G4double px_MeV, G4double py_MeV, G4double pz_MeV,
                       G4double theta_rad, G4double phi_rad,
                       G4double x_mm, G4double y_mm, G4double z_mm,
                       G4double t_ns) const
{
  if (fPhaseCSV.is_open()) {
    fPhaseCSV << event << ',' << track << ',' << parent << ',' << pdg << ','
              << std::setprecision(10)
              << charge_e << ',' << Ek_MeV << ',' << Etot_MeV << ','
              << px_MeV << ',' << py_MeV << ',' << pz_MeV << ','
              << theta_rad << ',' << phi_rad << ','
              << x_mm << ',' << y_mm << ',' << z_mm << ','
              << t_ns << '\n';
  }
}
