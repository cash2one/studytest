#ifndef __VERIFY_H__
#define __VERIFY_H__

#ifdef _cplusplus
extern "C"
{
#endif

int verify_saml_response(const char* samlresponsebuf, const char* certfiledir, const char* schemapath);

#ifdef _cplusplus
}
#endif

#endif // __VERIFY_H__

