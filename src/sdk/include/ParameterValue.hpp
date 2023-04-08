// Copyright 2018 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <stdint.h>
#include "Error.hpp"
#include <type_traits>

namespace System
{
namespace Impl
{

enum ParameterType : uint8_t
{
  PARAMETER_NOT_SET,
  PARAMETER_BOOL,
  PARAMETER_INTEGER,
  PARAMETER_DOUBLE,
  PARAMETER_STRING,
  PARAMETER_BYTE_ARRAY,
};

/// Return the name of a parameter type
void
parameter_type_to_string(char *out, ParameterType type);


struct _ParameterValue
{
  // The type of this parameter, which corresponds to the appropriate field below.
  ParameterType type;

  // "Variant" style storage of the parameter value. Only the value corresponding
  // the type field will have valid information.

  // Boolean value, can be either true or false.
  bool bool_value;

  // Integer value ranging from -9,223,372,036,854,775,808 to
  // 9,223,372,036,854,775,807.
  int64_t integer_value;

  // A double precision floating point value following IEEE 754.
  double double_value;

  // An array of bytes, used for anything.
  uint8_t byte_array_value[255];
  uint8_t byte_array_count;
};



/// Store the type and value of a parameter.
class ParameterValue
{
public:
  /// Construct a parameter value with type PARAMETER_NOT_SET.
  ParameterValue();
  /// Construct a parameter value with type PARAMETER_BOOL.
  explicit ParameterValue(const bool bool_value);
  /// Construct a parameter value with type PARAMETER_INTEGER.
  explicit ParameterValue(const int int_value);
  /// Construct a parameter value with type PARAMETER_INTEGER.
  explicit ParameterValue(const int64_t int_value);
  /// Construct a parameter value with type PARAMETER_DOUBLE.
  explicit ParameterValue(const float double_value);
  /// Construct a parameter value with type PARAMETER_DOUBLE.
  explicit ParameterValue(const double double_value);
  /// Construct a parameter value with type PARAMETER_STRING.
  explicit ParameterValue(const char * string_value);
  /// Construct a parameter value with type PARAMETER_BYTE_ARRAY.
  explicit ParameterValue(const uint8_t * byte_array_value, uint8_t byte_array_count);

  explicit ParameterValue(const ParameterValue &other);

  /// Return an enum indicating the type of the set value.
  ParameterType
  get_type() const;

  /// Equal operator.
  bool
  operator==(const ParameterValue & rhs) const;

  /// Not equal operator.
  bool
  operator!=(const ParameterValue & rhs) const;

  // The following get() variants require the use of ParameterType

  template<ParameterType type>
  constexpr
  typename std::enable_if<type == ParameterType::PARAMETER_BOOL, const bool &>::type
  get() const
  {
    if(value_.type != ParameterType::PARAMETER_BOOL) {
      ecu_fatal_error("Invalid param type");
    }
    return value_.bool_value;
  }

  template<ParameterType type>
  constexpr
  typename std::enable_if<type == ParameterType::PARAMETER_INTEGER, const int64_t &>::type
  get() const
  {
    if (value_.type != ParameterType::PARAMETER_INTEGER) {
        ecu_fatal_error("Invalid param type");
    }
    return value_.integer_value;
  }

  template<ParameterType type>
  constexpr
  typename std::enable_if<type == ParameterType::PARAMETER_DOUBLE, const double &>::type
  get() const
  {
    if (value_.type != ParameterType::PARAMETER_DOUBLE) {
        ecu_fatal_error("Invalid param type");
    }
    return value_.double_value;
  }

  template<ParameterType type>
  constexpr
  typename std::enable_if<type == ParameterType::PARAMETER_STRING, const char *>::type
  get() const
  {
    if (value_.type != ParameterType::PARAMETER_STRING) {
        ecu_fatal_error("Invalid param type");
    }
    return (char*)value_.byte_array_value;
  }

  template<ParameterType type>
  constexpr
  typename std::enable_if<
    type == ParameterType::PARAMETER_BYTE_ARRAY, const uint8_t*>::type
  get() const
  {
    if (value_.type != ParameterType::PARAMETER_BYTE_ARRAY) {
        ecu_fatal_error("Invalid param type");
    }
    return value_.byte_array_value;
  }

  // The following get() variants allow the use of primitive types

  template<typename type>
  constexpr
  typename std::enable_if<std::is_same<type, bool>::value, const bool &>::type
  get() const
  {
    return get<ParameterType::PARAMETER_BOOL>();
  }

  template<typename type>
  constexpr
  typename std::enable_if<
    std::is_integral<type>::value && !std::is_same<type, bool>::value, const int64_t &>::type
  get() const
  {
    return get<ParameterType::PARAMETER_INTEGER>();
  }

  template<typename type>
  constexpr
  typename std::enable_if<std::is_floating_point<type>::value, const double &>::type
  get() const
  {
    return get<ParameterType::PARAMETER_DOUBLE>();
  }

  template<typename type>
  constexpr
  typename std::enable_if<
    std::is_convertible<
      type, const uint8_t*>::value, const uint8_t*>::type
  get() const
  {
    return get<ParameterType::PARAMETER_BYTE_ARRAY>();
  }

  static void to_string(char *out, const ParameterValue & type);

  _ParameterValue value_;
};

} // namespace Impl

} // namespace System
