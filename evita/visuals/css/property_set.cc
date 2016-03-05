// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/css/property_set.h"

#include "base/logging.h"
#include "evita/visuals/css/color.h"
#include "evita/visuals/css/dimension.h"
#include "evita/visuals/css/percentage.h"
#include "evita/visuals/css/properties_forward.h"
#include "evita/visuals/css/unit.h"
#include "evita/visuals/css/value.h"
#include "evita/visuals/css/value_type.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// PropertySet::Iterator
//
PropertySet::Iterator::Iterator(const Iterator& other)
    : Iterator(other.property_set_, other.index_) {}

PropertySet::Iterator::Iterator(const PropertySet* property_set, size_t index)
    : index_(index), property_set_(property_set) {}

PropertySet::Iterator::~Iterator() {}

PropertySet::Iterator& PropertySet::Iterator::operator=(const Iterator& other) {
  DCHECK_EQ(property_set_, other.property_set_);
  index_ = other.index_;
  return *this;
}

PropertySet::Iterator::value_type PropertySet::Iterator::operator*() const {
  return std::make_pair(property_id(), DecodeValue());
}

PropertySet::Iterator& PropertySet::Iterator::operator++() {
  DCHECK_LT(index_, property_set_->words_.size());
  index_ += SizeOfChunk();
  return *this;
}

bool PropertySet::Iterator::operator==(const Iterator& other) const {
  DCHECK_EQ(property_set_, other.property_set_);
  return index_ == other.index_;
}

bool PropertySet::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

uint32_t PropertySet::Iterator::data() const {
  DCHECK_LT(index_, property_set_->words_.size());
  return property_set_->words_[index_].bits.data;
}

bool PropertySet::Iterator::is_small() const {
  return (data() & 1) == 1;
}

PropertyId PropertySet::Iterator::property_id() const {
  DCHECK_LT(index_, property_set_->words_.size());
  return property_set_->words_[index_].bits.property_id;
}

ValueType PropertySet::Iterator::type() const {
  DCHECK_LT(index_, property_set_->words_.size());
  return property_set_->words_[index_].bits.type;
}

Color PropertySet::Iterator::DecodeColor() const {
  const auto value = DecodeUint32();
  const auto red = static_cast<float>((value >> 24) & 0xFF) / 255;
  const auto green = static_cast<float>((value >> 16) & 0xFF) / 255;
  const auto blue = static_cast<float>((value >> 8) & 0xFF) / 255;
  const auto alpha = static_cast<float>(value & 0xFF) / 255;
  return Color(red, green, blue, alpha);
}

Dimension PropertySet::Iterator::DecodeDimension() const {
  const auto kUnitBits = 5;
  const auto kUnitMask = (1 << kUnitBits) - 1;
  const auto kSmallValueBits = kDataBits - kUnitBits - 1;
  const auto unit = static_cast<Unit>((data() >> 1) & kUnitMask);
  if (!is_small())
    return Dimension(DecodeFloat(), unit);
  const auto value = static_cast<int>(data()) >> (kUnitBits + 1);
  if (value < (1 << (kSmallValueBits - 1)))
    return Dimension(static_cast<float>(value) / 1000, unit);
  return Dimension(static_cast<float>(value - (1 << kSmallValueBits)) / 1000,
                   unit);
}

float PropertySet::Iterator::DecodeFloat() const {
  DCHECK_LT(index_ + 1, property_set_->words_.size());
  return property_set_->words_[index_ + 1].f32;
}

int PropertySet::Iterator::DecodeInteger() const {
  if (!is_small())
    return DecodeUint32();
  return DecodeSmallInteger();
}

Keyword PropertySet::Iterator::DecodeKeyword() const {
  return static_cast<Keyword>(data());
}

float PropertySet::Iterator::DecodeNumber() const {
  if (!is_small())
    return DecodeFloat();
  return static_cast<float>(DecodeSmallInteger()) / 1000;
}

Percentage PropertySet::Iterator::DecodePercentage() const {
  return Percentage(DecodeNumber());
}

int PropertySet::Iterator::DecodeSmallInteger() const {
  DCHECK(is_small()) << type();
  const auto value = static_cast<int>(data()) >> 1;
  const auto kSmallValueBits = kDataBits - 1;
  if (value < (1 << (kSmallValueBits - 1)))
    return value;
  return value - (1 << kSmallValueBits);
}

uint32_t PropertySet::Iterator::DecodeUint32() const {
  DCHECK_LT(index_ + 1, property_set_->words_.size());
  return property_set_->words_[index_ + 1].u32;
}

Value PropertySet::Iterator::DecodeValue() const {
  switch (type()) {
    case ValueType::Color:
      return Value(DecodeColor());
    case ValueType::Dimension:
      return Value(DecodeDimension());
    case ValueType::Integer:
      return Value(DecodeInteger());
    case ValueType::Number:
      return Value(DecodeNumber());
    case ValueType::Percentage:
      return Value(DecodePercentage());
    case ValueType::Keyword:
      return Value(DecodeKeyword());
  }
  NOTREACHED() << type();
  return Value();
}

size_t PropertySet::Iterator::SizeOfChunk() const {
  switch (type()) {
    case ValueType::Color:
      return 2;
    case ValueType::Dimension:
    case ValueType::Integer:
    case ValueType::Number:
    case ValueType::Percentage:
      return is_small() ? 1 : 2;
    case ValueType::Keyword:
      return 1;
  }
  NOTREACHED() << type();
  return 0;
}

//////////////////////////////////////////////////////////////////////
//
// PropertySet
//
PropertySet::PropertySet(const PropertySet& other) : words_(other.words_) {}

PropertySet::PropertySet(PropertySet&& other)
    : words_(std::move(other.words_)) {}

PropertySet::PropertySet() {}
PropertySet::~PropertySet() {}

PropertySet& PropertySet::operator=(const PropertySet& other) {
  words_ = other.words_;
  return *this;
}

PropertySet& PropertySet::operator=(PropertySet&& other) {
  words_ = std::move(other.words_);
  return *this;
}

bool PropertySet::operator==(const PropertySet& other) const {
  if (this == &other)
    return true;
  if (words_.size() != other.words_.size())
    return false;
  for (const auto& pair : *this) {
    const auto& it = std::find(other.begin(), other.end(), pair);
    if (it == other.end())
      return false;
  }
  return true;
}

bool PropertySet::operator!=(const PropertySet& other) const {
  return !operator==(other);
}

PropertySet::Iterator PropertySet::begin() const {
  return Iterator(this, 0);
}

PropertySet::Iterator PropertySet::end() const {
  return Iterator(this, words_.size());
}

Value PropertySet::ValueOf(PropertyId property_id) const {
  for (const auto& pair : *this) {
    if (pair.first == property_id)
      return std::move(pair.second);
  }
  return Value();
}

std::ostream& operator<<(std::ostream& ostream,
                         const PropertySet& property_set) {
  ostream << '{';
  auto delimiter = "";
  for (const auto& pair : property_set) {
    ostream << delimiter << pair.first << ':' << pair.second;
    delimiter = ", ";
  }
  return ostream << '}';
}

}  // namespace css
}  // namespace visuals