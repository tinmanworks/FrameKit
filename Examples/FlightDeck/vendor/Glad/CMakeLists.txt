message(STATUS "${CMAKE_CURRENT_LIST_FILE}")

set(GLAD_DIR glad_gl4.6)

# Glad is C; only grab .c and headers
file(GLOB_RECURSE glad_c
        "${CMAKE_CURRENT_LIST_DIR}/${GLAD_DIR}/*.c"
        "${CMAKE_CURRENT_LIST_DIR}/${GLAD_DIR}/*.h"
        "${CMAKE_CURRENT_LIST_DIR}/${GLAD_DIR}/glad/*.h"
        "${CMAKE_CURRENT_LIST_DIR}/${GLAD_DIR}/glad/*.c"
)

add_library(Glad STATIC ${glad_c})

# Glad should link as C, not C++
set_target_properties(Glad PROPERTIES LINKER_LANGUAGE C)

target_include_directories(Glad
        PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/${GLAD_DIR}"
)

source_group("Glad" FILES ${glad_c})
set_property(TARGET Glad PROPERTY FOLDER "Examples/FlightDeck/vendor")
