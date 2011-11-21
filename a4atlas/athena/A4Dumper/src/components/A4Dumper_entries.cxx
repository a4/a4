#include "GaudiKernel/DeclareFactoryEntries.h"

#include "../A4DumperSvc.h"
#include "../A4DumperAlg.h"

DECLARE_NAMESPACE_SERVICE_FACTORY  (A4Dumper, A4DumperSvc)
DECLARE_NAMESPACE_ALGORITHM_FACTORY(A4Dumper, A4DumperAlg)


DECLARE_FACTORY_ENTRIES(A4Dumper) {
  DECLARE_NAMESPACE_SERVICE   (A4Dumper, A4DumperSvc)
  DECLARE_NAMESPACE_ALGORITHM(A4Dumper, A4DumperAlg)
}
