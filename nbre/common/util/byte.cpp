// Copyright (C) 2018 go-nebulas authors
//
//
// This file is part of the go-nebulas library.
//
// the go-nebulas library is free software: you can redistribute it and/or
// modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// the go-nebulas library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with the go-nebulas library.  If not, see
// <http://www.gnu.org/licenses/>.
//
#include "common/util/byte.h"
#include "common/util/base58.h"
#include "common/util/base64.h"
#include <iomanip>
#include <sstream>

namespace neb {
namespace util {

namespace internal {
std::string convert_byte_to_hex(const byte_t *buf, size_t len) {
  if (!buf)
    return "";
  std::stringstream s;
  for (size_t i = 0; i < len; i++) {
    s << std::hex << std::setw(2) << std::setfill('0')
      << static_cast<int>(buf[i]);
  }
  return s.str();
}

std::string convert_byte_to_base58(const byte_t *buf, size_t len) {
  return ::neb::encode_base58(buf, buf + len);
}

std::string convert_byte_to_base64(const byte_t *buf) {
  unsigned char* b = (unsigned char*)buf;
  std::string input(reinterpret_cast<char*>(b));
  std::string output;
  ::neb::encode_base64(input, output);
  return output; 
}

bool convert_hex_to_bytes(const std::string &s, byte_t *buf, size_t &len) {
  auto char2int = [](char input) {
    if (input >= '0' && input <= '9')
      return input - '0';
    if (input >= 'A' && input <= 'F')
      return input - 'A' + 10;
    if (input >= 'a' && input <= 'f')
      return input - 'a' + 10;
    throw std::invalid_argument("Invalid input string");
  };

  try {
    size_t i = 0;
    while (i * 2 < s.size() && i * 2 + 1 < s.size()) {
      if (buf) {
        buf[i] = (char2int(s[i * 2]) << 4) + char2int(s[i * 2 + 1]);
      }
      i++;
    }
    len = i;
  } catch (std::exception &e) {
    return false;
  }
  return true;
}

void convert_string_to_byte(unsigned char *s, byte_t *buf, size_t size) {
    if (buf) {
      memcpy(buf, s, size);
    }
}

bool convert_base58_to_bytes(const std::string &s, byte_t *buf, size_t &len) {
  std::vector<unsigned char> ret;
  bool rv = ::neb::decode_base58(s, ret);
  if (rv) {
    len = ret.size();
    convert_string_to_byte(&ret[0], buf, len);
  }

  return rv;
}

bool convert_base64_to_bytes(const std::string &s, byte_t *buf, size_t &len){
  std::string output_string;
  bool rv = ::neb::decode_base64(s, output_string);

  if (rv) {
    len = output_string.size();
    if(nullptr != buf) {
      char* char_string = const_cast<char*>(output_string.c_str());
      convert_string_to_byte((unsigned char*)char_string, buf, len);
    }
  } else {
    throw std::invalid_argument("Invalid decode_base64.");
  }

  return rv;
}
} 
bytes::bytes() : m_value(nullptr), m_size(0) {}

bytes::bytes(size_t len)
    : m_value(std::unique_ptr<byte_t[]>(new byte_t[len])), m_size(len) {}

bytes::bytes(const bytes &v) : bytes(v.size()) {
  memcpy(m_value.get(), v.m_value.get(), v.size());
  m_size = v.size();
}

bytes::bytes(bytes &&v) : m_value(std::move(v.m_value)), m_size(v.size()) {}

bytes::bytes(std::initializer_list<byte_t> l) {
  if (l.size() > 0) {
    m_value = std::unique_ptr<byte_t[]>(new byte_t[l.size()]);
    std::copy(l.begin(), l.end(), m_value.get());
  }
  m_size = l.size();
}
bytes::bytes(const byte_t *buf, size_t buf_len) {
  m_size = buf_len;
  if (buf_len > 0) {
    m_value = std::unique_ptr<byte_t[]>(new byte_t[buf_len]);
    memcpy(m_value.get(), buf, buf_len);
  }
}
bytes &bytes::operator=(const bytes &v) {
  if (&v == this)
    return *this;
  if (v.value()) {
    m_value = std::unique_ptr<byte_t[]>(new byte_t[v.size()]);
    memcpy(m_value.get(), v.m_value.get(), v.size());
  }
  m_size = v.size();
  return *this;
}

bytes &bytes::operator=(bytes &&v) {
  m_value = std::move(v.m_value);
  m_size = v.m_size;
  return *this;
}

bool bytes::operator==(const bytes &v) const {
  if (v.size() != size())
    return false;
  return memcmp(v.value(), value(), size()) == 0;
}
bool bytes::operator!=(const bytes &v) const { return !operator==(v); }

std::string bytes::to_base58() const {
  return internal::convert_byte_to_base58(value(), size());
}

std::string bytes::to_base64() const {
  return internal::convert_byte_to_base64(const_cast<neb::byte_t*>(value()));
}

std::string bytes::to_hex() const {
  return internal::convert_byte_to_hex(value(), size());
}

bytes bytes::from_base58(const std::string &t) {
  size_t len = 0;
  bool succ = internal::convert_base58_to_bytes(t, nullptr, len);
  if (!succ)
    throw std::invalid_argument("invalid base58 string for from_base58");
  bytes ret(len);
  internal::convert_base58_to_bytes(t, ret.value(), len);
  return ret;
}

bytes bytes::from_base64(const std::string &t) {
  size_t len = 0;
  bool succeed = internal::convert_base64_to_bytes(t, nullptr, len);
  if (!succeed) {
    throw std::invalid_argument("invalid base64 string for from_base64");
  }

  bytes ret(len);
  internal::convert_base64_to_bytes(t, ret.value(), len);

  return ret;
}

bytes bytes::from_hex(const std::string &t) {
  size_t len = 0;
  bool succ = internal::convert_hex_to_bytes(t, nullptr, len);
  if (!succ) {
    throw std::invalid_argument("invalid hex string for from_hex");
  }

  bytes ret(len);
  bool succ_ret = internal::convert_hex_to_bytes(t, ret.value(), len);
  if (!succ_ret) {
    throw std::invalid_argument("invalid hex string for from_hex");
  }

  return ret;
}
}
}