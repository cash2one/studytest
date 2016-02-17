#include "parseHTML.h"
#include<libxml/xpath.h>
#include<libxml/parser.h>
#include<libxml/tree.h>
#include<libxml/xpathInternals.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

int parseHTML::setXpath(const vector<string>& xpath){
	_xpath = xpath;
	return 0;
}
int parseHTML::setPage(const string& page){
	_page_file = page;
	return 0;
}
int parseHTML::parse(map<string, map<string, string> >& ret){
    xmlDocPtr doc;
    xmlXPathContextPtr xpathCtx; 
	xmlNodeSetPtr nodeset;
    xmlNodePtr rootNode;
	
	xmlInitParser();
	doc = xmlReadFile(_page_file.c_str(), "UTF8", XML_PARSE_NOERROR|XML_PARSE_NOWARNING| XML_PARSE_NOBLANKS|XML_PARSE_OLD10);
	xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL) {
        fprintf(stderr,"Error: unable to create new XPath context\n");
        xmlFreeDoc(doc); 
        return(-1);
    }
	vector<string>::iterator iter;
	for(iter=_xpath.begin(); iter!=_xpath.end(); iter++){
		xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((const xmlChar*)((*iter).c_str()), xpathCtx);
		if(xpathObj == NULL) {
			fprintf(stderr,"Error: unable to evaluate xpath expression \"%s\"\n", (*iter).c_str());
			xmlXPathFreeContext(xpathCtx); 
			xmlFreeDoc(doc); 
			return(-1);
		}
		nodeset = xpathObj->nodesetval;
	   if (xmlXPathNodeSetIsEmpty(nodeset)) {  
			xmlXPathFreeObject(xpathObj);  
			printf("nodeset is empty\n");  
			return -1;  
		}   
		for (int i=0; i < nodeset->nodeNr; i++) {
			cout<<xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode, 1)<<endl;
		}
		xmlXPathFreeObject(xpathObj);
	}
    
    xmlXPathFreeContext(xpathCtx); 
	xmlFreeDoc(doc); 
	xmlCleanupParser();
	return 0;
}