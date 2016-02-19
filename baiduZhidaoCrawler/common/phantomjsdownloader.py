# -*- coding: utf-8 -*-

import os
from time import time
from cStringIO import StringIO
from urlparse import urldefrag
from zope.interface import implements
from twisted.internet import defer, reactor, protocol
from twisted.web.http_headers import Headers as TxHeaders
from twisted.web.iweb import IBodyProducer
from twisted.internet.error import TimeoutError
from twisted.web.http import PotentialDataLoss
from scrapy.xlib.tx import Agent, ProxyAgent, ResponseDone, HTTPConnectionPool, TCP4ClientEndpoint
from scrapy.http import Headers
from scrapy.responsetypes import responsetypes
from scrapy.core.downloader.webclient import _parse
from scrapy.utils.misc import load_object
from scrapy.http import HtmlResponse
from twisted.internet import utils
from config import http_proxy_host
from utils import  utils as CommUtils

class phantomjsDownloader(object):

    def __init__(self, agent=None):
        self._agent = agent

    def download(self, request):
        begintime = time()
        d = self._download(request)
        d.addCallback(self.parseData, request, begintime)
        return d

    def _download(self, request):
        proxy = ""
        local_ip = CommUtils.get_local_ip()
        if local_ip in http_proxy_host and http_proxy_host[local_ip]:
            proxy = http_proxy_host[local_ip]
        curdir = os.path.dirname(__file__)
        cmd = curdir + "/phantomjsdownloader.sh"
        js = curdir + "/phantomjsdownloader.js"
        d = utils.getProcessOutput(cmd, args=(js,request.url, proxy), reactor=reactor)
        def getOutput(result):
            return result
        d.addCallback(getOutput)
        return d

    def parseData(self, htmldoc, request, begintime):
        request.meta['download_latency'] = time() - begintime
        return HtmlResponse(request.url, body=htmldoc, request=request)
