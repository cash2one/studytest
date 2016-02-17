#ifndef _cplusplus
#define _cplusplus
#include "verify.h"
#endif

#include <saml/saml.h> // for opensaml

#include <xsec/framework/XSECException.hpp>
#include <xsec/enc/XSECCryptoException.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoX509.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoKeyRSA.hpp>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace saml;
using namespace std;


extern "C" int verify_saml_response(const char* samlresponsebuf, const char* certfiledir, const char* schemapath)
{ 
   string  strXml;

   SAMLConfig& conf1 = SAMLConfig::getConfig();
   conf1.schema_dir = schemapath; 
   if (!conf1.init())  {
       cerr << "unable to initialize SAML runtime" << endl;
	}

   stringstream ssXml(samlresponsebuf);
   SAMLSignedObject* pSamlSignedObj = NULL;

   int ret = 0;

   try
   {
       // 由字符串流构造一个SAMLResponse对象
       pSamlSignedObj = new SAMLResponse(ssXml);

       string certbuf,line;

       auto_ptr<OpenSSLCryptoX509> x509(new OpenSSLCryptoX509());

       ifstream infile(certfiledir);

       while (!getline(infile,line).fail()){
           if (line.find("CERTIFICATE") == string::npos) {
               certbuf+=line + '\n';
			}
		}

       x509->loadX509Base64Bin(certbuf.data(), certbuf.length());

       Iterator<SAMLAssertion*> i = dynamic_cast<SAMLResponse*>(pSamlSignedObj)->getAssertions();
       if (i.hasNext())
       {
           SAMLAssertion* a = i.next();
           if (a->isSigned()){
               a->verify(*x509);
			}
       }

       pSamlSignedObj->verify(*x509);

       cout << "Success!" << endl;

       ret = 1;
   }
   catch(SAMLException& e)
   {
       cerr << "caught a SAML exception: " << e << endl;
       ret = 0;
   }

   delete pSamlSignedObj;
   pSamlSignedObj = NULL;

   conf1.term();
   return ret;
}

