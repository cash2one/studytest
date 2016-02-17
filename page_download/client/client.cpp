#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include "grab_page.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace  ::page_download;

int main(int argc, char** argv) {
    boost::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    grab_pageClient client(protocol);

    transport->open();
	printf("ping()\n");
    client.ping();
        
	crawler_arg test;
	test.task_id = 1000001;
	test.start = 34;
	test.limit = 100;
	client.start_grab(test);

    transport->close();
    return 0;
}
