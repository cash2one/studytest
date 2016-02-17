/**
 * Autogenerated by Thrift Compiler (0.9.1)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef grab_page_H
#define grab_page_H

#include <thrift/TDispatchProcessor.h>
#include "server/page_download_types.h"

namespace page_download {

class grab_pageIf {
 public:
  virtual ~grab_pageIf() {}
  virtual int32_t ping() = 0;
  virtual int32_t start_grab(const crawler_arg& arg) = 0;
};

class grab_pageIfFactory {
 public:
  typedef grab_pageIf Handler;

  virtual ~grab_pageIfFactory() {}

  virtual grab_pageIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(grab_pageIf* /* handler */) = 0;
};

class grab_pageIfSingletonFactory : virtual public grab_pageIfFactory {
 public:
  grab_pageIfSingletonFactory(const boost::shared_ptr<grab_pageIf>& iface) : iface_(iface) {}
  virtual ~grab_pageIfSingletonFactory() {}

  virtual grab_pageIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(grab_pageIf* /* handler */) {}

 protected:
  boost::shared_ptr<grab_pageIf> iface_;
};

class grab_pageNull : virtual public grab_pageIf {
 public:
  virtual ~grab_pageNull() {}
  int32_t ping() {
    int32_t _return = 0;
    return _return;
  }
  int32_t start_grab(const crawler_arg& /* arg */) {
    int32_t _return = 0;
    return _return;
  }
};


class grab_page_ping_args {
 public:

  grab_page_ping_args() {
  }

  virtual ~grab_page_ping_args() throw() {}


  bool operator == (const grab_page_ping_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const grab_page_ping_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const grab_page_ping_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class grab_page_ping_pargs {
 public:


  virtual ~grab_page_ping_pargs() throw() {}


  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _grab_page_ping_result__isset {
  _grab_page_ping_result__isset() : success(false) {}
  bool success;
} _grab_page_ping_result__isset;

class grab_page_ping_result {
 public:

  grab_page_ping_result() : success(0) {
  }

  virtual ~grab_page_ping_result() throw() {}

  int32_t success;

  _grab_page_ping_result__isset __isset;

  void __set_success(const int32_t val) {
    success = val;
  }

  bool operator == (const grab_page_ping_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const grab_page_ping_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const grab_page_ping_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _grab_page_ping_presult__isset {
  _grab_page_ping_presult__isset() : success(false) {}
  bool success;
} _grab_page_ping_presult__isset;

class grab_page_ping_presult {
 public:


  virtual ~grab_page_ping_presult() throw() {}

  int32_t* success;

  _grab_page_ping_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _grab_page_start_grab_args__isset {
  _grab_page_start_grab_args__isset() : arg(false) {}
  bool arg;
} _grab_page_start_grab_args__isset;

class grab_page_start_grab_args {
 public:

  grab_page_start_grab_args() {
  }

  virtual ~grab_page_start_grab_args() throw() {}

  crawler_arg arg;

  _grab_page_start_grab_args__isset __isset;

  void __set_arg(const crawler_arg& val) {
    arg = val;
  }

  bool operator == (const grab_page_start_grab_args & rhs) const
  {
    if (!(arg == rhs.arg))
      return false;
    return true;
  }
  bool operator != (const grab_page_start_grab_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const grab_page_start_grab_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class grab_page_start_grab_pargs {
 public:


  virtual ~grab_page_start_grab_pargs() throw() {}

  const crawler_arg* arg;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _grab_page_start_grab_result__isset {
  _grab_page_start_grab_result__isset() : success(false) {}
  bool success;
} _grab_page_start_grab_result__isset;

class grab_page_start_grab_result {
 public:

  grab_page_start_grab_result() : success(0) {
  }

  virtual ~grab_page_start_grab_result() throw() {}

  int32_t success;

  _grab_page_start_grab_result__isset __isset;

  void __set_success(const int32_t val) {
    success = val;
  }

  bool operator == (const grab_page_start_grab_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const grab_page_start_grab_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const grab_page_start_grab_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _grab_page_start_grab_presult__isset {
  _grab_page_start_grab_presult__isset() : success(false) {}
  bool success;
} _grab_page_start_grab_presult__isset;

class grab_page_start_grab_presult {
 public:


  virtual ~grab_page_start_grab_presult() throw() {}

  int32_t* success;

  _grab_page_start_grab_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class grab_pageClient : virtual public grab_pageIf {
 public:
  grab_pageClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) :
    piprot_(prot),
    poprot_(prot) {
    iprot_ = prot.get();
    oprot_ = prot.get();
  }
  grab_pageClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) :
    piprot_(iprot),
    poprot_(oprot) {
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  int32_t ping();
  void send_ping();
  int32_t recv_ping();
  int32_t start_grab(const crawler_arg& arg);
  void send_start_grab(const crawler_arg& arg);
  int32_t recv_start_grab();
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class grab_pageProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  boost::shared_ptr<grab_pageIf> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (grab_pageProcessor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_ping(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_start_grab(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  grab_pageProcessor(boost::shared_ptr<grab_pageIf> iface) :
    iface_(iface) {
    processMap_["ping"] = &grab_pageProcessor::process_ping;
    processMap_["start_grab"] = &grab_pageProcessor::process_start_grab;
  }

  virtual ~grab_pageProcessor() {}
};

class grab_pageProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  grab_pageProcessorFactory(const ::boost::shared_ptr< grab_pageIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::boost::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::boost::shared_ptr< grab_pageIfFactory > handlerFactory_;
};

class grab_pageMultiface : virtual public grab_pageIf {
 public:
  grab_pageMultiface(std::vector<boost::shared_ptr<grab_pageIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~grab_pageMultiface() {}
 protected:
  std::vector<boost::shared_ptr<grab_pageIf> > ifaces_;
  grab_pageMultiface() {}
  void add(boost::shared_ptr<grab_pageIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  int32_t ping() {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->ping();
    }
    return ifaces_[i]->ping();
  }

  int32_t start_grab(const crawler_arg& arg) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->start_grab(arg);
    }
    return ifaces_[i]->start_grab(arg);
  }

};

} // namespace

#endif
