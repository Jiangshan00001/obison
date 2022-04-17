
enable_testing()

function(y_to_inc file_name_in file_name_out)
    add_test( NAME y_to_inc${file_name} COMMAND obison -i ${file_name_in} -o ${file_name_out} )
endfunction()


function(regrex_to_dfa reg_str file_name)
    add_test( NAME regex_to_dfa${file_name} COMMAND obison -s ${reg_str} -d ${file_name} )
endfunction()

function(hex_to_asm  HEX_FILE  ASM_FILE)
    add_test( NAME test_hex_to_asm${HEX_FILE} COMMAND asm8051 -dis -nd  ${HEX_FILE} ${ASM_FILE} )
endfunction()


function(diff_dfa  asm1  asm2)
    add_test( NAME test_diff_dfa${asm1} COMMAND diff -a -b  ${asm1} ${asm2} )
endfunction()

set(TEST_SRCS )

foreach(MY_ASM  ${TEST_SRCS})

    set(IFILE_NAME "")
    set(IFILE_BASE "")
    set(IFILE_CURR "")
    string(APPEND IFILE_NAME      "${MY_ASM}" ".r" )
    string(APPEND IFILE_BASE      "${MY_ASM}" ".dot" )
    string(APPEND IFILE_CURR      "${MY_ASM}" "_out.dot" )

    file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/${IFILE_NAME} REG_STR)
    regrex_to_dfa(${REG_STR}  ${CMAKE_CURRENT_BINARY_DIR}/${IFILE_CURR} )
    diff_dfa(${CMAKE_CURRENT_SOURCE_DIR}/${IFILE_BASE}  ${CMAKE_CURRENT_BINARY_DIR}/${IFILE_CURR})

endforeach()

set(TEST_Y_SRCS test1.y)

foreach(MY_ASM  ${TEST_Y_SRCS})
    y_to_inc(${CMAKE_CURRENT_SOURCE_DIR}/${MY_ASM}  ${CMAKE_CURRENT_BINARY_DIR}/${MY_ASM}_out.inc )
    diff_dfa(${CMAKE_CURRENT_SOURCE_DIR}/${MY_ASM}.inc  ${CMAKE_CURRENT_BINARY_DIR}/${MY_ASM}_out.inc)
endforeach()
