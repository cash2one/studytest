#ifndef _DOWN_PARSE_CLIENT_SERVICEHANDLER_H_
#define _DOWN_PARSE_CLIENT_SERVICEHANDLER_H_

#include "down_parse_client_service.h"
#include <stdint.h>


class down_parse_client_serviceHandler : virtual public down_parse_client_serviceIf {
public:
	down_parse_client_serviceHandler(std::string cf): conf_file(cf){}
	int32_t ping();
	int32_t start(const int32_t task_id, const int32_t is_retry);
	
private:
	std::string conf_file;
};

#endif

