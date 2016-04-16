#include "senml.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>

int senml_decode_json_s(const char *input, senml_pack_t *pack)
{
	(void)input;
	(void)pack;
	return 0;
}


senml_pack_t *senml_decode_json_d(const char *input)
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
	
	senml_base_info_t  *base_info = malloc(sizeof(senml_base_info_t));
	senml_record_t     *records   = malloc(sizeof(senml_record_t) * json_array_size(json_root));
	senml_pack_t       *pack      = malloc(sizeof(senml_pack_t));
	
	pack->base_info = NULL;
	pack->records   = records;
	pack->num       = json_array_size(json_root);
	
	for (unsigned int i = 0; i < json_array_size(json_root); i++) {
		json_t *json_record = json_array_get(json_root, i);
		
		if (!json_is_object(json_record)) {
			fprintf(stderr, "ERROR: record #%d is not valid\n", i+1);
			goto error;
		}
		
		json_t *object = NULL;
		
		object = json_object_get(json_record, SJ_NAME);
		
		if (object) {
			pack->records[i].name = malloc(json_string_length(object) + 1);
			strcpy(pack->records[i].name, json_string_value(object));
		}
		
		object = json_object_get(json_record, SJ_UNIT);
		
		if (object) {
			pack->records[i].unit = malloc(json_string_length(object) + 1);
			strcpy(pack->records[i].unit, json_string_value(object));
		}
		
		object = json_object_get(json_record, SJ_TIME);
		
		if (object) {
			pack->records[i].time = json_real_value(object);
		}
		
		object = json_object_get(json_record, SJ_UPDATE_TIME);
		
		if (object) {
			pack->records[i].update_time = (unsigned int)json_integer_value(object);
		}
		
		// TODO insert check for value sum here, not sure how yet
		
		object = json_object_get(json_record, SJ_VALUE);
		
		if (object) {
			pack->records[i].value_type = SENML_TYPE_FLOAT;
			pack->records[i].value_f    = json_real_value(object);
			continue;
		}
		
		object = json_object_get(json_record, SJ_BOOL_VALUE);
		
		if (object) {
			pack->records[i].value_type = SENML_TYPE_BOOL;
			
			if (json_is_true(object)) {
				pack->records[i].value_b = true;
			} else if (json_is_false(object)) {
				pack->records[i].value_b = false;
			} else {
				fprintf(stderr, "ERROR: vb is not boolean value\n");
				goto error;
			}
			
			continue;
		}
		
		object = json_object_get(json_record, SJ_STRING_VALUE);
		
		if (object) {
			pack->records[i].value_type = SENML_TYPE_STRING;
			pack->records[i].value_s    = malloc(json_string_length(object) + 1);
			strcpy(pack->records[i].value_s, json_string_value(object));
			continue;
		}
		
		// TODO what about binary values?
	}
	
	return pack;
	
	error:
	free(base_info);
	free(records);
	free(pack);
	return NULL;
}


int senml_encode_json_s(const senml_pack_t *pack, char *output, size_t len)
{
	(void)pack;
	(void)output;
	(void)len;
	return 0;
}


char *senml_encode_json_d(const senml_pack_t *pack)
{
	(void)pack;
	return NULL;
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
	printf("Name:\t%s\n", record->name ? record->name : "NULL");
	printf("Unit:\t%s\n", record->unit ? record->unit : "NULL");
	printf("Time:\t%f\n", record->time);
	printf("Update Time:\t%u\n", record->update_time);
	
	if (record->value_type == SENML_TYPE_FLOAT)
		printf("Value:\t%f\n", record->value_f);
	else if (record->value_type == SENML_TYPE_STRING)
		printf("Value:\t%s\n", record->value_s);
	else if (record->value_type == SENML_TYPE_BOOL)
		printf("Value:\t%s\n", record->value_b ? "true" : "false");
	
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
