# Release 0.9 2018-06-17
    ## New naming scheme for functions
    Removed backwards comaptibility by changing names of multiple methods to lowercase start of camelcase names (for uniformity)
    ## New functionality
    * Tiles now store their terrain height
    * Tiles now store their terrain type, which describes accessibility and neutral unit type (Xel'Naga Watchtower, minerals, gas, destructible rocks).
    * Path finding, between chokepoints.
    * Added file `src/Definitions.h` containing constans which users can use to optimize the regions and chokepoints for their needs.
    * Move to gitlab.
