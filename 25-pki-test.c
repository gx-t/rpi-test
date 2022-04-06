#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

static char* get_query_token(char** pp)
{
    char* token = *pp;
    while(**pp && **pp != '&') (*pp) ++;
    if(**pp)
        *(*pp)++ = 0;
    return token;
}

static void http_header()
{
    fprintf(stdout, "Content-type: application/json; charset=UTF-8\n\n");
    fflush(stdout);
}

static char* err_openssl()
{
    return ERR_error_string(ERR_get_error(), NULL);
}

static void err_allocate()
{
    fprintf(stderr, "Cannot allocate memory\n");
}

static void err_generate_rsa()
{
    fprintf(stderr, "Error generating RSA key\n%s\n", err_openssl());
}

//static void json_print_bn(const char* var_name, const BIGNUM* bn)
//{
//    int len = BN_num_bytes(bn);
//    uint8_t bin[len];
//    uint8_t* pp = bin;
//    BN_bn2bin(bn, bin);
//    printf("\"%s\":\"");
//    while(len --)
//    {
//        printf("%02X", *pp);
//        pp ++;
//    }
//    printf("\"");
//
//}

static void f_dump_rsa(const RSA* rsa)
{
    printf("{\"len\":\"%d", RSA_bits(rsa));
    printf("\",\"n\":\"");
    BN_print_fp(stdout, RSA_get0_n(rsa));
    printf("\",\"e\":\"");
    BN_print_fp(stdout, RSA_get0_e(rsa));
    printf("\",\"d\":\"");
    BN_print_fp(stdout, RSA_get0_d(rsa));
    printf("\",\"p\":\"");
    BN_print_fp(stdout, RSA_get0_p(rsa));
    printf("\",\"q\":\"");
    BN_print_fp(stdout, RSA_get0_q(rsa));
    printf("\",\"dp\":\"");
    BN_print_fp(stdout, RSA_get0_dmp1(rsa));
    printf("\",\"dq\":\"");
    BN_print_fp(stdout, RSA_get0_dmq1(rsa));
    printf("\",\"qinv\":\"");
    BN_print_fp(stdout, RSA_get0_iqmp(rsa));
    printf("\"}\n");
}

static int f_rsa_generate(char* pp)
{
    RSA* rsa = NULL;
    BIGNUM* e = NULL;

    int cleanup(int ret)
    {
        RSA_free(rsa);
        BN_free(e);
        return ret;
    }

    int len = 0;
    unsigned long pub_exp = 0;
    char* str_len = get_query_token(&pp);
        len = atoi(str_len);

    if(len < 512 || len > 4096)
        len = 1024;
   
   char* str_pub_exp = get_query_token(&pp);
   pub_exp = atoi(str_pub_exp);

   if(pub_exp < 3 || pub_exp > 65537)
       pub_exp = 65537;

    e = BN_new();
    if(!e)
    {
        err_allocate();
        return cleanup(1);
    }
    BN_set_word(e, pub_exp);
    rsa = RSA_new();
    if(!rsa)
    {
        err_allocate();
        return cleanup(1);
    }
    if(!RSA_generate_key_ex(rsa, len, e, NULL))
    {
        err_generate_rsa();
        return cleanup(1);
    }
    f_dump_rsa(rsa);
    return cleanup(0);
}

int main(int argc, char* argv[])
{
    http_header();

    char* pp = getenv("QUERY_STRING");
    if(!pp)
        return 1;

    char* cmd = get_query_token(&pp); 
    if(!strcmp("rsagen", cmd))
        return f_rsa_generate(pp);
    return 1;
}

