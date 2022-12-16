

FIND_PATH(LIBPICO_INCLUDE_DIR NAMES ps4000aApi.h
                HINTS /usr/include /opt/picoscope/include/libps4000a/)
#message("LIBPICO_INCLUDE_DIR: " ${LIBPICO_INCLUDE_DIR})
FIND_LIBRARY(LIBPICO_LIBRARY NAMES ps4000a libps4000a picoscope-ps4000a
                HINTS /usr/lib64 /opt/picoscope/lib)
#message("LIBPICO_LIBRARY: " ${LIBPICO_LIBRARY})

#MARK_AS_ADVANCED(LIBPICO_INCLUDE_DIR LIBPICO_LIBRARY)

include_directories(${LIBPICO_INCLUDE_DIR})
include_directories(/opt/picoscope/lib)
link_directories(${LIBPICO_INCLUDE_DIR})
link_directories(${LIBPICO_LIBRARY})
#include(${LIBPICO_LIBRARY})

SET(PICOSCOPE_LIB_DIR ${LIBPICO_LIBRARY})
SET(PICOSCOPE_INCLUDE_DIR ${LIBPICO_INCLUDE_DIR})

