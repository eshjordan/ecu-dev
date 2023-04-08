// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#include "Error.hpp"
#include "ParameterValue.hpp"

namespace System
{

namespace Impl
{

class Parameter;

namespace detail
{

// This helper function is required because you cannot do specialization on a
// class method, so instead we specialize this template function and call it
// from the unspecialized, but dependent, class method.
template<typename T>
auto
get_value_helper(const Parameter * parameter);

}  // namespace detail

/// Structure to store an arbitrary parameter with templated get/set methods.
class Parameter
{
public:
  /// Construct with an empty name and a parameter value of type rclcpp::PARAMETER_NOT_SET.
  Parameter();

  /// Construct with given name and a parameter value of type rclcpp::PARAMETER_NOT_SET.
  explicit Parameter(const char *name);

  /// Construct with given name and given parameter value.
  Parameter(const char *name, const ParameterValue & value);

  /// Construct with given name and given parameter value.
  template<typename ValueTypeT>
  Parameter(const char *name, ValueTypeT value)
  : Parameter(name, ParameterValue(value))
  {}

  friend class Channel;

  friend class ParameterList;


//   explicit Parameter(const rclcpp::node_interfaces::ParameterInfo & parameter_info);

  /// Equal operator.
  bool
  operator==(const Parameter & rhs) const;

  /// Not equal operator.
  bool
  operator!=(const Parameter & rhs) const;

  /// Get the type of the parameter
  ParameterType
  get_type() const;

  /// Get the type name of the parameter
  void
  get_type_name(char *out) const;

  /// Get the name of the parameter
  void
  get_name(char *out) const;

  /// Get the internal storage for the parameter value.
  const ParameterValue &
  get_parameter_value() const;

  /// Get value of parameter using rclcpp::ParameterType as template argument.
  /**
   * \throws rclcpp::exceptions::InvalidParameterTypeException if the type doesn't match
   */
  template<ParameterType ParamT>
  decltype(auto)
  get_value() const
  {
    return value_.get<ParamT>();
  }

  /// Get value of parameter using c++ types as template argument.
  template<typename T>
  decltype(auto)
  get_value() const;

  /// Get value of parameter as boolean.
  /**
   * \throws rclcpp::ParameterTypeException if the type doesn't match
   */
  bool
  as_bool() const;

  /// Get value of parameter as integer.
  /**
   * \throws rclcpp::ParameterTypeException if the type doesn't match
   */
  int64_t
  as_int() const;

  /// Get value of parameter as double.
  /**
   * \throws rclcpp::ParameterTypeException if the type doesn't match
   */
  double
  as_double() const;

  /// Get value of parameter as string.
  /**
   * \throws rclcpp::ParameterTypeException if the type doesn't match
   */
  const char *
  as_string() const;

  /// Get value of parameter as byte array (vector<uint8_t>).
  /**
   * \throws rclcpp::ParameterTypeException if the type doesn't match
   */
  const uint8_t *
  as_byte_array() const;

  /// Get value of parameter as bool array (vector<bool>).
  /**
   * \throws rclcpp::ParameterTypeException if the type doesn't match
   */
  const bool *
  as_bool_array() const;

  /// Get value of parameter as integer array (vector<int64_t>).
  /**
   * \throws rclcpp::ParameterTypeException if the type doesn't match
   */
  const int64_t *
  as_integer_array() const;

  /// Get value of parameter as double array (vector<double>).
  /**
   * \throws rclcpp::ParameterTypeException if the type doesn't match
   */
  const double *
  as_double_array() const;

  /// Get value of parameter as string array (vector<std::string>).
  /**
   * \throws rclcpp::ParameterTypeException if the type doesn't match
   */
  const char **
  as_string_array() const;

  /// Convert a parameter message in a Parameter class object.
  static Parameter
  from_parameter_msg(const Parameter & parameter);

  /// Convert the class in a parameter message.
  Parameter
  to_parameter_msg() const;

  /// Get value of parameter as a string.
  void
  value_to_string(char *) const;

  /// Return a json encoded version of the parameter intended for a dict.
  void
  _to_json_dict_entry(char *out, const Parameter & param) const;

  /// Return a json encoded version of the parameter intended for a list.
  void
  to_string(char *out, const Parameter & param) const;

  /// Return a json encoded version of a vector of parameters, as a string.
  void
  to_string(char *out, const Parameter * parameters, const uint8_t parameter_count) const;

private:
  char name_[255];
  ParameterValue value_;
};

namespace detail
{

template<typename T>
auto
get_value_helper(const Parameter * parameter)
{
  return parameter->get_parameter_value().get<T>();
}

// // Specialization allowing Parameter::get() to return a const ref to the parameter value object.
// template<>
// inline
// auto
// get_value_helper<ParameterValue>(const Parameter * parameter)
// {
//   return parameter->get_parameter_value();
// }

// Specialization allowing Parameter::get() to return a const ref to the parameter itself.
template<>
inline
auto
get_value_helper<Parameter>(const Parameter * parameter)
{
  // Use this lambda to ensure it's a const reference being returned (and not a copy).
  auto type_enforcing_lambda =
    [&parameter]() -> const Parameter & {
      return *parameter;
    };
  return type_enforcing_lambda();
}

}  // namespace detail

/// \cond
template<typename T>
decltype(auto)
Parameter::get_value() const
{
  // use the helper to specialize for the ParameterValue and Parameter cases.
  return detail::get_value_helper<T>(this);
}
/// \endcond

}  // namespace Impl

}  // namespace System
