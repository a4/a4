from AthenaPython import PyAthena

from AthenaCommon.Logging import logging
from AthenaCommon.AlgSequence import AlgSequence

class A4DumperAlg( A4Dumper__A4DumperAlg ):

    def __init__( self,
                  name,
                  seq = topSequence,
                  **kwargs ):

        self.__logger = logging.getLogger( "A4DumperAlg" )

