#include "httpdh.h"
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int sign(request_rec *r, char* msg, char** sign_str);
