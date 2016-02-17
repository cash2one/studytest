#include "httpdh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zlib.h>
#include <apr_base64.h>


int compress_cookie_signature(request_rec *r, char* signature, char** dest);
int uncompress_cookie_signature(request_rec *r, char* base64signcompressed, char** uncompressed_signvalue);
