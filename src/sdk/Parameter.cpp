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

#include "Parameter.hpp"

#include <string.h>


using System::Impl::ParameterType;
using System::Impl::Parameter;

Parameter::Parameter()
: name_("")
{
}

Parameter::Parameter(const char *name)
: value_()
{
    strcpy(name_, name);
}

Parameter::Parameter(const char *name, const System::Impl::ParameterValue & value)
: Parameter(name)
{
    value_ = ParameterValue(value);
}

bool
Parameter::operator==(const Parameter & rhs) const
{
  return this->name_ == rhs.name_ && this->value_ == rhs.value_;
}

bool
Parameter::operator!=(const Parameter & rhs) const
{
  return !(*this == rhs);
}

ParameterType
Parameter::get_type() const
{
  return value_.get_type();
}

void
Parameter::get_type_name(char *out) const
{
  parameter_type_to_string(out, get_type());
  return;
}

void
Parameter::get_name(char *out) const
{
  strcpy(out, name_);
  return;
}

const System::Impl::ParameterValue &
Parameter::get_parameter_value() const
{
  return value_;
}

bool
Parameter::as_bool() const
{
  return get_value<ParameterType::PARAMETER_BOOL>();
}

int64_t
Parameter::as_int() const
{
  return get_value<ParameterType::PARAMETER_INTEGER>();
}

double
Parameter::as_double() const
{
  return get_value<ParameterType::PARAMETER_DOUBLE>();
}

const char *
Parameter::as_string() const
{
  return (const char*)get_value<ParameterType::PARAMETER_STRING>();
}

const uint8_t *
Parameter::as_byte_array() const
{
  return get_value<ParameterType::PARAMETER_BYTE_ARRAY>();
}

const bool *
Parameter::as_bool_array() const
{
    return (const bool*)get_value<ParameterType::PARAMETER_BYTE_ARRAY>();

}

const int64_t *
Parameter::as_integer_array() const
{
    return (const int64_t *)get_value<ParameterType::PARAMETER_BYTE_ARRAY>();
}

const double *
Parameter::as_double_array() const
{
    return (const double *)get_value<ParameterType::PARAMETER_BYTE_ARRAY>();
}

const char **
Parameter::as_string_array() const
{
    return (const char **)get_value<ParameterType::PARAMETER_BYTE_ARRAY>();
}

void
Parameter::value_to_string(char *out) const
{
  Parameter::to_string(out, *this);
  return;
}

void
Parameter::_to_json_dict_entry(char *out, const Parameter & param) const
{
  char tmp[64] = {0};
  strcpy(out, "\"");
  param.get_name(tmp);
  strcat(out, tmp);
  strcat(out, "\": {\"type\": \"");
  param.get_type_name(tmp);
  strcat(out, "\", \"value\": \"");
  param.value_to_string(tmp);
  strcat(out, "\"}");
}

void
Parameter::to_string(char *out, const Parameter & param) const
{
  char tmp[64] = {0};
  strcpy(out, "{\"name\": \"");
  param.get_name(tmp);
  strcat(out, tmp);
  strcat(out, "\", \"type\": \"");
  param.get_type_name(tmp);
  strcat(out, tmp);
  strcat(out, "\", \"value\": \"");
  param.value_to_string(tmp);
  strcat(out, tmp);
  strcat(out, "\"}");
}

void
Parameter::to_string(char *out, const Parameter * parameters, const uint8_t parameter_count) const
{
  char tmp[256] = {0};
  strcpy(out, "{");
  bool first = true;
  for (uint8_t i = 0; i < parameter_count; i++)
  {
    const auto & pv = parameters[i];
    if (first == false) {
      strcat(out, ", ");
    } else {
      first = false;
    }
    _to_json_dict_entry(tmp, pv);
    strcat(out, tmp);
  }
  strcat(out, "}");
}
