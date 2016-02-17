#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <sys/time.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include "DmsService.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;


int main(int argc, char** argv) {
    boost::shared_ptr<TTransport> socket(new TSocket("127.0.0.1", 9090));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    DmsServiceClient client(protocol);
    transport->open();

    DocReturn result;
    string key("845543asdsddsfdsf544545456");
    client.get(result,key);

    cout<<"return code:"<<result.ret<<endl;
    cout<<"doc.docid:"<<result.doc.docid<<endl;
    cout<<"doc.finger:"<<result.doc.finger<<endl;
    cout<<"doc.title:"<<result.doc.title<<endl;
    cout<<"doc.title2:"<<result.doc.title2<<endl;
    cout<<"doc.category:"<<result.doc.category<<endl;
    cout<<"doc.subcategory:"<<result.doc.subcategory<<endl;
    cout<<"doc.keywords.size():"<<result.doc.keywords.size()<<endl;
    cout<<"doc.url:"<<result.doc.url<<endl;
    cout<<"doc.list_pic.url:"<<result.doc.list_pic.url<<endl;
    cout<<"doc.list_pic.sogou_pic_url:"<<result.doc.list_pic.sogou_pic_url<<endl;
    cout<<"doc.list_pic.width:"<<result.doc.list_pic.width<<endl;
    cout<<"doc.list_pic.height:"<<result.doc.list_pic.height<<endl;
    cout<<"doc.list_pic.size:"<<result.doc.list_pic.size<<endl;
    cout<<"doc.domain:"<<result.doc.domain<<endl;
    cout<<"doc.summary:"<<result.doc.summary<<endl;
    cout<<"doc.fetch_time:"<<result.doc.fetch_time<<endl;
    cout<<"doc.last_modify_time:"<<result.doc.last_modify_time<<endl;
    cout<<"doc.price:"<<result.doc.price<<endl;
    cout<<"doc.contact.name:"<<result.doc.contact.name<<endl;
    cout<<"doc.contact.text_phone:"<<result.doc.contact.text_phone<<endl;

    client.del_doc(key);

    transport->close();
    return 0;
}
