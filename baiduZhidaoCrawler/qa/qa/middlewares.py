# -*- coding: utf-8 -*-

from common.config import http_proxy_host
from common.utils  import utils


class zhidaoMiddleware(object):
    def process_request(self, request, spider):
        local_ip = utils.get_local_ip()
        if local_ip in http_proxy_host and http_proxy_host[local_ip]:
            request.meta['proxy'] = http_proxy_host[local_ip]

