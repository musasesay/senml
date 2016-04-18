#include "senml.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <jansson.h>
#include <cbor.h>

int senml_decode_json_s(const char *input, senml_pack_t *pack)
{
	(void)input;
	(void)pack;
	return 0;
}


static inline bool senml_is_base_info(json_t *record)
{
	if (json_object_get(record, SJ_VERSION) ||
	    json_object_get(record, SJ_BASE_NAME) ||
	    json_object_get(record, SJ_BASE_TIME) ||
	    json_object_get(record, SJ_BASE_UNIT) ||
	    json_object_get(record, SJ_BASE_VALUE))
		return true;
	else
		return false;
}


senml_pack_t *senml_decode_json(const char *input)
{
	json_error_t json_error;
	
	json_t *json_root = json_loads(input, 0, &json_error);
	
	if (!json_root) {
		fprintf(stderr, "ERROR: on line %d: %s\n", json_error.line, json_error.text);
		return NULL;
	}
	
	if (!json_is_array(json_root)) {
		fprintf(stderr, "ERROR: not an array\n");
		json_decref(json_root);
		return NULL;
	}
	
	senml_pack_t       *pack      = malloc(sizeof(senml_pack_t));
	
	pack->base_info = NULL;
	pack->records   = malloc(sizeof(senml_record_t) * json_array_size(json_root));
	pack->num       = json_array_size(json_root);
	
	unsigned int i = 0;
	
	json_t *json_record = json_array_get(json_root, i);
	json_t *object      = NULL;
	
	if (senml_is_base_info(json_record)) {
		pack->base_info = malloc(sizeof(senml_base_info_t));
		pack->base_info->base_value_type = SENML_TYPE_UNDEF;
		i++;
		(pack->num)--;
		
		object = json_object_get(json_record, SJ_VERSION);
		
		if (object) {
			pack->base_info->version = (uint8_t)json_integer_value(object);
		}
		
		object = json_object_get(json_record, SJ_BASE_NAME);
		
		if (object) {
			pack->base_info->base_name = malloc(json_string_length(object) + 1);
			strcpy(pack->base_info->base_name, json_string_value(object));
		}
		
		object = json_object_get(json_record, SJ_BASE_TIME);
		
		if (object) {
			pack->base_info->base_time = json_real_value(object);
		}
		
		object = json_object_get(json_record, SJ_BASE_UNIT);
		
		if (object) {
			pack->base_info->base_unit = malloc(json_string_length(object) + 1);
			strcpy(pack->base_info->base_unit, json_string_value(object));
		}
		
		object = json_object_get(json_record, SJ_BASE_VALUE);
		
		if (object) {
			// FIXME how do we handle different data types here?
		}
	}
	
	for (unsigned int index = 0; i < json_array_size(json_root); i++, index++) {
		json_record = json_array_get(json_root, i);
		
		if (!json_is_object(json_record)) {
			fprintf(stderr, "ERROR: record #%d is not valid\n", i+1);
			goto error;
		}
		
		object = json_object_get(json_record, SJ_NAME);
		
		if (object) {
			pack->records[index].name = malloc(json_string_length(object) + 1);
			strcpy(pack->records[index].name, json_string_value(object));
		}
		
		object = json_object_get(json_record, SJ_UNIT);
		
		if (object) {
			pack->records[index].unit = malloc(json_string_length(object) + 1);
			strcpy(pack->records[index].unit, json_string_value(object));
		}
		
		object = json_object_get(json_record, SJ_TIME);
		
		if (object) {
			pack->records[index].time = json_real_value(object);
		}
		
		object = json_object_get(json_record, SJ_UPDATE_TIME);
		
		if (object) {
			pack->records[index].update_time = (unsigned int)json_integer_value(object);
		}
		
		// TODO insert check for value sum here, not sure how yet
		
		object = json_object_get(json_record, SJ_VALUE);
		
		if (object) {
			pack->records[index].value_type = SENML_TYPE_FLOAT;
			pack->records[index].value_f    = json_real_value(object);
			continue;
		}
		
		object = json_object_get(json_record, SJ_BOOL_VALUE);
		
		if (object) {
			pack->records[index].value_type = SENML_TYPE_BOOL;
			
			if (json_is_true(object)) {
				pack->records[index].value_b = true;
			} else if (json_is_false(object)) {
				pack->records[index].value_b = false;
			} else {
				fprintf(stderr, "ERROR: vb is not boolean value\n");
				goto error;
			}
			
			continue;
		}
		
		object = json_object_get(json_record, SJ_STRING_VALUE);
		
		if (object) {
			pack->records[index].value_type = SENML_TYPE_STRING;
			pack->records[index].value_s    = malloc(json_string_length(object) + 1);
			strcpy(pack->records[index].value_s, json_string_value(object));
			continue;
		}
		
		// TODO what about binary values?
	}
	
	json_decref(json_root);
	
	return pack;
	
	error:
	json_decref(json_root);
	free(pack->records);
	free(pack->base_info);
	free(pack);
	return NULL;
}


char *senml_encode_json(const senml_pack_t *pack)
{
	json_t *array         = json_array();
	json_t *base_info_obj = NULL;
	
	char *result;
	
	if (pack->base_info) {
		base_info_obj = json_object();
		
		if (pack->base_info->version)
			json_object_set_new(base_info_obj, SJ_VERSION,
					    json_integer(pack->base_info->version));
		
		if (pack->base_info->base_name)
			json_object_set_new(base_info_obj, SJ_BASE_NAME,
					    json_string(pack->base_info->base_name));
		
		if (pack->base_info->base_time > 0)
			json_object_set_new(base_info_obj, SJ_BASE_TIME,
					    json_real(pack->base_info->base_time));
		
		if (pack->base_info->base_unit)
			json_object_set_new(base_info_obj, SJ_BASE_UNIT,
					    json_string(pack->base_info->base_unit));
		
		// TODO handle base value
		
		json_array_append(array, base_info_obj);
	}
	
	for (unsigned int i = 0; i < pack->num; i++) {
		json_t *object = json_object();
		
		if (pack->records[i].name)
			json_object_set_new(object, SJ_NAME, json_string(pack->records[i].name));
		
		if (pack->records[i].unit)
			json_object_set_new(object, SJ_UNIT, json_string(pack->records[i].unit));
		
		if (pack->records[i].time != 0)
			json_object_set_new(object, SJ_TIME, json_real(pack->records[i].time));
		
		if (pack->records[i].update_time != 0)
			json_object_set_new(object, SJ_UPDATE_TIME,
					    json_real(pack->records[i].update_time));
		
		if (pack->records[i].value_type == SENML_TYPE_FLOAT)
			json_object_set_new(object, SJ_VALUE, json_real(pack->records[i].value_f));
		else if (pack->records[i].value_type == SENML_TYPE_STRING)
			json_object_set_new(object, SJ_STRING_VALUE,
					    json_string(pack->records[i].value_s));
		else if (pack->records[i].value_type == SENML_TYPE_BOOL)
			json_object_set_new(object, SJ_BOOL_VALUE,
					    pack->records[i].value_b ? json_true() : json_false());
// 		else if (pack->records[i].value_type == SENML_TYPE_BINARY)
			// FIXME handle binary value
		
		json_array_append_new(array, object);
	}
	
	result = json_dumps(array, JSON_ENSURE_ASCII | JSON_COMPACT);
	
	json_decref(base_info_obj);
	json_decref(array);
	
	return result;
}


senml_pack_t *senml_decode_cbor(const unsigned char *input, size_t len)
{
	(void)input;
	(void)len;
	return NULL;
}


static inline cbor_item_t *cbor_build_neg_int8(int8_t value)
{
	cbor_item_t *item = cbor_new_int8();
	cbor_mark_negint(item);
	cbor_set_uint8(item, -1*value - 1);
	return item;
}


unsigned char *senml_encode_cbor(const senml_pack_t *pack, size_t *len)
{
	cbor_item_t *array         = cbor_new_indefinite_array();
	cbor_item_t *base_info_map = NULL;
	
	unsigned char *result;
	size_t         buf_size;
	
	if (pack->base_info) {
		base_info_map = cbor_new_indefinite_map();
		
		if (pack->base_info->version)
			cbor_map_add(base_info_map, (struct cbor_pair){
				.key   = cbor_build_neg_int8(SC_VERSION),
				.value = cbor_build_uint8(pack->base_info->version)
			});
		
		if (pack->base_info->base_name)
			cbor_map_add(base_info_map, (struct cbor_pair){
				.key   = cbor_build_neg_int8(SC_BASE_NAME),
				.value = cbor_build_string(pack->base_info->base_name)
			});
		
		if (pack->base_info->base_time > 0)
			cbor_map_add(base_info_map, (struct cbor_pair){
				.key   = cbor_build_neg_int8(SC_BASE_TIME),
				.value = cbor_build_float4(pack->base_info->base_time)
			});
		
		if (pack->base_info->base_unit)
			cbor_map_add(base_info_map, (struct cbor_pair){
				.key   = cbor_build_neg_int8(SC_BASE_UNIT),
				.value = cbor_build_string(pack->base_info->base_unit)
			});
		
		// TODO handle base value
		
		cbor_array_push(array, base_info_map);
	}
	
	for (unsigned int i = 0; i < pack->num; i++) {
		cbor_item_t *map = cbor_new_indefinite_map();
		
		if (pack->records[i].name)
			cbor_map_add(map, (struct cbor_pair){
				.key   = cbor_build_uint8(SC_NAME),
				.value = cbor_build_string(pack->records[i].name)
			});
		
		if (pack->records[i].unit)
			cbor_map_add(map, (struct cbor_pair){
				.key   = cbor_build_uint8(SC_UNIT),
				.value = cbor_build_string(pack->records[i].unit)
			});
		
		if (pack->records[i].time != 0)
			cbor_map_add(map, (struct cbor_pair){
				.key   = cbor_build_uint8(SC_TIME),
				.value = cbor_build_float8(pack->records[i].time)
			});
		
		if (pack->records[i].update_time != 0)
			cbor_map_add(map, (struct cbor_pair){
				.key   = cbor_build_uint8(SC_UPDATE_TIME),
				.value = cbor_build_uint32(pack->records[i].update_time)
			});
		
		if (pack->records[i].value_type == SENML_TYPE_FLOAT)
			cbor_map_add(map, (struct cbor_pair){
				.key   = cbor_build_uint8(SC_VALUE),
				.value = cbor_build_float8(pack->records[i].value_f)
			});
		else if (pack->records[i].value_type == SENML_TYPE_STRING)
			cbor_map_add(map, (struct cbor_pair){
				.key   = cbor_build_uint8(SC_STRING_VALUE),
				.value = cbor_build_string(pack->records[i].value_s)
			});
		else if (pack->records[i].value_type == SENML_TYPE_BOOL)
			cbor_map_add(map, (struct cbor_pair){
				.key   = cbor_build_uint8(SC_UPDATE_TIME),
				.value = cbor_build_bool(pack->records[i].value_b)
			});
// 		else if (pack->records[i].value_type == SENML_TYPE_BINARY)
			// FIXME handle binary value
		
		cbor_array_push(array, map);
	}
	
	*len = cbor_serialize_alloc(array, &result, &buf_size);
	
	return result;
}


void senml_print_base_info(const senml_base_info_t *base_info)
{
	printf("Version:\t%u\n", base_info->version);
	printf("Base Name:\t%s\n", base_info->base_name ? base_info->base_name : "NULL");
	printf("Base Time:\t%f\n", base_info->base_time);
	printf("Base Unit:\t%s\n", base_info->base_unit ? base_info->base_unit : "NULL");

	if (base_info->base_value_type == SENML_TYPE_FLOAT)
		printf("Base Value:\t%f\n", base_info->base_value_f);
	else if (base_info->base_value_type == SENML_TYPE_STRING)
		printf("Base Value:\t%s\n", base_info->base_value_s);
	else if (base_info->base_value_type == SENML_TYPE_BOOL)
		printf("Base Value:\t%s\n", base_info->base_value_b ? "true" : "false");
	
	printf("\n");
}


void senml_print_record(const senml_record_t *record)
{
	printf("Name:\t\t%s\n", record->name ? record->name : "NULL");
	printf("Unit:\t\t%s\n", record->unit ? record->unit : "NULL");
	printf("Time:\t\t%f\n", record->time);
	printf("Update Time:\t%u\n", record->update_time);
	
	if (record->value_type == SENML_TYPE_FLOAT)
		printf("Value:\t\t%f\n", record->value_f);
	else if (record->value_type == SENML_TYPE_STRING)
		printf("Value:\t\t%s\n", record->value_s);
	else if (record->value_type == SENML_TYPE_BOOL)
		printf("Value:\t\t%s\n", record->value_b ? "true" : "false");
	
	printf("\n");
}


void senml_print_pack(const senml_pack_t *pack)
{
	if (pack->base_info)
		senml_print_base_info(pack->base_info);
	
	for (size_t i = 0; i < pack->num; i++) {
		senml_print_record(&(pack->records[i]));
	}
}
