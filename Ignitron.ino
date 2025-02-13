

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <string>
#include <NimBLEDevice.h> // github NimBLE

#include "src/SparkButtonHandler.h"
#include "src/SparkDataControl.h"
#include "src/SparkDisplayControl.h"
#include "src/SparkLEDControl.h"

// Device Info Definitions
const std::string DEVICE_NAME = "Ignitron";
const std::string VERSION = "1.0";

// Control classes
SparkDataControl *spark_dc;
SparkButtonHandler *spark_bh;
SparkLEDControl *spark_led;
SparkDisplayControl *spark_display;
SparkOTAServer *spark_ota;

// Check for initial boot
bool isInitBoot;
int operationMode = SPARK_MODE_APP;

class SparkOTAServer;

/////////////////////////////////////////////////////////
//
// INIT AND RUN
//
/////////////////////////////////////////////////////////

void setup() {
	//delay(2000);
	isInitBoot = true;
	Serial.begin(115200);
	while (!Serial)
		;

	Serial.println("Initializing");
	spark_dc = new SparkDataControl();
	spark_bh = new SparkButtonHandler(spark_dc);
	operationMode = spark_bh->init();
	if (operationMode == SPARK_MODE_APP) {
		uint8_t mac_keyboard[] = { 0xB4, 0xE6, 0x2D, 0xB2, 0x1B, 0x36 }; //{0x36, 0x33, 0x33, 0x33, 0x33, 0x33};
		esp_base_mac_addr_set(&mac_keyboard[0]);
		Serial.println("======= Entering APP mode =======");
	} else if (operationMode == SPARK_MODE_AMP) {
		//uint8_t mac_keyboard[] = { 0xB4, 0xE6, 0x2D, 0xB2, 0x1B, 0x46 }; //{0x36, 0x33, 0x33, 0x33, 0x33, 0x33};
		//esp_base_mac_addr_set(&mac_keyboard[0]);
		Serial.println("======= Entering AMP mode =======");
	}
	// Setting operation mode before initializing
	spark_dc->init(operationMode);

	spark_display = new SparkDisplayControl(spark_dc);
	spark_dc->setDisplayControl(spark_display);
	spark_display->init(operationMode);
	// Assigning data control to buttons;
	spark_bh->setDataControl(spark_dc);
	// Initializing control classes
	spark_led = new SparkLEDControl(spark_dc);
	if (operationMode == SPARK_MODE_AMP) {
		spark_dc->connectToWifi();
	}
	Serial.println("Initialization done.");

}

void loop() {

	// Methods to call only in APP mode
	if (operationMode == SPARK_MODE_APP) {
		while (!(spark_dc->checkBLEConnection())) {
			spark_display->update(isInitBoot);
			spark_bh->readButtons();
		}

		//After connection is established, continue.
		// On first boot, set the preset to Hardware setting 1.
		if (isInitBoot == true) {
			DEBUG_PRINTLN("Initial boot, setting preset to HW 1");
			if (spark_dc->switchPreset(1)) {
				isInitBoot = false;
			}
		}
	}
	// Check if presets have been updated
	spark_dc->checkForUpdates();
	// Reading button input
	spark_bh->readButtons();
	// Update LED status
	spark_led->updateLEDs();
	// Update display
	spark_display->update();
}
