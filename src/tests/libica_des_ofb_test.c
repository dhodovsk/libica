/* This program is released under the Common Public License V1.0
 *
 * You should have received a copy of Common Public License V1.0 along with
 * with this program.
 */

/* Copyright IBM Corp. 2010, 2011 */
#include <fcntl.h>
#include <sys/errno.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h> 
#include "ica_api.h"

#define NR_RANDOM_TESTS 10000

void dump_array(unsigned char *ptr, unsigned int size)
{
	unsigned char *ptr_end;
	unsigned char *h;
	int i = 1;

	h = ptr;
	ptr_end = ptr + size;
	while (h < (unsigned char *)ptr_end) {
		printf("0x%02x ",(unsigned char ) *h);
		h++;
		if (i == 8) {
			printf("\n");
			i = 1;
		} else {
			++i;
		}
	}
	printf("\n");
}

void dump_ofb_data(unsigned char *iv, unsigned int iv_length,
                   unsigned char *key, unsigned int key_length,
                   unsigned char *input_data, unsigned int data_length,
                   unsigned char *output_data)
{
	printf("IV \n");
	dump_array(iv, iv_length);
	printf("Key \n");
	dump_array(key, key_length);
	printf("Input Data\n");
	dump_array(input_data, data_length);
	printf("Output Data\n");
	dump_array(output_data, data_length);
}

int load_random_test_data(unsigned char *data, unsigned int data_length,
		    	   unsigned char *iv, unsigned int iv_length,
		    	   unsigned char *key, unsigned int key_length)
{
	int rc;
	rc = ica_random_number_generate(data_length, data);
	if (rc) {
		printf("ica_random_number_generate with rc = %i errnor = %i\n",
		       rc, errno);
		return rc;
	}
	rc = ica_random_number_generate(iv_length, iv);
	if (rc) {
		printf("ica_random_number_generate with rc = %i errnor = %i\n",
		       rc, errno);
		return rc;
	}
	rc = ica_random_number_generate(key_length, key);
	if (rc) {
		printf("ica_random_number_generate with rc = %i errnor = %i\n",
		       rc, errno);
		return rc;
	}
	return rc;
}

int random_des_ofb(int iteration, int silent, unsigned int data_length)
{
	unsigned int iv_length = sizeof(ica_des_vector_t);
	unsigned int key_length = sizeof(ica_des_key_single_t);

	unsigned char iv[iv_length];
	unsigned char tmp_iv[iv_length];
	unsigned char key[key_length];
	unsigned char input_data[data_length];
	unsigned char encrypt[data_length];
	unsigned char decrypt[data_length];

	int rc = 0;
	memset(encrypt, 0x00, data_length);
	memset(decrypt, 0x00, data_length);

	load_random_test_data(input_data, data_length, iv, iv_length, key,
			      key_length);
	memcpy(tmp_iv, iv, iv_length);

	printf("Test Parameters for iteration = %i\n", iteration);
	printf("key length = %i, data length = %i, iv length = %i\n",
	       key_length, data_length, iv_length);

	rc = ica_des_ofb(input_data, encrypt, data_length, key, tmp_iv, 1);
	if (rc) {
		printf("ica_des_ofb encrypt failed with rc = %i\n", rc);
		dump_ofb_data(iv, iv_length, key, key_length, input_data,
			      data_length, encrypt);
	}
	if (!silent && !rc) {
		printf("Encrypt:\n");
		dump_ofb_data(iv, iv_length, key, key_length, input_data,
			      data_length, encrypt);
	}

	if (rc) {
		printf("DES OFB test exited after encryption\n");
		return rc;
	}

	memcpy(tmp_iv, iv, iv_length);

	rc = ica_des_ofb(encrypt, decrypt, data_length, key, tmp_iv, 0);
	if (rc) {
		printf("ica_des_ofb decrypt failed with rc = %i\n", rc);
		dump_ofb_data(iv, iv_length, key, key_length, encrypt,
			      data_length, decrypt);
		return rc;
	}


	if (!silent && !rc) {
		printf("Decrypt:\n");
		dump_ofb_data(iv, iv_length, key, key_length, encrypt,
			      data_length, decrypt);
	}

	if (memcmp(decrypt, input_data, data_length)) {
		printf("Decryption Result does not match the original data!\n");
		printf("Original data:\n");
		dump_array(input_data, data_length);
		printf("Decryption Result:\n");
		dump_array(decrypt, data_length);
		rc++;
	}
	return rc;
}

int main(int argc, char **argv)
{
	unsigned int silent = 0;
	if (argc > 1) {
		if (strstr(argv[1], "silent"))
			silent = 1;
	}
	int rc = 0;
	int error_count = 0;
	int iteration;
	unsigned int data_length = sizeof(ica_des_vector_t);
	for(iteration = 1; iteration <= NR_RANDOM_TESTS; iteration++)	{
		int silent = 1;
		rc = random_des_ofb(iteration, silent, data_length);
		if (rc) {
			printf("random_des_ofb failed with rc = %i\n", rc);
			error_count++;
			goto out;
		} else
			printf("random_des_ofb finished successfuly\n");
		data_length += sizeof(ica_des_vector_t);
	}
out:
	if (error_count)
		printf("%i testcases failed\n", error_count);
	else
		printf("All testcases finished successfully\n");

	return rc;
}
