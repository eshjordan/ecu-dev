#pragma once

#include "stdint.h"


/**
 * @brief Enum to represent each of the generic C types, so we can record types in member
 * variables.
 *
 */
typedef enum {
    PARAM_UNDEFINED,
    PARAM_SHORT_INT,
    PARAM_INT,
    PARAM_LONG_INT,
    PARAM_LONG_LONG_INT,
    PARAM_UNSIGNED_SHORT_INT,
    PARAM_UNSIGNED_INT,
    PARAM_UNSIGNED_LONG_INT,
    PARAM_UNSIGNED_LONG_LONG_INT,
    PARAM_BOOL,
    PARAM_SIGNED_CHAR,
    PARAM_UNSIGNED_CHAR,
    PARAM_CHAR,
    PARAM_WCHAR,
    PARAM_CHAR16,
    PARAM_CHAR32,
    PARAM_FLOAT,
    PARAM_DOUBLE,
    PARAM_LONG_DOUBLE,
    PARAM_STRING
} ParameterType_en;


/**
 * @brief Parameter struct to store named values.
 *
 */
typedef struct
{
    /** @brief Value stored by the Parameter. */
    uint8_t m_value[32];

    /** @brief Name of the Parameter. */
    char m_name[32];

    /** @brief Type of the Parameter. */
    ParameterType_en m_type;

    /** @brief Parameter */
    uint32_t m_size;

} Parameter_st;
