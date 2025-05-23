#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_BMP280.h>
#include "mbedtls/ecdh.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/aes.h"
#include "mbedtls/ecp.h"
#include "base64.h"
#include "secrets.h"

WiFiClient client;
Adafruit_BMP280 bmp;

mbedtls_entropy_context entropy;
mbedtls_ctr_drbg_context ctr_drbg;

void connectWiFi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to WiFi");
}

void setup()
{
  Serial.begin(115200);
  delay(3000);
  connectWiFi();

  /* Comment this if sensor is sensor is not connected and you want to use demo data */
  bmp.begin(0x76);

  mbedtls_entropy_init(&entropy);
  mbedtls_ctr_drbg_init(&ctr_drbg);
  const char *pers = "esp32_ecc";
  mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *)pers, strlen(pers));
}

void loop()
{
  /* Use sensor data */
  double temperature = bmp.readTemperature();
  double pressure = bmp.readPressure();
  double altitude = bmp.readAltitude(1013.25);

  /* Use demo data instead */
  // double temperature = 11.0;
  // double pressure = 1015.40;
  // double altitude = 3500.25;

  Serial.printf("T: %.2lf, P: %.2lf, A: %.2lf\n", temperature, pressure, altitude);

  mbedtls_ecdh_context ctx;
  mbedtls_ecdh_init(&ctx);
  mbedtls_ecp_group_load(&ctx.grp, MBEDTLS_ECP_DP_SECP256R1);


  mbedtls_ecdh_gen_public(&ctx.grp, &ctx.d, &ctx.Q, mbedtls_ctr_drbg_random, &ctr_drbg);
  mbedtls_mpi_read_binary(&ctx.Qp.X, serverPubKeyX, 32);
  mbedtls_mpi_read_binary(&ctx.Qp.Y, serverPubKeyY, 32);
  mbedtls_mpi_lset(&ctx.Qp.Z, 1);


  mbedtls_ecdh_compute_shared(&ctx.grp, &ctx.z, &ctx.Qp, &ctx.d, mbedtls_ctr_drbg_random, &ctr_drbg);
  uint8_t sharedSecret[32];
  mbedtls_mpi_write_binary(&ctx.z, sharedSecret, 32);

  uint8_t aesKey[16];
  memcpy(aesKey, sharedSecret, 16);

  /* Use this version for fixed maximum payload length */
  // char payload[64];
  // snprintf(payload, sizeof(payload), "T=%.2lf,P=%.2lf,A=%.2lf", temperature, pressure, altitude);
  // size_t payloadLen = strlen(payload);

  /* Use this version for variable payload length */
  char *payload = nullptr;
  int payloadLen = asprintf(&payload, "T=%.2lf,P=%.2lf,A=%.2lf", temperature, pressure, altitude);
  if (payloadLen == -1 || !payload)
  {
    Serial.println("Failed to allocate payload");
    return;
  }
  Serial.println(payload);

  size_t paddedLen = ((payloadLen + 15) / 16) * 16;
  uint8_t padded[paddedLen];
  memset(padded, 0, paddedLen);
  memcpy(padded, payload, payloadLen);

  uint8_t iv[16];
  esp_fill_random(iv, 16);

  uint8_t encrypted[paddedLen];
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, aesKey, 128);
  uint8_t ivCopy[16];
  memcpy(ivCopy, iv, 16);
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, paddedLen, ivCopy, padded, encrypted);
  mbedtls_aes_free(&aes);

  // Send to server
  HTTPClient http;
  http.begin(client, SERVER_URL);
  http.addHeader("Content-Type", "application/json");

  JsonDocument doc;
  doc["data"] = base64::encode(encrypted, paddedLen);

  JsonArray pubkey = doc["pubkey"].to<JsonArray>();
  uint8_t pubX[32], pubY[32];
  mbedtls_mpi_write_binary(&ctx.Q.X, pubX, 32);
  mbedtls_mpi_write_binary(&ctx.Q.Y, pubY, 32);
  for (int i = 0; i < 32; i++)
    pubkey.add(pubX[i]);
  for (int i = 0; i < 32; i++)
    pubkey.add(pubY[i]);

  JsonArray ivJson = doc["iv"].to<JsonArray>();
  for (int i = 0; i < 16; i++)
    ivJson.add(iv[i]);

  String json;
  serializeJson(doc, json);
  int httpCode = http.POST(json);
  String response = http.getString();
  Serial.printf("HTTP %d: %s\n", httpCode, response.c_str());

  http.end();
  mbedtls_ecdh_free(&ctx);

  free(payload);

  delay(15000);
}
