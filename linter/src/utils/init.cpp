#include "utils/init.h"

#include <Surelog/CommandLine/CommandLineParser.h>

void InitCommandLineParser(SURELOG::CommandLineParser* clp) {
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
