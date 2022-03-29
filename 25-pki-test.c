#include <stdio.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

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

static void err_generate_prime(const char* var_name)
{
    fprintf(stderr, "Error generating prime number %s\n", var_name);
}

static void err_generate_rsa()
{
    fprintf(stderr, "Error generating RSA key\n%s\n", err_openssl());
}

static void f_dump_rsa(const RSA* rsa)
{
    printf("{\"n\":\"");
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
    printf("\",\"iqmp\":\"");
    BN_print_fp(stdout, RSA_get0_iqmp(rsa));
    printf("\"}\n");
}

//static void test_gen_prime(int bits)
//{
//    BIGNUM* p = BN_new();
//    BIGNUM* q = BN_new();
//    if(!p || !q)
//    {
//        err_allocate();
//        goto cleanup;
//    }
//    if(!BN_generate_prime_ex(p, bits, 0, NULL, NULL, NULL))
//    {
//        err_generate_prime("p");
//        goto cleanup;
//    }
//    if(!BN_generate_prime_ex(q, bits, 0, NULL, NULL, NULL))
//    {
//        err_generate_prime("q");
//        goto cleanup;
//    }
//    char* ss = BN_bn2hex(p);
//    if(!ss)
//    {
//        err_allocate();
//        goto cleanup;
//    }
//    printf("p = %s\n", ss);
//    OPENSSL_free(ss);
//    ss = BN_bn2hex(q);
//    if(!ss)
//    {
//        err_allocate();
//        goto cleanup;
//    }
//    printf("q = %s\n", ss);
//    OPENSSL_free(ss);
//cleanup:
//    BN_free(p);
//    BN_free(q);
//}

static void test_rsa_generate()
{
    BIGNUM* e = BN_new();
    if(!e)
    {
        err_allocate();
        return;
    }
    BN_set_word(e, 65537);
    RSA* rsa = RSA_new();
    if(!rsa)
    {
        err_allocate();
        goto cleanup;
    }
    if(!RSA_generate_key_ex(rsa, 1024, e, NULL))
    {
        err_generate_rsa();
        goto cleanup;
    }
    f_dump_rsa(rsa);
cleanup:
    RSA_free(rsa);
    BN_free(e);
}

int main(int argc, char* argv[])
{
    http_header();
    test_rsa_generate();
    return 0;
}

