#ifndef MYTRAJECTORY_HH
#define MYTRAJECTORY_HH

#include "G4Trajectory.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"

class MyTrajectory : public G4Trajectory {
public:
    MyTrajectory(const G4Track* aTrack);
    ~MyTrajectory() override;

    void DrawTrajectory(G4int i_mode = 0) const override;

private:
    G4Colour fColor;
};

#endif
