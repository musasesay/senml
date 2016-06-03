#ifndef SENML_H
#define SENML_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


#define SENML_SUPPORTED_VERSION  (5)


/*
 * Definitions of valid SenML keys in JSON representation
 */
#define SJ_VERSION       "bver"
#define SJ_BASE_NAME     "bn"
#define SJ_BASE_TIME     "bt"
#define SJ_BASE_UNIT     "bu"
#define SJ_BASE_VALUE    "bv"
#define SJ_NAME          "n"
#define SJ_UNIT          "u"
#define SJ_VALUE         "v"
#define SJ_STRING_VALUE  "vs"
#define SJ_BOOL_VALUE    "vb"
#define SJ_VALUE_SUM     "s"
#define SJ_TIME          "t"
#define SJ_UPDATE_TIME   "ut"
#define SJ_DATA_VALUE    "vd"


/*
 * Definitions of valid SenML keys in CBOR representation
 */
#define SC_VERSION       (-1)
#define SC_BASE_NAME     (-2)
#define SC_BASE_TIME     (-3)
#define SC_BASE_UNIT     (-4)
#define SC_BASE_VALUE    (-5)
#define SC_NAME          (0)
#define SC_UNIT          (1)
#define SC_VALUE         (2)
#define SC_STRING_VALUE  (3)
#define SC_BOOL_VALUE    (4)
#define SC_VALUE_SUM     (5)
#define SC_TIME          (6)
#define SC_UPDATE_TIME   (7)
#define SC_DATA_VALUE    (8)


typedef enum {
	SENML_TYPE_UNDEF  = 0,
	SENML_TYPE_FLOAT  = 1,
	SENML_TYPE_STRING = 2,
	SENML_TYPE_BOOL   = 3,
	SENML_TYPE_BINARY = 4
} senml_value_type_t;


typedef struct {
	uint8_t *p;
	size_t   len;
} senml_bin_data_t;


typedef struct {
	uint8_t             version;    // defaults to 5
	char               *base_name;  // optional, may be NULL
	double              base_time;  // optional, may be 0
	char               *base_unit;  // optional, may be NULL
	senml_value_type_t  base_value_type;
	union {
		double            base_value_f;
		char             *base_value_s;
		bool              base_value_b;    // FIXME is this sensible?
		senml_bin_data_t  base_value_bin;  // FIXME is this sensible?
	};
} senml_base_info_t;


typedef struct {
	char              *name;
	char              *unit;
	double             time;
	unsigned int       update_time;  // draft says this value is given in seconds, but perhaps
					 // fractions of a second should be supported as well?
					 // would have to change unsigned int to double then FIXME
	// xxx value_sum  FIXME clear definition missing
	senml_value_type_t value_type;
	union {
		double            value_f;
		char             *value_s;
		bool              value_b;
		senml_bin_data_t  value_bin;  // FIXME don't know how to handle this yet
	};
} senml_record_t;


typedef struct {
	senml_base_info_t  *base_info;
	senml_record_t     *records;
	size_t              num;
} senml_pack_t;



////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Decodes a SenML pack in JSON format and writes the results in \p pack. The memory necessary to 
 * store the decoded records must be allocated in advance.
 * @param[in]     input The JSON document containing the SenML pack.
 * @param[in,out] pack The <code>senml_pack_t</code> structure in which the records will be stored.
 * <code>pack->num</code> must indicate the number of <code>senml_record_t</code> elements
 * <code>pack->records</code> can hold.
 * @return 0 on success, -1 if \p input is not a valid SenML document, or -2 if \p input contains
 * more records than <code>pack->records</code> can hold.
 */
// int senml_decode_json_s(const char *input, senml_pack_t *pack);


/**
 * Creates a SenML document in JSON format. The memory necessary to store the resulting JSON
 * document must be allocated in advance.
 * @param[in] pack The <code>senml_pack_t</code> elements that contains the SenML records.
 * @param[in,out] output The char buffer that will contain the finished JSON document.
 * @param[in] len The size of the allocated memory that \p output points to in bytes.
 * @return 0 on success, -1 if \p pack contains invalid data or options, or -2 if not enough memory
 * was allocated.
 */
// int senml_encode_json_s(const senml_pack_t *pack, char *output, size_t len);
////////////////////////////////////////////////////////////////////////////////////////////////////



/**
 * Decodes a SenML pack in JSON format and writes the results in \p pack. The memory necessary to 
 * store the decoded records will be allocated automatically.
 * @param[in] input The JSON document containing the SenML pack.
 * @return A valid pointer to a <code>senml_pack_t</code> elements, or NULL on failure.
 */
senml_pack_t *senml_decode_json(const char *input, size_t len);


/**
 * Creates a SenML document in JSON format. The memory necessary to store the resulting JSON 
 * document will be allocated automatically.
 * @param[in] pack The <code>senml_pack_t</code> elements that contains the SenML records.
 * @return A valid pointer to the finished JSON document, or NULL on failure.
 */
char *senml_encode_json(const senml_pack_t *pack);


/**
 * Decodes a SenML pack in CBOR format and writes the results in \p pack. The memory necessary to 
 * store the decoded records will be allocated automatically.
 * @param[in] input The CBOR document containing the SenML pack.
 * @param[in] len The length of \p input in bytes.
 * @return A valid pointer to a <code>senml_pack_t</code> elements, or NULL on failure.
 */
senml_pack_t *senml_decode_cbor(const unsigned char *input, size_t len);


/**
 * Creates a SenML document in CBOR format. The memory necessary to store the resulting CBOR
 * document will be allocated automatically.
 * @param[in] pack The <code>senml_pack_t</code> elements that contains the SenML records.
 * @param[out] len The length of the resulting CBOR document.
 * @return A valid pointer to the finished JSON document, or NULL on failure.
 */
unsigned char *senml_encode_cbor(const senml_pack_t *pack, size_t *len);


/**
 * Pretty prints the base info.
 * @param[in] base_info
 */
void senml_print_base_info(const senml_base_info_t *base_info);


/**
 * Pretty prints a record.
 * @param[in] record
 */
void senml_print_record(const senml_record_t *record);


/**
 * Pretty prints a pack (base info if available and all records).
 * @param[in] pack
 */
void senml_print_pack(const senml_pack_t *pack);


#endif /* SENML_H */
