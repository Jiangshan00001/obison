aux_source_directory(. SRC_LISTA)
aux_source_directory(util SRC_LISTB)

include_directories(.)
include_directories(util)

add_executable(obison ${SRC_LISTA} ${SRC_LISTB})
