#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include "Calculator.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace wendao;

int main(int argc, char** argv) {
    boost::shared_ptr<TTransport> socket(new TSocket("127.0.0.1", 9090));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    CalculatorClient client(protocol);

    try {
        transport->open();
        client.ping();

        int32_t sum = client.add(1,1);
        printf("1+1=%d\n", sum);

        Work work;
        work.op = Operation::DIVIDE;
        work.num1 = 10;
        work.num2 = 0;
        try {
             client.calculate(work);
        }
        catch (InvalidOperation &io) {
             printf("InvalidOperation: %s\n", io.why.c_str());
        }

        work.op = Operation::SUBTRACT;
        work.num1 = 15;
        work.num2 = 10;
        int32_t diff = client.calculate(work);
        printf("15-10=%d\n", diff);


        transport->close();
    }
    catch (TException &tx) {
        printf("something wrong occur\n");
        printf("ERROR: %s\n", tx.what());
    }
	return 0;
}
