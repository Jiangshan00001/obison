aux_source_directory(. SRC_LISTA)
aux_source_directory(util SRC_LISTB)
aux_source_directory(ofb SRC_LISTC)

include_directories(.)
include_directories(util)
include_directories(ofb)

add_executable(obison ${SRC_LISTA} ${SRC_LISTB} ${SRC_LISTC})
