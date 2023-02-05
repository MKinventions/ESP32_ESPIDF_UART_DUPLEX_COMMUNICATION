/* UART asynchronous example, that uses separate RX and TX tasks

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "cJSON.h"

static const int RX_BUF_SIZE = 1024;

#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)

#define POTENTIOMETER1 ADC1_CHANNEL_6//GPIO34(ADC1_6)
#define POTENTIOMETER2 ADC1_CHANNEL_7//GPIO35(ADC1_7)
#define POTENTIOMETER3 ADC2_CHANNEL_5//GPIO12(ADC2_5)
#define POTENTIOMETER4 ADC2_CHANNEL_6//GPIO14(ADC2_6)
#define BTN1 13
#define BTN2 27
#define BTN3 22
#define BTN4 23



void init(void) {
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}


static void tx_task(void *arg)
{


	adc1_config_width(ADC_WIDTH_BIT_10);//(0 - 1023)
//    adc1_config_channel_atten(POTENTIOMETER1, ADC_ATTEN_DB_11);   //~150mv - 2450mv(11dB)
    adc1_config_channel_atten(POTENTIOMETER1, ADC_ATTEN_DB_11);   //~150mv - 2450mv(11dB)
    adc1_config_channel_atten(POTENTIOMETER2, ADC_ATTEN_DB_11);   //~150mv - 2450mv(11dB)
    adc2_config_channel_atten(POTENTIOMETER3, ADC_ATTEN_DB_11 );
    adc2_config_channel_atten(POTENTIOMETER4, ADC_ATTEN_DB_11 );

    gpio_pad_select_gpio(BTN1);
    gpio_pad_select_gpio(BTN2);
    gpio_pad_select_gpio(BTN3);
    gpio_pad_select_gpio(BTN4);

    gpio_pullup_en(BTN1);
    gpio_pullup_en(BTN2);
    gpio_pullup_en(BTN3);
    gpio_pullup_en(BTN4);

    gpio_set_direction(BTN1, GPIO_MODE_INPUT);
    gpio_set_direction(BTN2, GPIO_MODE_INPUT);
    gpio_set_direction(BTN3, GPIO_MODE_INPUT);
    gpio_set_direction(BTN4, GPIO_MODE_INPUT);


    while (1) {



		int POTENTIOMETER1_VALUE = adc1_get_raw(POTENTIOMETER1);
		int POTENTIOMETER2_VALUE = adc1_get_raw(POTENTIOMETER2);
        int POTENTIOMETER3_VALUE;
		int POTENTIOMETER4_VALUE;
		adc2_get_raw(POTENTIOMETER3, ADC_WIDTH_BIT_10, &POTENTIOMETER3_VALUE);
		adc2_get_raw(POTENTIOMETER4, ADC_WIDTH_BIT_10, &POTENTIOMETER4_VALUE);

        int BUTTON1 = gpio_get_level(BTN1);
        int BUTTON2 = gpio_get_level(BTN2);
        int BUTTON3 = gpio_get_level(BTN3);
        int BUTTON4 = gpio_get_level(BTN4);

        float TEMP = 30.056;
        float HUMI = 56;


        char version[10] = "1.1.2v";
    	char pot1Value[10];
    	char pot2Value[10];
    	char pot3Value[10];
    	char pot4Value[10];
    	char btn1State[10];
    	char btn2State[10];
    	char btn3State[10];
    	char btn4State[10];
    	char tempValue[10];
    	char humiValue[10];


    	sprintf(pot1Value, "%d", POTENTIOMETER1_VALUE);
    	sprintf(pot2Value, "%d", POTENTIOMETER2_VALUE);
    	sprintf(pot3Value, "%d", POTENTIOMETER3_VALUE);
    	sprintf(pot4Value, "%d", POTENTIOMETER4_VALUE);
    	sprintf(btn1State, "%d", BUTTON1);
    	sprintf(btn2State, "%d", BUTTON2);
    	sprintf(btn3State, "%d", BUTTON3);
    	sprintf(btn4State, "%d", BUTTON4);
    	sprintf(tempValue, "%f", TEMP);
    	sprintf(humiValue, "%f", HUMI);




//    		ESP_LOGI("TX", "Serialize.....");
    		cJSON *root;
    		root = cJSON_CreateArray();

    		cJSON *element;
    		element = cJSON_CreateString(version);
    		cJSON_AddItemToArray(root, element);
    		element = cJSON_CreateString(pot1Value);
    		cJSON_AddItemToArray(root, element);
    		element = cJSON_CreateString(pot2Value);
    		cJSON_AddItemToArray(root, element);
    		element = cJSON_CreateString(pot3Value);
    		cJSON_AddItemToArray(root, element);
    		element = cJSON_CreateString(pot4Value);
    		cJSON_AddItemToArray(root, element);

    		element = cJSON_CreateString(btn1State);
    		cJSON_AddItemToArray(root, element);
    		element = cJSON_CreateString(btn2State);
    		cJSON_AddItemToArray(root, element);
    		element = cJSON_CreateString(btn3State);
    		cJSON_AddItemToArray(root, element);
    		element = cJSON_CreateString(btn4State);
    		cJSON_AddItemToArray(root, element);

    		element = cJSON_CreateString(tempValue);
    		cJSON_AddItemToArray(root, element);
    		element = cJSON_CreateString(humiValue);
    		cJSON_AddItemToArray(root, element);


    		char *sending_json_string = cJSON_Print(root);
//    		printf("\nSending Data : %s\n", sending_json_string);
    	//	ESP_LOGI(TAG, "my_json_string\n%s",my_json_string);
    		cJSON_Delete(root);

       uart_write_bytes(UART_NUM_1, sending_json_string, strlen(sending_json_string));
    	vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}


static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);

    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 100 / portTICK_RATE_MS);
        if (rxBytes > 0) {
            data[rxBytes] = 0;

//            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);

            char* incoming_json_string = (char*) data;

//        	ESP_LOGI("RX", "Deserialize.....");
        	cJSON *root2 = cJSON_Parse(incoming_json_string);
        	int root2_array_size = cJSON_GetArraySize(root2);
        //	ESP_LOGI(TAG, "root2_array_size=%d", root2_array_size);

        	char str1[25];

        	char firmwareVersion[10];
        	char potentiometer1[10];
        	char potentiometer2[10];
        	char potentiometer3[10];
        	char potentiometer4[10];
        	char button1State[2];
        	char button2State[2];
        	char button3State[2];
        	char button4State[2];
        	char tempValue[10];
        	char humiValue[10];

        	char* version;
        	int pot1value = 0;
        	int pot2value = 0;
        	int pot3value = 0;
        	int pot4value = 0;
        	int btn1State = 0;
        	int btn2State = 0;
        	int btn3State = 0;
        	int btn4State = 0;
        	float temp = 0.0;
        	float humi = 0;


        	for (int i = 0; i < root2_array_size; i++) {
        		cJSON *element = cJSON_GetArrayItem(root2, i);
        		strcpy(str1, element->valuestring);

        		if (i == 0) {
        			strcpy(firmwareVersion, str1);
        			version = firmwareVersion;
//        			printf("firmwareVersion = %s\n", version);
        		}

        		else if (i == 1) {
        			strcpy(potentiometer1, str1);
        			pot1value = atoi(potentiometer1);
//        			printf("potentiometer1 = %d\n", pot1value);
        		}
        		else if (i == 2) {
        			strcpy(potentiometer2, str1);
        			pot2value = atoi(potentiometer2);
//        			printf("potentiometer2 = %d\n", pot2value);
        		}
        		else if (i == 3) {
        			strcpy(potentiometer3, str1);
        			pot3value = atoi(potentiometer3);
//        			printf("potentiometer3 = %d\n", pot3value);
        		}
        		else if (i == 4) {
        			strcpy(potentiometer4, str1);
        			pot4value = atoi(potentiometer4);
//        			printf("potentiometer4 = %d\n", pot4value);
        		}


        		else if (i == 5) {
        			strcpy(button1State, str1);
        			btn1State = atoi(button1State);
//        			printf("button1State = %s\n", btn1State);
        		}
        		else if (i == 6) {
        			strcpy(button2State, str1);
        			btn2State = atoi(button2State);
//        			printf("button2State = %s\n", button2State);
        		}
        		else if (i == 7) {
        			strcpy(button3State, str1);
        			btn3State = atoi(button3State);
//        			printf("button3State = %s\n", btn3State);
        		}
        		else if (i == 8) {
        			strcpy(button4State, str1);
        			btn4State = atoi(button4State);
//        			printf("button4State = %s\n", btn4State);
        		}

        		else if (i == 9) {
        			strcpy(tempValue, str1);
        			temp = atof(tempValue);
//        			printf("Temperature = %.2f\n", temp);
        		}
        		else if (i == 10) {
        			strcpy(humiValue, str1);
        			humi = atof(humiValue);
//        			printf("Humidity = %.2f\n", humi);
        		}
        	}




        	printf("\nVersion:%s, P1:%d, P2:%d, P3:%d, P4:%d, B1:%d, B2:%d, B3:%d, B4:%d, Temp:%.2f, Hum:%.2f\n",
        	 version, pot1value, pot2value, pot3value, pot4value, btn1State, btn2State, btn3State, btn4State, temp, humi);



        	cJSON_Delete(root2);
        	cJSON_free(incoming_json_string);
        }



    }
    free(data);
}



void app_main(void)
{
    init();
    xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
}
