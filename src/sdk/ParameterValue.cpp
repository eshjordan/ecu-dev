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

#include "ParameterValue.hpp"
#include <stdio.h>
#include <string.h>


using System::Impl::ParameterType;
using System::Impl::ParameterValue;

void
System::Impl::parameter_type_to_string(char *out, const ParameterType type)
{
  switch (type) {
    case ParameterType::PARAMETER_NOT_SET:
      strcpy(out, "not set");
      return;
    case ParameterType::PARAMETER_BOOL:
      strcpy(out, "bool");
      return;
    case ParameterType::PARAMETER_INTEGER:
      strcpy(out, "integer");
      return;
    case ParameterType::PARAMETER_DOUBLE:
      strcpy(out, "double");
      return;
    case ParameterType::PARAMETER_BYTE_ARRAY:
      strcpy(out, "byte_array");
      return;
    default:
      strcpy(out, "unknown type");
      return;
  }
}

void
System::Impl::ParameterValue::to_string(char * out, const ParameterValue & value)
{
  switch (value.get_type()) {
    case ParameterType::PARAMETER_NOT_SET:
      strcpy(out, "not set");
      return;
    case ParameterType::PARAMETER_BOOL:
      strcpy(out, value.get<bool>() ? "true" : "false");
      return;
    case ParameterType::PARAMETER_INTEGER:
      sprintf(out, "%ld", value.get<int>());
      return;
    case ParameterType::PARAMETER_DOUBLE:
      sprintf(out, "%lf", value.get<double>());
      return;
    case ParameterType::PARAMETER_BYTE_ARRAY:
      for (uint8_t i = 0; i < value.value_.byte_array_count; i++)
      {
        char tmp[7] = {0};
        if (i == 0) {
          sprintf(out, "[0x%hx", value.value_.byte_array_value[i]);
        } else {
          sprintf(tmp, ", 0x%hx", value.value_.byte_array_value[i]);
          strcat(out, tmp);
        }
      }
      strcat(out, "]");
      return;
    default:
      strcpy(out, "unknown type");
      return;
  }
}

ParameterValue::ParameterValue()
{
  value_.type = ParameterType::PARAMETER_NOT_SET;
}

ParameterValue::ParameterValue(const ParameterValue & value)
{
  value_ = value.value_;
  switch (value.value_.type) {
    case PARAMETER_BOOL:
    case PARAMETER_INTEGER:
    case PARAMETER_DOUBLE:
    case PARAMETER_BYTE_ARRAY:
    case PARAMETER_STRING:
    case PARAMETER_NOT_SET:
      break;
    default:
      // TODO(jordan): use custom exception
      ecu_fatal_error("Unknown type: %d", value.value_.type);
  }
}

ParameterValue::ParameterValue(const bool bool_value)
{
  value_.bool_value = bool_value;
  value_.type = ParameterType::PARAMETER_BOOL;
}

ParameterValue::ParameterValue(const int int_value)
{
  value_.integer_value = int_value;
  value_.type = ParameterType::PARAMETER_INTEGER;
}

ParameterValue::ParameterValue(const int64_t int_value)
{
  value_.integer_value = int_value;
  value_.type = ParameterType::PARAMETER_INTEGER;
}

ParameterValue::ParameterValue(const float double_value)
{
  value_.double_value = static_cast<double>(double_value);
  value_.type = ParameterType::PARAMETER_DOUBLE;
}

ParameterValue::ParameterValue(const double double_value)
{
  value_.double_value = double_value;
  value_.type = ParameterType::PARAMETER_DOUBLE;
}

ParameterValue::ParameterValue(const char * string_value)
{
  memcpy(value_.byte_array_value, (uint8_t*)string_value, strlen(string_value));
  value_.type = ParameterType::PARAMETER_STRING;
}

ParameterValue::ParameterValue(const uint8_t *byte_array_value, uint8_t byte_array_count)
{
  memcpy(value_.byte_array_value, byte_array_value, byte_array_count);
  value_.byte_array_count = byte_array_count;
  value_.type = ParameterType::PARAMETER_BYTE_ARRAY;
}

ParameterType
ParameterValue::get_type() const
{
  return static_cast<ParameterType>(value_.type);
}

bool
ParameterValue::operator==(const ParameterValue & rhs) const
{
  if (this->get_type() != rhs.get_type())
  {
    return false;
  }

  switch (this->value_.type) {
    case PARAMETER_NOT_SET:
      return true;
    case PARAMETER_BOOL:
      return this->value_.bool_value == rhs.value_.bool_value;
    case PARAMETER_INTEGER:
      return this->value_.integer_value == rhs.value_.integer_value;
    case PARAMETER_DOUBLE:
      return this->value_.double_value == rhs.value_.double_value;
    case PARAMETER_BYTE_ARRAY:
      if (this->value_.byte_array_count != rhs.value_.byte_array_count) {return false;}
      return 0 == memcmp(this->value_.byte_array_value, rhs.value_.byte_array_value, this->value_.byte_array_count);
    default:
      return false;
  }
}

bool
ParameterValue::operator!=(const ParameterValue & rhs) const
{
  return !(*this == rhs);
}
