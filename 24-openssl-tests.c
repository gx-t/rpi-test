#include <stdio.h>
#include <openssl/bn.h>

static void err_generate_prime(const char* var_name)
{
    fprintf(stderr, "Error generating prime number %s\n", var_name);
}

static void err_allocate_string()
{
    fprintf(stderr, "Cannot allocate string data\n");
}

static void test_gen_prime(int bits)
{
    BIGNUM* p = BN_new();
    BIGNUM* q = BN_new();
    if(!p || !q)
        goto cleanup;
    if(!BN_generate_prime_ex(p, bits, 0, NULL, NULL, NULL))
    {
        err_generate_prime("p");
        goto cleanup;
    }
    if(!BN_generate_prime_ex(q, bits, 0, NULL, NULL, NULL))
    {
        err_generate_prime("q");
        goto cleanup;
    }
    char* ss = BN_bn2hex(p);
    if(!ss)
    {
        err_allocate_string();
        goto cleanup;
    }
    printf("p = %s\n", ss);
    OPENSSL_free(ss);
    ss = BN_bn2hex(q);
    if(!ss)
    {
        err_allocate_string();
        goto cleanup;
    }
    printf("p = %s\n", ss);
    OPENSSL_free(ss);
cleanup:
    BN_free(p);
    BN_free(q);
}

int main(int argc, char* argv[])
{
    test_gen_prime(512);
    return 0;
}

