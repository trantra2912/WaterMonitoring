#include "driver/i2c.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc_cal.h"
#include "esp_system.h"

#include "esp_http_client.h"
#include "../components/connect_wifi.h"
#include "../components/myTDS.h"
#include "../components/nvs_interface.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

static const char *TAG = "HTTP_CLIENT";
#define SCOUNT 50
float T, DObh, ntu, WQIdoduc, tds;
float V0 = 4095, R1 = 5000, R0 = 10000, B = 3950, T0 = 298.15;
float V1, R2, r;
int analogBuffer[SCOUNT];

static esp_adc_cal_characteristics_t adc1_chars;
QueueHandle_t queue;
nvs_handle_t nvsHandle;

int getMedianNum(int bArray[], int iFilterLen, adc1_channel_t adc_chan)
{
  int adc=0;
  for (int i = 0; i < iFilterLen; i++)
  {
    bArray[i]=adc1_get_raw(adc_chan);
    vTaskDelay(100 / portTICK_RATE_MS);
  }
  for (int i = 0; i < iFilterLen - 1; i++)
  {
    for (int j = 0; j < iFilterLen - i - 1; j++)
    {
      if (bArray[j] > bArray[j + 1])
      {
        int bTemp = bArray[j];
        bArray[j] = bArray[j + 1];
        bArray[j + 1] = bTemp;
      }
    }
  }
  for(int i = 0; i <iFilterLen ; i ++)
  {
		adc += bArray[i];
	}
	adc /=iFilterLen;
  return adc;
}

float WQI_doduc(float Cp){
  float WQI_doduc;
  if(Cp<=5){
    WQI_doduc=100;
  }
  if(5<Cp&&Cp<20){
    WQI_doduc=(float)(100-75)*(20-Cp)/(20-5)+75;
  }
  if(Cp==20){
    WQI_doduc=75;
  }
  if(20<Cp&&Cp<30){
    WQI_doduc=(float)(75-50)*(30-Cp)/(30-20)+50;
  }
  if(Cp==30){
    WQI_doduc=50;
  }
  if(30<Cp&&Cp<70){
    WQI_doduc=(float)(50-25)*(70-Cp)/(70-30)+25;
  }
  if(Cp==70){
    WQI_doduc=25;
  }
  if(70<Cp&&Cp<100){
    WQI_doduc=(float)(25-1)*(100-Cp)/(100-70)+1;
  }
  if(Cp>=100){
    WQI_doduc=1;
  }
  return WQI_doduc;
}
void DO_bh(void *arg)
{
  int a = 1;
  int analogBuffer1[SCOUNT];
  r = R0 * exp(-B / T0);
  while (1)
  {
      V1 = getMedianNum(analogBuffer1, SCOUNT, ADC1_CHANNEL_4);
      R2 = R1 * (V0 / V1 - 1);
      T = B / log(R2 / r) - 273.15;
      DObh = 14.652 - 0.41022 * T + 0.007991 * pow(T, 2) - 0.000077774 * pow(T, 3);
      printf("T=%.2f, DObh=%.2f\n ", T, DObh);
      xQueueSend(queue, &a, (TickType_t)0);
      //vTaskDelay(1000 / portTICK_RATE_MS);
  }
}

void NTU(void *arg)
{
  float V2;
  int analogBuffer2[SCOUNT];
  while (1)
  {
      V2 = (float)getMedianNum(analogBuffer2, SCOUNT, ADC1_CHANNEL_5) * 5 / 4095;
      if(V2 < 2.5){
        ntu = 3000;
      }
      else{
        ntu = -1120.4*pow(V2,2)+5742.3*V2-4352.9; 
      }
      WQIdoduc= WQI_doduc(ntu);
      printf("V2=%.2f, ntu=%.2f, WQIdoduc=%.2f\n", V2, ntu, WQIdoduc);
      //vTaskDelay(1000 / portTICK_RATE_MS);
  }
}

void TDS(void *arg)
{
  while (1)
  {
    tds = get_TDS_toSalt(ADC1_CHANNEL_6, T, 0.5, 4095.0, 5000.0);
	  printf("\ntds=%.2f\n",tds);
  }
}

void send_data_to_pwm(void *pvParameters)
{
  int b;
  char pwm_url[] = "http://192.168.1.160:3000";
  char data[] = "/upload?Nhietdo=%.2f&DO_baohoa=%.2f&NTU=%.2f&WQI_NTU=%.2f&TDS=%.2f";
  char post_data[200];
  esp_err_t err;

  esp_http_client_config_t config = {
      .url = pwm_url,
      .method = HTTP_METHOD_GET,
  };
  esp_http_client_handle_t client = esp_http_client_init(&config);
  esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");
  while (1)
  {
    xQueueReceive(queue, &b, (TickType_t)1000);
    if(b==1)
    {
      vTaskDelay(500/ portTICK_RATE_MS);
      strcpy(post_data, "");
      snprintf(post_data, sizeof(post_data), data, T, DObh, ntu, WQIdoduc, tds);
      ESP_LOGI(TAG, "post = %s", post_data);
      esp_http_client_set_post_field(client, post_data, strlen(post_data));
      esp_http_client_set_url(client, post_data);
      err = esp_http_client_perform(client);

      if (err == ESP_OK)
      {
        int status_code = esp_http_client_get_status_code(client);
        if (status_code == 200)
        {
          ESP_LOGI(TAG, "Message sent Successfully");
        }
        else
        {
          ESP_LOGI(TAG, "Message sent Failed");
          goto exit;
        }
      }
      else
      {
        ESP_LOGI(TAG, "Message sent Failed1");
        goto exit;
      }
    }
  }
  exit:
    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
}
void app_main(void)
{
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars); // hiệu chỉnh ADC1 ở mức suy giảm 11db
  adc1_config_width(ADC_WIDTH_BIT_DEFAULT);                                                     // đặt cấu hình ADC1 ở độ rộng bit mặc định (12bit)
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);                                   // đặt tham số suy giảm của ADC1 kênh 4 là GPIO32 thành 11db
  adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
  queue = xQueueCreate(5, sizeof(int));

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  connect_wifi();
  // r = R0 * exp(-B / T0);
  // V1 = getMedianNum(analogBuffer, SCOUNT, ADC1_CHANNEL_4);
  // R2 = R1 * (V0 / V1 - 1);
  // T = B / log(R2 / r) - 273.15;
  // printf("T=%f\n ", T);
  //TDS_calib(nvsHandle, ADC1_CHANNEL_6, 5000.0, 4096.0, "storage", 111.0, T);
  TDS_init_param(&nvsHandle);
  if (wifi_connect_status)
  {
    xTaskCreatePinnedToCore(DO_bh, "DO_bh", 2048, NULL, 10, NULL, 0);
    xTaskCreatePinnedToCore(NTU, "NTU", 2048, NULL, 10, NULL, 1);
    xTaskCreatePinnedToCore(TDS, "TDS", 2048, NULL, 10, NULL, 0);

    xTaskCreate(&send_data_to_pwm, "send_data_to_pwm", 8192, NULL, 10, NULL);
  }
}
