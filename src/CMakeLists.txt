aux_source_directory(. SRC_LISTA)
aux_source_directory(util SRC_LISTB)
aux_source_directory(ofb SRC_LISTC)

include_directories(.)
include_directories(util)
include_directories(ofb)

#include_directories(${CMAKE_CURRENT_BINARY_DIR})


#include(FetchContent)
#FetchContent_Declare(oflex URL https://github.com/Jiangshan00001/oflex/archive/refs/heads/master.zip)
#FetchContent_MakeAvailable(oflex)

#FetchContent_Declare(obison_old URL https://github.com/Jiangshan00001/obison/archive/refs/heads/main.zip)
#FetchContent_MakeAvailable(obison_old)


#add_custom_command(
#  OUTPUT
#  	${CMAKE_CURRENT_BINARY_DIR}/bison_parser.h
#  COMMAND
#    obison_old -i ${CMAKE_CURRENT_SOURCE_DIR}/ofb/obison11.yy  -p ${CMAKE_CURRENT_BINARY_DIR}/bison_parser.h -e ${CMAKE_CURRENT_BINARY_DIR}/bison_header.h -c ansi_c_parser -n ansi_c_header
#
#  WORKING_DIRECTORY
#  	${CMAKE_CURRENT_BINARY_DIR}
#  DEPENDS
#  	${CMAKE_CURRENT_SOURCE_DIR}/ofb/obison11.yy
#  COMMENT
#  	"yy->parser"
#  )
  
##add_custom_command(
#  OUTPUT
#  	${CMAKE_CURRENT_BINARY_DIR}/lex_header.h
#  COMMAND
#    oflex -i ${CMAKE_CURRENT_SOURCE_DIR}/ofb/obison11.l  -o ${CMAKE_CURRENT_BINARY_DIR}/lex_header.h -t ${CMAKE_CURRENT_BINARY_DIR}/otoken.h -p ansi_c_lex -k ansi_c_token
#  WORKING_DIRECTORY
#  	${CMAKE_CURRENT_BINARY_DIR}
#  DEPENDS
#  	${CMAKE_CURRENT_SOURCE_DIR}/ofb/obison11.l 
#  COMMENT
#  	".l->lex"
#  )
  
add_executable(obison ${SRC_LISTA} ${SRC_LISTB} ${SRC_LISTC})
