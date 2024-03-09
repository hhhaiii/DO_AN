#include <ArduinoBLE.h>
#include <mic.h> // Assuming this is your microphone library

// Microphone Settings
#define DEBUG 1
#define SAMPLES 1600

mic_config_t mic_config = {
  .channel_cnt = 1,
  .sampling_rate = 16000,
  .buf_size = 1600,
  .debug_pin = LED_BUILTIN
};

NRF52840_ADC_Class Mic(&mic_config);
int16_t recording_buf[SAMPLES];
volatile static bool record_ready = false;

// Updated UUIDs
#define SERVICE_UUID "19B10000-E8F2-537E-4F6C-D104768A1214"
#define CHARACTERISTIC_UUID_AUDIO "19B10001-E8F2-537E-4F6C-D104768A1214"

// BLE Service and Characteristic
BLEService audioService(SERVICE_UUID);
// Corrected initialization with explicit value size and fixed length flag
BLECharacteristic audioDataCharacteristic(CHARACTERISTIC_UUID_AUDIO, BLERead | BLENotify | BLEWrite, sizeof(recording_buf), true);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

//  Serial.println("Initializing microphone...");
  Mic.set_callback(audio_rec_callback);
  if (!Mic.begin()) {
 //   Serial.println("Mic initialization failed");
    while (1);
  }
 // Serial.println("Mic initialized.");

//  Serial.println("Initializing BLE...");
  if (!BLE.begin()) {
 //   Serial.println("Failed to start BLE!");
    while (1);
  }

  BLE.setLocalName("SCT Audio");
  BLE.setAdvertisedService(audioService);
  
  audioService.addCharacteristic(audioDataCharacteristic);
  BLE.addService(audioService);

  // Corrected writeValue call with explicit casting
  audioDataCharacteristic.writeValue((uint8_t)0);
  
  BLE.advertise();
 // Serial.println("BLE Peripheral is now advertising");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
  //  Serial.println("Connected to central device");
    
    while (central.connected()) {
      if (record_ready) {
        // Plot the audio data in the Serial Plotter
        for (int i = 0; i < SAMPLES; i++) {
          Serial.println(recording_buf[i]);
        }

        // Transmit the audio data
        audioDataCharacteristic.writeValue((uint8_t*)recording_buf, 2 * SAMPLES);
           record_ready = false;
        
      }
    }

  //  Serial.println("Disconnected from central device");
  }
}

static void audio_rec_callback(uint16_t *buf, uint32_t buf_len) {
  static uint32_t idx = 0;
  
  for (uint32_t i = 0; i < buf_len; i++) {
    recording_buf[idx++] = buf[i];
    if (idx >= SAMPLES){ 
      idx = 0;
      record_ready = true;
      break;
    } 
  }
}