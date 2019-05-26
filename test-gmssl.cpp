/*
update time : 
author : create by helloworld
need:   GmSSL 2.0 - OpenSSL 1.1.0d  26 Jan 2017
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/engine.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/rand.h>
#include <openssl/sm2.h>



#define ERROR(...) \
do{ \
    fprintf(stderr, "[ERROR  ]%s %s(Line %d): ",__FILE__,__FUNCTION__,__LINE__); \
    fprintf(stderr, __VA_ARGS__); \
    printf("\n"); \
}while(0) ;

#define INFO(...) \
do{ \
    fprintf(stdout, "[INFO  ]%s %s(Line %d): ",__FILE__,__FUNCTION__,__LINE__); \
    fprintf(stdout, __VA_ARGS__); \
    printf("\n"); \
}while(0) ;


void print_arr(const char *string, unsigned char*data, int size)
{
	int i;

	printf("---------------------------------------------\n");
	printf("%s len=%d\n", string,size);
	for(i=0; i<size; i++)
	{
		printf("%02x ", data[i]);
		if((i%16) == 15)
			printf("\n");
	}
	if(size%16)
		printf("\n");
	printf("---------------------------------------------\n");
}


BIO *bio_err ;
BIO *bio_stdout ;

 char g_pubkey[] = ""
"-----BEGIN PUBLIC KEY-----\r\n"
"MFkwEwYHKoZIzj0CAQYIKoEcz1UBgi0DQgAEu4kSnox0T5SND9KGGlpTvcQ5hiz0\r\n"
"Wm7kigVRTEc0+5wrSoyvnZYuYKLFSSzK2ZbMH+etX7UeRSiPplD63fChNw==\r\n"
"-----END PUBLIC KEY-----\r\n"
"";

 char g_prikey[] = ""
"-----BEGIN PRIVATE KEY-----\r\n"
"MIGHAgEAMBMGByqGSM49AgEGCCqBHM9VAYItBG0wawIBAQQgQnx5UCnYOJv558pc\r\n"
"rU9mgorePGbbF8/JxsR9N8LrKAmhRANCAAS7iRKejHRPlI0P0oYaWlO9xDmGLPRa\r\n"
"buSKBVFMRzT7nCtKjK+dli5gosVJLMrZlswf561ftR5FKI+mUPrd8KE3\r\n"
"-----END PRIVATE KEY-----\r\n"
"";





static EVP_PKEY *getPEMPublicKeyFromBuf(const char* buf,size_t len)
{
	EVP_PKEY *pkey = NULL;
	BIO* key = NULL;
	key = BIO_new(BIO_s_mem());
	BIO_write(key,buf,len);
	BIO_seek(key,0);
	pkey = PEM_read_bio_PUBKEY(key, NULL, 0, NULL);
	BIO_free(key);
	return pkey;
}

static EVP_PKEY *getPEMPrivateKeyFromBuf(const char* buf,size_t len)
{
	EVP_PKEY *pkey = NULL;
	BIO* key = NULL;
	key = BIO_new(BIO_s_mem());
	BIO_write(key,buf,len);
	BIO_seek(key,0);
	pkey = PEM_read_bio_PrivateKey(key, NULL, 0, NULL);
	BIO_free(key);
	return pkey;
}

static int test_sm2_encrypt(EVP_PKEY *pkey, unsigned char *cbuf, size_t buflen,unsigned char * out ,size_t* poutlen)
{
	int ret = 0;

	if(SM2_encrypt_with_recommended(cbuf, buflen, out, poutlen, EVP_PKEY_get0_EC_KEY(pkey)) != 1){
		ERR_print_errors_fp(stderr);
		printf("SM2_encrypt_with_recommended failed \n");
		ret = -1; goto end;
	}
	size_t i;
	printf("ciphertext (%d bytes) = \n", *poutlen);
	for (i = 0; i < *poutlen; i++) {
		printf("%02X ", out[i]);
		if(i % 16 == 15){
			printf("\n");
		}
	}
	printf("\n");


	ret = 1;
end:

	return ret;
}

static int test_sm2_decrypt(EVP_PKEY *pkey, unsigned char *cbuf, size_t cbuflen)
{
	int ret = 0;
	unsigned char mbuf[1024];
	size_t mbuflen = sizeof(mbuf);

	memset(mbuf, 0, sizeof(mbuf));
	if (!SM2_decrypt_with_recommended(cbuf, cbuflen,mbuf, &mbuflen,  EVP_PKEY_get0_EC_KEY(pkey))) {
		fprintf(stderr, "error: %s %d\n", __FILE__, __LINE__);
		ERR_print_errors_fp(stderr);
		goto end;
	}

	printf("decrypted message = %s\n", mbuf);
	

	ret = 1;
end:
	
	return ret;
}


static int test_evp_pkey_sign(EVP_PKEY *pkey, unsigned char *cbuf, size_t cbuflen,unsigned char * out, size_t *poutlen)
{
	int ret = 0;
	EVP_PKEY_CTX *pkctx = NULL;

	if (!(pkctx = EVP_PKEY_CTX_new(pkey, NULL))) {
		fprintf(stderr, "error: %s %d\n", __FILE__, __LINE__);
		goto end;
	}

	if (!EVP_PKEY_sign_init(pkctx)) {
		fprintf(stderr, "error: %s %d\n", __FILE__, __LINE__);
		goto end;
	}

	if (!EVP_PKEY_sign(pkctx, out, poutlen, cbuf, cbuflen)) {
		ERR_print_errors_fp(stderr);
		fprintf(stderr, "error: %s %d\n", __FILE__, __LINE__);
		goto end;
	}
	size_t i;
	printf("sign data ciphertext (%d bytes) = \n", *poutlen);
	for (i = 0; i < *poutlen; i++) {
		printf("%02X ", out[i]);
		if(i % 16 == 15){
			printf("\n");
		}
	}
	printf("\n");

	ret = 1;
end:
	EVP_PKEY_CTX_free(pkctx);
	return ret;
}


static int test_evp_pkey_verify_sign(EVP_PKEY *pkey,unsigned char* buf,size_t buflen ,unsigned char *signbuf, size_t signlen)
{
	int ret = 0;
	EVP_PKEY_CTX *pkctx = NULL;

	if (!(pkctx = EVP_PKEY_CTX_new(pkey, NULL))) {
		ERR_print_errors_fp(stderr);
		fprintf(stderr, "error: %s %d\n", __FILE__, __LINE__);
		goto end;
	}

	if (!EVP_PKEY_verify_init(pkctx)) {
		ERR_print_errors_fp(stderr);
		fprintf(stderr, "error: %s %d\n", __FILE__, __LINE__);
		goto end;
	}

	if (!EVP_PKEY_verify(pkctx, signbuf, signlen, buf, buflen)) {
		fprintf(stderr, "error: %s %d\n", __FILE__, __LINE__);
		goto end;
	}
	else
	{
		printf("verify sign OK\n");
	}

	ret = 1;
end:
	EVP_PKEY_CTX_free(pkctx);
	return ret;
}





int sm3Test(){
  EVP_MD_CTX *mdctx;
  const EVP_MD *md;
  unsigned char mess1[] = "1234567812345678";

  unsigned char md_value[EVP_MAX_MD_SIZE] = {0};
  int md_len, i;

  md = EVP_get_digestbyname("sm3");

  if(!md) {
         return -1;
  }

  mdctx = EVP_MD_CTX_new(); //allocates, initializes and returns a digest context.
  EVP_DigestInit_ex(mdctx, md, NULL);  //sets up digest context ctx to use a digest type from ENGINE impl. 
  EVP_DigestUpdate(mdctx, mess1, 64); //hashes cnt bytes of data at d into the digest context ctx. 
  EVP_DigestFinal_ex(mdctx, md_value, (unsigned int *)&md_len);//retrieves the digest value from ctx and places it in md. 
  EVP_MD_CTX_free(mdctx);

  printf("sm3 Digest is: \n");
  for (i = 0; i < md_len; i++){
    printf("%02x ", md_value[i]);
		if(i % 16 == 15){
			printf("\n");
		}
  } 
  printf("\n");

  return 0;
}





int main(){
	unsigned char buf[]= "1234567812345678";
	unsigned char out[1024]={0};
	size_t outlen = 1024;
	
	unsigned char signdata[1024]={0};
	size_t signlen = 1024;
	
	size_t buflen = strlen((char*)buf);
	BIO *bio_err = BIO_new_fp(stderr, BIO_NOCLOSE | BIO_FP_TEXT);
	BIO *bio_stdout = BIO_new_fp(stdout, BIO_NOCLOSE | BIO_FP_TEXT);
	
	EVP_PKEY* privKey = getPEMPrivateKeyFromBuf(g_prikey,strlen(g_prikey));
	
	EVP_PKEY* pubKey = getPEMPublicKeyFromBuf(g_pubkey,strlen(g_pubkey));
	

	//加密
	test_sm2_encrypt(pubKey, buf, buflen,out,&outlen);
	

	//解密
	test_sm2_decrypt(privKey,out, outlen);


	//sm3摘要
	sm3Test();
	

	
	putchar('\n');

    //sm2签名
	test_evp_pkey_sign(privKey,buf,buflen,signdata,&signlen);
	//sm2验签
	test_evp_pkey_verify_sign(pubKey,buf,buflen,signdata,signlen);
	putchar('\n');

	BIO_free(bio_stdout);
	BIO_free(bio_err);
	
	EVP_PKEY_free(privKey);
	EVP_PKEY_free(pubKey);
	
	
	return 0;
}

