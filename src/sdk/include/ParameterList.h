#pragma once

#include "Parameter.h"
#include "stddef.h"


/**
 * @brief Get the number of Parameters in the ParameterList.
 *
 * @return size_t Number of Parameters in the ParameterList.
 */
size_t parameterListGetSize(void);


/**
 * @brief Add a new named Parameter to the ParameterList.
 *
 * @tparam T Type of the Parameter's value.
 * @param name Name of the Parameter.
 * @param value Value of the Parameter.
 */
void parameterListAddParameter(const char *const name, const void *const value, const ParameterType_en type, const uint32_t size);

/**
 * @brief Get the value of a named parameter.
 *
 * @tparam T Type of the Parameter's value. Set explicitly to avoid runtime errors.
 * @param name Name of the Parameter.
 * @return T Current value of the Parameter.
 */
uint32_t parameterListGetParameter(const char *const name, void *const value, ParameterType_en *const type);


/**
 * @brief Set a new value for a named Parameter.
 *
 * @tparam T Type of the Parameter's value. Set explicitly to avoid runtime errors.
 * @param name Name of the Parameter.
 * @param value New value of the Parameter.
 */
void parameterListSetParameter(const char *const name, const void *const value, const ParameterType_en type, const uint32_t size);
