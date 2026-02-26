#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"

void ActionInitialization::Build() const {
    SetUserAction(new PrimaryGeneratorAction());

    // Create RunAction and pass its pointer into EventAction
    auto* runAction = new RunAction();
    SetUserAction(runAction);

    SetUserAction(new EventAction(runAction));
}
