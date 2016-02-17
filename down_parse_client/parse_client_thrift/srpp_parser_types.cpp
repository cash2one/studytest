/**
 * Autogenerated by Thrift Compiler (0.9.1)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "srpp_parser_types.h"

#include <algorithm>



const char* parse_arg::ascii_fingerprint = "194A357BC9EE908DC4763D0A5147760A";
const uint8_t parse_arg::binary_fingerprint[16] = {0x19,0x4A,0x35,0x7B,0xC9,0xEE,0x90,0x8D,0xC4,0x76,0x3D,0x0A,0x51,0x47,0x76,0x0A};

uint32_t parse_arg::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->html_content);
          this->__isset.html_content = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->extra);
          this->__isset.extra = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->engine);
          this->__isset.engine = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 4:
        if (ftype == ::apache::thrift::protocol::T_I32) {
          xfer += iprot->readI32(this->ret_action);
          this->__isset.ret_action = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t parse_arg::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("parse_arg");

  xfer += oprot->writeFieldBegin("html_content", ::apache::thrift::protocol::T_STRING, 1);
  xfer += oprot->writeString(this->html_content);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("extra", ::apache::thrift::protocol::T_STRING, 2);
  xfer += oprot->writeString(this->extra);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("engine", ::apache::thrift::protocol::T_STRING, 3);
  xfer += oprot->writeString(this->engine);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("ret_action", ::apache::thrift::protocol::T_I32, 4);
  xfer += oprot->writeI32(this->ret_action);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(parse_arg &a, parse_arg &b) {
  using ::std::swap;
  swap(a.html_content, b.html_content);
  swap(a.extra, b.extra);
  swap(a.engine, b.engine);
  swap(a.ret_action, b.ret_action);
  swap(a.__isset, b.__isset);
}


