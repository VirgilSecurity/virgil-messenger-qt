
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

FUNCTION(PAD_STRING OUT_VARIABLE DESIRED_LENGTH FILL_CHAR VALUE)
    STRING(LENGTH "${VALUE}" VALUE_LENGTH)
    MATH(EXPR REQUIRED_PADS "${DESIRED_LENGTH} - ${VALUE_LENGTH}")
    SET(PAD ${VALUE})
    IF(REQUIRED_PADS GREATER 0)
        MATH(EXPR REQUIRED_MINUS_ONE "${REQUIRED_PADS} - 1")
        FOREACH(FOO RANGE ${REQUIRED_MINUS_ONE})
            SET(PAD "${FILL_CHAR}${PAD}")
        ENDFOREACH()
    ENDIF()
    SET(${OUT_VARIABLE} "${PAD}" PARENT_SCOPE)
ENDFUNCTION()