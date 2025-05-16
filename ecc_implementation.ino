/**
 * @file main.cpp
 * @author Kaustubh (kaustubhdubey.com)
 * @brief
 * @version 0.2
 * @date 2025-05-16
 *
 * @copyright Copyright (c) 2025
 *
 * I’m posting this update after a long time. It includes a demo/concept showing how to perform key exchange using ECDH (Elliptic Curve Diffie–Hellman), as well as encryption and decryption using the AES standard.
 * This is in a separate file because I created it specifically for the ESP32 (my ESP8266 was being used for another purpose). I believe this code should work on the ESP8266 as well. I plan to post a full implementation soon, which was originally intended to be part of the main file.
 * By the way, I used PlatformIO for this specific code. If you're running it on the Arduino IDE, you may need to make a few adjustments, but I don’t think it will require many changes.
 * For now, I am just uploading this code as an .ino file, but in the future, I plan to publish this project using the PlatformIO framework.
 */

 #include <Arduino.h>
 #include "mbedtls/ecdsa.h"
 #include "mbedtls/ecdh.h"
 #include "mbedtls/ecp.h"
 #include "mbedtls/ctr_drbg.h"
 #include "mbedtls/entropy.h"
 #include "mbedtls/aes.h"
 
 void print_hex(const char *label, const uint8_t *data, size_t len)
 {
   Serial.print(label);
   for (size_t i = 0; i < len; i++)
   {
     if (i % 16 == 0)
       Serial.print("\n");
     Serial.printf("%02X ", data[i]);
   }
   Serial.println();
 }
 
 void aes_encrypt(const uint8_t *key, const uint8_t *iv, const uint8_t *input,
                  uint8_t *output, size_t len)
 {
   mbedtls_aes_context aes;
   mbedtls_aes_init(&aes);
   mbedtls_aes_setkey_enc(&aes, key, 256);
   uint8_t iv_copy[16];
   memcpy(iv_copy, iv, 16);
   mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, len, iv_copy, input, output);
   mbedtls_aes_free(&aes);
 }
 
 void aes_decrypt(const uint8_t *key, const uint8_t *iv, const uint8_t *input,
                  uint8_t *output, size_t len)
 {
   mbedtls_aes_context aes;
   mbedtls_aes_init(&aes);
   mbedtls_aes_setkey_dec(&aes, key, 256);
   uint8_t iv_copy[16];
   memcpy(iv_copy, iv, 16);
   mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, len, iv_copy, input, output);
   mbedtls_aes_free(&aes);
 }
 
 /**
  * Demo code for ECC key generation and AES encryption/decryption
  * using the shared secret derived from ECDH.
  * The same concept can be user in end-to-end encryption as well as in transport-level encryption.
  */
 
 void setup()
 {
   Serial.begin(115200);
   delay(5000);
   Serial.println("ECC Key gen demo:");
 
   const char *pers = "ecdh_demo";
   mbedtls_entropy_context entropy;
   mbedtls_ctr_drbg_context ctr_drbg;
 
   mbedtls_entropy_init(&entropy);
   mbedtls_ctr_drbg_init(&ctr_drbg);
   mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                         (const unsigned char *)pers, strlen(pers));
 
   /** 
    * For now, let's refer to the two parties as user1 and user2, but in the full implementation, user2 will be replaced by server.
    * This will also require me to implement backend code to handle the server-side operations.
    */
   
   mbedtls_ecdh_context user1, user2;
   mbedtls_ecdh_init(&user1);
   mbedtls_ecdh_init(&user2);
 
   mbedtls_ecp_group_load(&user1.grp, MBEDTLS_ECP_DP_SECP256R1);
   mbedtls_ecp_group_load(&user2.grp, MBEDTLS_ECP_DP_SECP256R1);
 
   // Generate key pairs
   mbedtls_ecdh_gen_public(&user1.grp, &user1.d, &user1.Q,
                           mbedtls_ctr_drbg_random, &ctr_drbg);
   mbedtls_ecdh_gen_public(&user2.grp, &user2.d, &user2.Q,
                           mbedtls_ctr_drbg_random, &ctr_drbg);
 
   // Exchange public keys and compute shared secrets
   mbedtls_ecdh_compute_shared(&user1.grp, &user1.z, &user2.Q, &user1.d,
                               mbedtls_ctr_drbg_random, &ctr_drbg);
   mbedtls_ecdh_compute_shared(&user2.grp, &user2.z, &user1.Q, &user2.d,
                               mbedtls_ctr_drbg_random, &ctr_drbg);
 
   // Convert shared secret to binary
   uint8_t shared_secret[32];
   mbedtls_mpi_write_binary(&user1.z, shared_secret, 32);
 
   print_hex("Shared Secret:", shared_secret, 32);
 
   // Encrypt a message using shared secret
   const char *msg = "A random message";
   uint8_t padded[32] = {0};
   memcpy(padded, msg, strlen(msg)); // AES CBC needs block-aligned size
 
   uint8_t iv[16] = {0};
   uint8_t encrypted[32] = {0};
   uint8_t decrypted[32] = {0};
 
   aes_encrypt(shared_secret, iv, padded, encrypted, 32);
   print_hex("Encrypted:", encrypted, 32);
 
   aes_decrypt(shared_secret, iv, encrypted, decrypted, 32);
   Serial.print("Decrypted: ");
   Serial.println((char *)decrypted);
 
   // Cleanup
   mbedtls_ecdh_free(&user1);
   mbedtls_ecdh_free(&user2);
   mbedtls_ctr_drbg_free(&ctr_drbg);
   mbedtls_entropy_free(&entropy);
 }
 
 /**
  * Demo code for ECC key generation only
 
 void setup() {
   Serial.begin(115200);
   Serial.println("Key generation will start after 5 seconds...");
   delay(5000);
   Serial.println("ECC Key gen demo:");
 
   mbedtls_ecdsa_context ctx;
   mbedtls_entropy_context entropy;
   mbedtls_ctr_drbg_context ctr_drbg;
 
   mbedtls_ecdsa_init(&ctx);
   mbedtls_entropy_init(&entropy);
   mbedtls_ctr_drbg_init(&ctr_drbg);
 
   const char* pers = "ecc_keygen";
   int ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                   (const unsigned char*)pers, strlen(pers));
   if (ret != 0) {
     Serial.printf("Seed failed: -0x%04X\n", -ret);
     return;
   }
 
   ret = mbedtls_ecdsa_genkey(&ctx, MBEDTLS_ECP_DP_SECP256R1,
                              mbedtls_ctr_drbg_random, &ctr_drbg);
   if (ret != 0) {
     Serial.printf("Key generation failed: -0x%04X\n", -ret);
     return;
   }
 
   // public key
   uint8_t pubkey[65];
   size_t pub_len = 0;
   mbedtls_ecp_point_write_binary(&ctx.grp, &ctx.Q,
                                  MBEDTLS_ECP_PF_UNCOMPRESSED,
                                  &pub_len, pubkey, sizeof(pubkey));
 
   // private key
   uint8_t privkey[32];
   mbedtls_mpi_write_binary(&ctx.d, privkey, sizeof(privkey));
 
   print_hex("Private Key:", privkey, sizeof(privkey));
   print_hex("Public Key:", pubkey, pub_len);
 
   mbedtls_ecdsa_free(&ctx);
   mbedtls_ctr_drbg_free(&ctr_drbg);
   mbedtls_entropy_free(&entropy);
 }
 
 */
 
 void loop()
 {
 }
 