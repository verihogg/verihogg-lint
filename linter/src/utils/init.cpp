#include "utils/init.h"

using namespace SURELOG;

void initCommandLineParser(CommandLineParser* clp) {
  clp->noPython();
  clp->setParse(true);
  clp->setCompile(true);
  clp->setElaborate(false);
  clp->setwritePpOutput(true);
  clp->setCacheAllowed(false);
  clp->setFilterInfo();
  clp->setFilterNote();
  clp->setFilterWarning();
}
