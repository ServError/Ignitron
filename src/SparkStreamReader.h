/*
 * SparkDataControl.cpp
 *
 *  Created on: 19.08.2021
 *      Author: stangreg
 */

#ifndef SPARK_STREAM_READER_H // include guard
#define SPARK_STREAM_READER_H

#include <tuple>
#include <vector>
#include <string>

#include "SparkHelper.h"
#include "SparkTypes.h"
#include "Common.h"


using ByteVector = std::vector<byte>;


#define MSG_TYPE_PRESET 1
#define MSG_TYPE_HWPRESET 2
#define MSG_TYPE_FX_ONOFF 3
#define MSG_TYPE_FX_CHANGE 4
#define MSG_TYPE_FX_PARAM 5

#define MSG_PROCESS_RES_COMPLETE 1
#define MSG_PROCESS_RES_INCOMPLETE 2
#define MSG_PROCESS_RES_INITIAL 3

class SparkStreamReader{
	// Parser for Spark messages (from App or Amp)
	// -------------------------------------------

private:

	// String representations of processed data
	std::string raw;
	std::string text;
	std::string indent;
	std::string json;

	// Vector containing struct of cmd, sub_cmd, and payload
	std::vector<CmdData> message = { };
	// Unstructured input data, needs to go through structure_data first
	std::vector<ByteVector> unstructured_data = {};

	// payload of a CmdData object to be interpreted. msg_pos is pointing at the next byte to read
	ByteVector msg;
	int msg_pos;
	// indicator if a block received is the last one
	bool msg_last_block = false;
	std::vector<ByteVector> response;


	// In case a preset was received from Spark, it is saved here. Can then be read by main program
	Preset currentSetting_;
	// Preset number. Can be retrieved by main program in case it has been updated by Spark Amp.
	int currentPresetNumber_ = 0;
	//Flags to indicate that either preset or presetNumber have been updated
	boolean isPresetUpdated_ = false;
	boolean isPresetNumberUpdated_ = false;
	ByteVector acknowledgements;
	int last_message_type_ = 0;

	// Functions to process calls based on identified cmd/sub_cmd.
	void read_effect_parameter();
	void read_effect();
	void read_effect_onoff();
	void read_hardware_preset();
	void read_store_hardware_preset();
	void read_preset();


	// Functions to structure and process input data (high level)
	std::vector<CmdData> read_message();
	boolean structure_data();
	void interpret_data();
	void set_interpreter (ByteVector _msg);
	int run_interpreter (byte _cmd, byte _sub_cmd);

	// Low level functions to read single elements from structured payload
	byte read_byte();
	std::string read_prefixed_string();
	std::string read_string();
	float read_float ();
	boolean read_onoff();

	// Functions to create string representations of processed data
	void start_str();
	void end_str();
	void add_indent();
	void del_indent();
	void add_separator();
	void add_newline();
	void add_python(char* python_str);
	void add_str(char* a_title, std::string a_str, char* nature = "all");
	void add_int(char* a_title, int an_int, char* nature = "all");
	void add_float(char* a_title, float a_float, char* nature = "all");
	void add_float_pure(float a_float, char* nature = "all");
	void add_bool(char* a_title, boolean a_bool, char* nature = "all");




public:

	//Constructors
	SparkStreamReader();
	// setting the messag so it can be structured and interpreted
	void setMessage(std::vector<ByteVector> msg);
	std::string getJson();

	// Preset related methods to make information public
	const Preset currentSetting() const {return currentSetting_;}
	const int currentPresetNumber() const { return currentPresetNumber_;}
	const boolean isPresetNumberUpdated() const { return isPresetNumberUpdated_;}
	const boolean isPresetUpdated() const { return isPresetUpdated_;}
	const int lastMessageType() const { return last_message_type_;}

	void resetPresetNumberUpdateFlag();
	void resetPresetUpdateFlag();
	std::tuple<boolean, byte, byte> needsAck(ByteVector block);
	int processBlock(ByteVector block);
	byte getLastAckAndEmpty();

};

#endif 
