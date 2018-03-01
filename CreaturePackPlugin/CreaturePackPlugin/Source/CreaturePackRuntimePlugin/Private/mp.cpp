#include "mp.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


namespace mpMini {
	enum {
		MSG_MINI_TYPE_POSITIVE_FIXNUM, /*  0 */
		MSG_MINI_TYPE_FIXMAP,          /*  1 */
		MSG_MINI_TYPE_FIXARRAY,        /*  2 */
		MSG_MINI_TYPE_FIXSTR,          /*  3 */
		MSG_MINI_TYPE_NIL,             /*  4 */
		MSG_MINI_TYPE_BOOLEAN,         /*  5 */
		MSG_MINI_TYPE_BIN8,            /*  6 */
		MSG_MINI_TYPE_BIN16,           /*  7 */
		MSG_MINI_TYPE_BIN32,           /*  8 */
		MSG_MINI_TYPE_EXT8,            /*  9 */
		MSG_MINI_TYPE_EXT16,           /* 10 */
		MSG_MINI_TYPE_EXT32,           /* 11 */
		MSG_MINI_TYPE_FLOAT,           /* 12 */
		MSG_MINI_TYPE_DOUBLE,          /* 13 */
		MSG_MINI_TYPE_UINT8,           /* 14 */
		MSG_MINI_TYPE_UINT16,          /* 15 */
		MSG_MINI_TYPE_UINT32,          /* 16 */
		MSG_MINI_TYPE_UINT64,          /* 17 */
		MSG_MINI_TYPE_SINT8,           /* 18 */
		MSG_MINI_TYPE_SINT16,          /* 19 */
		MSG_MINI_TYPE_SINT32,          /* 20 */
		MSG_MINI_TYPE_SINT64,          /* 21 */
		MSG_MINI_TYPE_FIXEXT1,         /* 22 */
		MSG_MINI_TYPE_FIXEXT2,         /* 23 */
		MSG_MINI_TYPE_FIXEXT4,         /* 24 */
		MSG_MINI_TYPE_FIXEXT8,         /* 25 */
		MSG_MINI_TYPE_FIXEXT16,        /* 26 */
		MSG_MINI_TYPE_STR8,            /* 27 */
		MSG_MINI_TYPE_STR16,           /* 28 */
		MSG_MINI_TYPE_STR32,           /* 29 */
		MSG_MINI_TYPE_ARRAY16,         /* 30 */
		MSG_MINI_TYPE_ARRAY32,         /* 31 */
		MSG_MINI_TYPE_MAP16,           /* 32 */
		MSG_MINI_TYPE_MAP32,           /* 33 */
		MSG_MINI_TYPE_NEGATIVE_FIXNUM  /* 34 */
	};

	enum {
		POSITIVE_FIXNUM_MARKER = 0x00,
		FIXMAP_MARKER = 0x80,
		FIXARRAY_MARKER = 0x90,
		FIXSTR_MARKER = 0xA0,
		NIL_MARKER = 0xC0,
		FALSE_MARKER = 0xC2,
		TRUE_MARKER = 0xC3,
		BIN8_MARKER = 0xC4,
		BIN16_MARKER = 0xC5,
		BIN32_MARKER = 0xC6,
		EXT8_MARKER = 0xC7,
		EXT16_MARKER = 0xC8,
		EXT32_MARKER = 0xC9,
		FLOAT_MARKER = 0xCA,
		DOUBLE_MARKER = 0xCB,
		U8_MARKER = 0xCC,
		U16_MARKER = 0xCD,
		U32_MARKER = 0xCE,
		U64_MARKER = 0xCF,
		S8_MARKER = 0xD0,
		S16_MARKER = 0xD1,
		S32_MARKER = 0xD2,
		S64_MARKER = 0xD3,
		FIXEXT1_MARKER = 0xD4,
		FIXEXT2_MARKER = 0xD5,
		FIXEXT4_MARKER = 0xD6,
		FIXEXT8_MARKER = 0xD7,
		FIXEXT16_MARKER = 0xD8,
		STR8_MARKER = 0xD9,
		STR16_MARKER = 0xDA,
		STR32_MARKER = 0xDB,
		ARRAY16_MARKER = 0xDC,
		ARRAY32_MARKER = 0xDD,
		MAP16_MARKER = 0xDE,
		MAP32_MARKER = 0xDF,
		NEGATIVE_FIXNUM_MARKER = 0xE0
	};

	enum {
		FIXARRAY_SIZE = 0xF,
		FIXMAP_SIZE = 0xF,
		FIXSTR_SIZE = 0x1F
	};

	enum {
		ERROR_NONE,
		STR_DATA_LENGTH_TOO_LONG_ERROR,
		BIN_DATA_LENGTH_TOO_LONG_ERROR,
		ARRAY_LENGTH_TOO_LONG_ERROR,
		MAP_LENGTH_TOO_LONG_ERROR,
		INPUT_VALUE_TOO_LARGE_ERROR,
		FIXED_VALUE_WRITING_ERROR,
		TYPE_MARKER_READING_ERROR,
		TYPE_MARKER_WRITING_ERROR,
		DATA_READING_ERROR,
		DATA_WRITING_ERROR,
		EXT_TYPE_READING_ERROR,
		EXT_TYPE_WRITING_ERROR,
		INVALID_TYPE_ERROR,
		LENGTH_READING_ERROR,
		LENGTH_WRITING_ERROR,
		ERROR_MAX
	};

	const char *mp_error_strings[ERROR_MAX + 1] = {
		"No Error",
		"Specified string data length is too long (> 0xFFFFFFFF)",
		"Specified binary data length is too long (> 0xFFFFFFFF)",
		"Specified array length is too long (> 0xFFFFFFFF)",
		"Specified map length is too long (> 0xFFFFFFFF)",
		"Input value is too large",
		"Error writing fixed value",
		"Error reading type marker",
		"Error writing type marker",
		"Error reading packed data",
		"Error writing packed data",
		"Error reading ext type",
		"Error writing ext type",
		"Invalid type",
		"Error reading size",
		"Error writing size",
		"Max Error"
	};

	static const int32_t _i = 1;
#define is_bigendian() ((*(char *)&_i) == 0)

	static uint16_t be16(uint16_t x) {
		char *b = (char *)&x;

		if (!is_bigendian()) {
			char swap = 0;

			swap = b[0];
			b[0] = b[1];
			b[1] = swap;
		}

		return x;
	}

	static uint32_t be32(uint32_t x) {
		char *b = (char *)&x;

		if (!is_bigendian()) {
			char swap = 0;

			swap = b[0];
			b[0] = b[3];
			b[3] = swap;

			swap = b[1];
			b[1] = b[2];
			b[2] = swap;
		}

		return x;
	}

	static uint64_t be64(uint64_t x) {
		char *b = (char *)&x;

		if (!is_bigendian()) {
			char swap = 0;

			swap = b[0];
			b[0] = b[7];
			b[7] = swap;

			swap = b[1];
			b[1] = b[6];
			b[6] = swap;

			swap = b[2];
			b[2] = b[5];
			b[5] = swap;

			swap = b[3];
			b[3] = b[4];
			b[4] = swap;
		}

		return x;
	}

	static float befloat(float x) {
		char *b = (char *)&x;

		if (!is_bigendian()) {
			char swap = 0;

			swap = b[0];
			b[0] = b[3];
			b[3] = swap;

			swap = b[1];
			b[1] = b[2];
			b[2] = swap;
		}

		return x;
	}

	static double bedouble(double x) {
		char *b = (char *)&x;

		if (!is_bigendian()) {
			char swap = 0;

			swap = b[0];
			b[0] = b[7];
			b[7] = swap;

			swap = b[1];
			b[1] = b[6];
			b[6] = swap;

			swap = b[2];
			b[2] = b[5];
			b[5] = swap;

			swap = b[3];
			b[3] = b[4];
			b[4] = swap;
		}

		return x;
	}

	bool 
	msg_mini::read_one_byte(uint8_t *x) {
		return read(x, sizeof(uint8_t));
	}

	bool 
	msg_mini::read_marker_type(uint8_t *marker) {
		if (read_one_byte(marker))
			return true;

		error = TYPE_MARKER_READING_ERROR;
		return false;
	}

	const char* 
	msg_mini::get_strerror() const
	{
		if (error > ERROR_NONE && error < ERROR_MAX)
			return mp_error_strings[error];

		return "";
	}

	bool 
	msg_mini::msg_mini_build_generic_objects()
	{
		// Assumes you must have packed everything into a main array
		generic_data.clear();
		uint32_t main_obj_num = 0;
		bool can_read = msg_mini_read_array(&main_obj_num);
		if (!can_read)
		{
			error = INVALID_TYPE_ERROR;
			return false;
		}

		for (size_t main_idx = 0; main_idx < (size_t)main_obj_num; main_idx++)
		{
			// Determine object type
			store_read_pos();
			msg_mini_object read_msg_obj;
			msg_mini_read_object(&read_msg_obj);
			restore_read_pos();

			// Parse through valid object types
			if (msg_mini_object_is_array(&read_msg_obj))
			{
				msg_mini_object test_msg_obj;
				uint32_t array_size;
				msg_mini_read_array(&array_size);

				// determine array type
				store_read_pos();
				msg_mini_read_object(&test_msg_obj);
				restore_read_pos();

				msg_mini_generic_data new_generic_data(0);

				if (msg_mini_object_is_uint(&test_msg_obj))
				{
					new_generic_data.type = MSG_MINI_GENERIC_ARRAY_INT_TYPE;
					new_generic_data.int_array_val.resize(array_size);

					for (size_t j = 0; j < (size_t)array_size; j++)
					{
						msg_mini_read_int(&new_generic_data.int_array_val[j]);
					}
				}
				else if (msg_mini_object_is_float(&test_msg_obj))
				{
					new_generic_data.type = MSG_MINI_GENERIC_ARRAY_FLOAT_TYPE;
					new_generic_data.float_array_val.resize(array_size);

					for (size_t j = 0; j < (size_t)array_size; j++)
					{
						msg_mini_read_real(&new_generic_data.float_array_val[j]);
					}
				}
				else if (msg_mini_object_is_str(&test_msg_obj))
				{
					new_generic_data.type = MSG_MINI_GENERIC_ARRAY_STRING_TYPE;
					new_generic_data.str_array_val.resize(array_size);

					for (size_t j = 0; j < (size_t)array_size; j++)
					{
						msg_mini_read_str(new_generic_data.str_array_val[j]);
					}
				}

				generic_data.push_back(new_generic_data);
			}
			else if (msg_mini_object_is_bin(&read_msg_obj))
			{
				uint32_t array_size;
				msg_mini_read_bin(&array_size);
				msg_mini_generic_data new_generic_data(0);

				new_generic_data.type = MSG_MINI_GENERIC_ARRAY_BYTE_TYPE;
				new_generic_data.byte_array_val.resize(array_size);

				readBytesChunk(array_size, 
					[&new_generic_data](int idx, uint8_t data)
				{
					new_generic_data.byte_array_val[idx] = data;
				});
				generic_data.push_back(new_generic_data);
			}
			else if (msg_mini_object_is_float(&read_msg_obj))
			{
				msg_mini_generic_data new_generic_data(MSG_MINI_GENERIC_FLOAT_TYPE);
				msg_mini_read_real(&new_generic_data.float_val);
				generic_data.push_back(new_generic_data);
			}
			else if (msg_mini_object_is_int(&read_msg_obj))
			{
				msg_mini_generic_data new_generic_data(MSG_MINI_GENERIC_INT_TYPE);
				msg_mini_read_int(&new_generic_data.int_val);
				generic_data.push_back(new_generic_data);
			}
			else if (msg_mini_object_is_str(&read_msg_obj))
			{
				msg_mini_generic_data new_generic_data(MSG_MINI_GENERIC_STRING_TYPE);
				msg_mini_read_str(new_generic_data.string_val);
				generic_data.push_back(new_generic_data);
			}
			else {
				// not supported type so just stop
				error = INVALID_TYPE_ERROR;
				return false;
			}
		}

		return true;
	}

	const std::vector<msg_mini_generic_data>& msg_mini::msg_mini_get_generic_objects() const
	{
		return generic_data;
	}

	bool 
	msg_mini::msg_mini_read_pfix(uint8_t *c) 
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		if (obj.type != MSG_MINI_TYPE_POSITIVE_FIXNUM) {
			error = INVALID_TYPE_ERROR;
			return false;
		}

		*c = obj.as.u8;
		return true;
	}

	bool 
	msg_mini::msg_mini_read_nfix(int8_t *c) 
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		if (obj.type != MSG_MINI_TYPE_NEGATIVE_FIXNUM) {
			error = INVALID_TYPE_ERROR;
			return false;
		}

		*c = obj.as.s8;
		return true;
	}

	bool 
	msg_mini::msg_mini_read_sfix(int8_t *c) 
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		switch (obj.type) {
		case MSG_MINI_TYPE_POSITIVE_FIXNUM:
		case MSG_MINI_TYPE_NEGATIVE_FIXNUM:
			*c = obj.as.s8;
			return true;
		default:
			error = INVALID_TYPE_ERROR;
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_read_s8(int8_t *c) 
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		if (obj.type != MSG_MINI_TYPE_SINT8) {
			error = INVALID_TYPE_ERROR;
			return false;
		}

		*c = obj.as.s8;
		return true;
	}

	bool 
	msg_mini::msg_mini_read_s16(int16_t *s) 
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		if (obj.type != MSG_MINI_TYPE_SINT16) {
			error = INVALID_TYPE_ERROR;
			return false;
		}

		*s = obj.as.s16;
		return true;
	}

	bool 
	msg_mini::msg_mini_read_s32(int32_t *i) 
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		if (obj.type != MSG_MINI_TYPE_SINT32) {
			error = INVALID_TYPE_ERROR;
			return false;
		}

		*i = obj.as.s32;
		return true;
	}

	bool 
	msg_mini::msg_mini_read_s64(int64_t *l) 
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		if (obj.type != MSG_MINI_TYPE_SINT64) {
			error = INVALID_TYPE_ERROR;
			return false;
		}

		*l = obj.as.s64;
		return true;
	}

	bool 
	msg_mini::msg_mini_read_int(int32_t *i) 
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		switch (obj.type) {
		case MSG_MINI_TYPE_POSITIVE_FIXNUM:
		case MSG_MINI_TYPE_NEGATIVE_FIXNUM:
		case MSG_MINI_TYPE_SINT8:
			*i = obj.as.s8;
			return true;
		case MSG_MINI_TYPE_UINT8:
			*i = obj.as.u8;
			return true;
		case MSG_MINI_TYPE_SINT16:
			*i = obj.as.s16;
			return true;
		case MSG_MINI_TYPE_UINT16:
			*i = obj.as.u16;
			return true;
		case MSG_MINI_TYPE_SINT32:
			*i = obj.as.s32;
			return true;
		case MSG_MINI_TYPE_UINT32:
			if (obj.as.u32 <= 2147483647) {
				*i = obj.as.u32;
				return true;
			}
		default:
			error = INVALID_TYPE_ERROR;
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_read_ufix(uint8_t *c) 
	{
		return msg_mini_read_pfix(c);
	}

	bool 
	msg_mini::msg_mini_read_u8(uint8_t *c) 
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		if (obj.type != MSG_MINI_TYPE_UINT8) {
			error = INVALID_TYPE_ERROR;
			return false;
		}

		*c = obj.as.u8;
		return true;
	}

	bool 
	msg_mini::msg_mini_read_u16(uint16_t *s) 
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		if (obj.type != MSG_MINI_TYPE_UINT16) {
			error = INVALID_TYPE_ERROR;
			return false;
		}

		*s = obj.as.u16;
		return true;
	}

	bool 
	msg_mini::msg_mini_read_u32(uint32_t *i) 
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		if (obj.type != MSG_MINI_TYPE_UINT32) {
			error = INVALID_TYPE_ERROR;
			return false;
		}

		*i = obj.as.u32;
		return true;
	}

	bool 
	msg_mini::msg_mini_read_u64(uint64_t *l) 
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		if (obj.type != MSG_MINI_TYPE_UINT64) {
			error = INVALID_TYPE_ERROR;
			return false;
		}

		*l = obj.as.u64;
		return true;
	}

	bool 
	msg_mini::msg_mini_read_float(float *f) 
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		if (obj.type != MSG_MINI_TYPE_FLOAT) {
			error = INVALID_TYPE_ERROR;
			return false;
		}

		*f = obj.as.flt;

		return true;
	}

	bool 
	msg_mini::msg_mini_read_double(double *d) 
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		if (obj.type != MSG_MINI_TYPE_DOUBLE) {
			error = INVALID_TYPE_ERROR;
			return false;
		}

		*d = obj.as.dbl;

		return true;
	}

	bool 
	msg_mini::msg_mini_read_real(float *d) 
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		switch (obj.type) {
		case MSG_MINI_TYPE_FLOAT:
			*d = (float)obj.as.flt;
			return true;
		case MSG_MINI_TYPE_DOUBLE:
			*d = (float)obj.as.dbl;
			return true;
		default:
			error = INVALID_TYPE_ERROR;
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_read_nil() 
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		if (obj.type == MSG_MINI_TYPE_NIL)
			return true;

		error = INVALID_TYPE_ERROR;
		return false;
	}

	bool 
	msg_mini::msg_mini_read_bool(bool *b) 
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		if (obj.type != MSG_MINI_TYPE_BOOLEAN) {
			error = INVALID_TYPE_ERROR;
			return false;
		}

		if (obj.as.boolean)
			*b = true;
		else
			*b = false;

		return true;
	}

	bool 
	msg_mini::msg_mini_read_str_size(uint32_t *size) 
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		switch (obj.type) {
		case MSG_MINI_TYPE_FIXSTR:
		case MSG_MINI_TYPE_STR8:
		case MSG_MINI_TYPE_STR16:
		case MSG_MINI_TYPE_STR32:
			*size = obj.as.str_size;
			return true;
		default:
			error = INVALID_TYPE_ERROR;
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_read_str(std::string& data)
	{
		uint32_t str_size = 0;

		if (!msg_mini_read_str_size(&str_size))
			return false;

		std::shared_ptr<char> raw_data = std::shared_ptr<char>(new char[str_size + 1], std::default_delete<char[]>());

		if (!read(raw_data.get(), str_size)) {
			error = DATA_READING_ERROR;
			return false;
		}

		raw_data.get()[str_size] = 0;
		data = std::string(raw_data.get());

		return true;
	}

	bool 
	msg_mini::msg_mini_read_bin(uint32_t *size)
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		switch (obj.type) {
		case MSG_MINI_TYPE_BIN8:
		case MSG_MINI_TYPE_BIN16:
		case MSG_MINI_TYPE_BIN32:
			*size = obj.as.array_size;
			return true;
		default:
			error = INVALID_TYPE_ERROR;
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_read_array(uint32_t *size) 
	{
		msg_mini_object obj;

		if (!msg_mini_read_object(&obj))
			return false;

		switch (obj.type) {
		case MSG_MINI_TYPE_FIXARRAY:
		case MSG_MINI_TYPE_ARRAY16:
		case MSG_MINI_TYPE_ARRAY32:
			*size = obj.as.array_size;
			return true;
		default:
			error = INVALID_TYPE_ERROR;
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_read_object(msg_mini_object *obj)
	{
		uint8_t type_marker = 0;

		if (!read_marker_type(&type_marker))
			return false;

		if (type_marker <= 0x7F) {
			obj->type = MSG_MINI_TYPE_POSITIVE_FIXNUM;
			obj->as.u8 = type_marker;
		}
		else if (type_marker <= 0x8F) {
			obj->type = MSG_MINI_TYPE_FIXMAP;
			obj->as.map_size = type_marker & FIXMAP_SIZE;
		}
		else if (type_marker <= 0x9F) {
			obj->type = MSG_MINI_TYPE_FIXARRAY;
			obj->as.array_size = type_marker & FIXARRAY_SIZE;
		}
		else if (type_marker <= 0xBF) {
			obj->type = MSG_MINI_TYPE_FIXSTR;
			obj->as.str_size = type_marker & FIXSTR_SIZE;
		}
		else if (type_marker == NIL_MARKER) {
			obj->type = MSG_MINI_TYPE_NIL;
			obj->as.u8 = 0;
		}
		else if (type_marker == FALSE_MARKER) {
			obj->type = MSG_MINI_TYPE_BOOLEAN;
			obj->as.boolean = false;
		}
		else if (type_marker == TRUE_MARKER) {
			obj->type = MSG_MINI_TYPE_BOOLEAN;
			obj->as.boolean = true;
		}
		else if (type_marker == FLOAT_MARKER) {
			obj->type = MSG_MINI_TYPE_FLOAT;
			if (!read(&obj->as.flt, sizeof(float))) {
				error = DATA_READING_ERROR;
				return false;
			}
			obj->as.flt = befloat(obj->as.flt);
		}
		else if (type_marker == DOUBLE_MARKER) {
			obj->type = MSG_MINI_TYPE_DOUBLE;
			if (!read(&obj->as.dbl, sizeof(double))) {
				error = DATA_READING_ERROR;
				return false;
			}
			obj->as.dbl = bedouble(obj->as.dbl);
		}
		else if (type_marker == U8_MARKER) {
			obj->type = MSG_MINI_TYPE_UINT8;
			if (!read(&obj->as.u8, sizeof(uint8_t))) {
				error = DATA_READING_ERROR;
				return false;
			}
		}
		else if (type_marker == U16_MARKER) {
			obj->type = MSG_MINI_TYPE_UINT16;
			if (!read(&obj->as.u16, sizeof(uint16_t))) {
				error = DATA_READING_ERROR;
				return false;
			}
			obj->as.u16 = be16(obj->as.u16);
		}
		else if (type_marker == U32_MARKER) {
			obj->type = MSG_MINI_TYPE_UINT32;
			if (!read(&obj->as.u32, sizeof(uint32_t))) {
				error = DATA_READING_ERROR;
				return false;
			}
			obj->as.u32 = be32(obj->as.u32);
		}
		else if (type_marker == U64_MARKER) {
			obj->type = MSG_MINI_TYPE_UINT64;
			if (!read(&obj->as.u64, sizeof(uint64_t))) {
				error = DATA_READING_ERROR;
				return false;
			}
			obj->as.u64 = be64(obj->as.u64);
		}
		else if (type_marker == S8_MARKER) {
			obj->type = MSG_MINI_TYPE_SINT8;
			if (!read(&obj->as.s8, sizeof(int8_t))) {
				error = DATA_READING_ERROR;
				return false;
			}
		}
		else if (type_marker == S16_MARKER) {
			obj->type = MSG_MINI_TYPE_SINT16;
			if (!read(&obj->as.s16, sizeof(int16_t))) {
				error = DATA_READING_ERROR;
				return false;
			}
			obj->as.s16 = be16(obj->as.s16);
		}
		else if (type_marker == S32_MARKER) {
			obj->type = MSG_MINI_TYPE_SINT32;
			if (!read(&obj->as.s32, sizeof(int32_t))) {
				error = DATA_READING_ERROR;
				return false;
			}
			obj->as.s32 = be32(obj->as.s32);
		}
		else if (type_marker == S64_MARKER) {
			obj->type = MSG_MINI_TYPE_SINT64;
			if (!read(&obj->as.s64, sizeof(int64_t))) {
				error = DATA_READING_ERROR;
				return false;
			}
			obj->as.s64 = be64(obj->as.s64);
		}
		else if (type_marker == STR8_MARKER) {
			obj->type = MSG_MINI_TYPE_STR8;
			if (!read(&obj->as.u8, sizeof(uint8_t))) {
				error = DATA_READING_ERROR;
				return false;
			}
			obj->as.str_size = obj->as.u8;
		}
		else if (type_marker == STR16_MARKER) {
			obj->type = MSG_MINI_TYPE_STR16;
			if (!read(&obj->as.u16, sizeof(uint16_t))) {
				error = DATA_READING_ERROR;
				return false;
			}
			obj->as.str_size = be16(obj->as.u16);
		}
		else if (type_marker == STR32_MARKER) {
			obj->type = MSG_MINI_TYPE_STR32;
			if (!read(&obj->as.u32, sizeof(uint32_t))) {
				error = DATA_READING_ERROR;
				return false;
			}
			obj->as.str_size = be32(obj->as.u32);
		}
		else if (type_marker == ARRAY16_MARKER) {
			obj->type = MSG_MINI_TYPE_ARRAY16;
			if (!read(&obj->as.u16, sizeof(uint16_t))) {
				error = DATA_READING_ERROR;
				return false;
			}
			obj->as.array_size = be16(obj->as.u16);
		}
		else if (type_marker == ARRAY32_MARKER) {
			obj->type = MSG_MINI_TYPE_ARRAY32;
			if (!read(&obj->as.u32, sizeof(uint32_t))) {
				error = DATA_READING_ERROR;
				return false;
			}
			obj->as.array_size = be32(obj->as.u32);
		}
		else if (type_marker == BIN16_MARKER) {
			obj->type = MSG_MINI_TYPE_BIN16;
			if (!read(&obj->as.u16, sizeof(uint16_t))) {
				error = DATA_READING_ERROR;
				return false;
			}
			obj->as.array_size = be16(obj->as.u16);
		}		
		else if (type_marker == BIN32_MARKER) {
			obj->type = MSG_MINI_TYPE_BIN32;
			if (!read(&obj->as.u32, sizeof(uint32_t))) {
				error = DATA_READING_ERROR;
				return false;
			}
			obj->as.array_size = be32(obj->as.u32);
		}
		else if (type_marker == MAP16_MARKER) {
			obj->type = MSG_MINI_TYPE_MAP16;
			if (!read(&obj->as.u16, sizeof(uint16_t))) {
				error = DATA_READING_ERROR;
				return false;
			}
			obj->as.map_size = be16(obj->as.u16);
		}
		else if (type_marker == MAP32_MARKER) {
			obj->type = MSG_MINI_TYPE_MAP32;
			if (!read(&obj->as.u32, sizeof(uint32_t))) {
				error = DATA_READING_ERROR;
				return false;
			}
			obj->as.map_size = be32(obj->as.u32);
		}
		else if (type_marker >= NEGATIVE_FIXNUM_MARKER) {
			obj->type = MSG_MINI_TYPE_NEGATIVE_FIXNUM;
			obj->as.s8 = type_marker;
		}
		else {
			error = INVALID_TYPE_ERROR;
			return false;
		}

		return true;
	}

	bool 
	msg_mini::msg_mini_object_is_char(msg_mini_object *obj) 
	{
		switch (obj->type) {
		case MSG_MINI_TYPE_NEGATIVE_FIXNUM:
		case MSG_MINI_TYPE_SINT8:
			return true;
		default:
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_object_is_short(msg_mini_object *obj) 
	{
		switch (obj->type) {
		case MSG_MINI_TYPE_NEGATIVE_FIXNUM:
		case MSG_MINI_TYPE_SINT8:
		case MSG_MINI_TYPE_SINT16:
			return true;
		default:
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_object_is_int(msg_mini_object *obj) 
	{
		switch (obj->type) {
		case MSG_MINI_TYPE_NEGATIVE_FIXNUM:
		case MSG_MINI_TYPE_SINT8:
		case MSG_MINI_TYPE_SINT16:
		case MSG_MINI_TYPE_SINT32:
			return true;
		default:
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_object_is_long(msg_mini_object *obj) 
	{
		switch (obj->type) {
		case MSG_MINI_TYPE_NEGATIVE_FIXNUM:
		case MSG_MINI_TYPE_SINT8:
		case MSG_MINI_TYPE_SINT16:
		case MSG_MINI_TYPE_SINT32:
		case MSG_MINI_TYPE_SINT64:
			return true;
		default:
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_object_is_sinteger(msg_mini_object *obj) 
	{
		return msg_mini_object_is_long(obj);
	}

	bool 
	msg_mini::msg_mini_object_is_uchar(msg_mini_object *obj) 
	{
		switch (obj->type) {
		case MSG_MINI_TYPE_POSITIVE_FIXNUM:
		case MSG_MINI_TYPE_UINT8:
			return true;
		default:
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_object_is_ushort(msg_mini_object *obj) 
	{
		switch (obj->type) {
		case MSG_MINI_TYPE_POSITIVE_FIXNUM:
		case MSG_MINI_TYPE_UINT8:
			return true;
		case MSG_MINI_TYPE_UINT16:
			return true;
		default:
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_object_is_uint(msg_mini_object *obj) 
	{
		switch (obj->type) {
		case MSG_MINI_TYPE_POSITIVE_FIXNUM:
		case MSG_MINI_TYPE_UINT8:
		case MSG_MINI_TYPE_UINT16:
		case MSG_MINI_TYPE_UINT32:
			return true;
		default:
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_object_is_ulong(msg_mini_object *obj) 
	{
		switch (obj->type) {
		case MSG_MINI_TYPE_POSITIVE_FIXNUM:
		case MSG_MINI_TYPE_UINT8:
		case MSG_MINI_TYPE_UINT16:
		case MSG_MINI_TYPE_UINT32:
		case MSG_MINI_TYPE_UINT64:
			return true;
		default:
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_object_is_uinteger(msg_mini_object *obj) 
	{
		return msg_mini_object_is_ulong(obj);
	}

	bool 
	msg_mini::msg_mini_object_is_float(msg_mini_object *obj) 
	{
		if (obj->type == MSG_MINI_TYPE_FLOAT)
			return true;

		return false;
	}

	bool 
	msg_mini::msg_mini_object_is_double(msg_mini_object *obj) 
	{
		if (obj->type == MSG_MINI_TYPE_DOUBLE)
			return true;

		return false;
	}

	bool
	msg_mini::msg_mini_object_is_nil(msg_mini_object *obj) 
	{
		if (obj->type == MSG_MINI_TYPE_NIL)
			return true;

		return false;
	}

	bool 
	msg_mini::msg_mini_object_is_bool(msg_mini_object *obj) 
	{
		if (obj->type == MSG_MINI_TYPE_BOOLEAN)
			return true;

		return false;
	}

	bool 
	msg_mini::msg_mini_object_is_str(msg_mini_object *obj) 
	{
		switch (obj->type) {
		case MSG_MINI_TYPE_FIXSTR:
		case MSG_MINI_TYPE_STR8:
		case MSG_MINI_TYPE_STR16:
		case MSG_MINI_TYPE_STR32:
			return true;
		default:
			return false;
		}
	}

	bool
	msg_mini::msg_mini_object_is_bin(msg_mini_object *obj)
	{
		switch (obj->type) {
		case MSG_MINI_TYPE_BIN8:
		case MSG_MINI_TYPE_BIN16:
		case MSG_MINI_TYPE_BIN32:
			return true;
		default:
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_object_is_array(msg_mini_object *obj) 
	{
		switch (obj->type) {
		case MSG_MINI_TYPE_FIXARRAY:
		case MSG_MINI_TYPE_ARRAY16:
		case MSG_MINI_TYPE_ARRAY32:
			return true;
		default:
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_object_is_map(msg_mini_object *obj) 
	{
		switch (obj->type) {
		case MSG_MINI_TYPE_FIXMAP:
		case MSG_MINI_TYPE_MAP16:
		case MSG_MINI_TYPE_MAP32:
			return true;
		default:
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_object_as_char(msg_mini_object *obj, int8_t *c) 
	{
		switch (obj->type) {
		case MSG_MINI_TYPE_POSITIVE_FIXNUM:
		case MSG_MINI_TYPE_NEGATIVE_FIXNUM:
		case MSG_MINI_TYPE_SINT8:
			*c = obj->as.s8;
			return true;
		case MSG_MINI_TYPE_UINT8:
			if (obj->as.u8 <= 127) {
				*c = obj->as.s8;
				return true;
			}
		default:
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_object_as_short(msg_mini_object *obj, int16_t *s) 
	{
		switch (obj->type) {
		case MSG_MINI_TYPE_POSITIVE_FIXNUM:
		case MSG_MINI_TYPE_NEGATIVE_FIXNUM:
		case MSG_MINI_TYPE_SINT8:
			*s = obj->as.s8;
			return true;
		case MSG_MINI_TYPE_UINT8:
			*s = obj->as.u8;
			return true;
		case MSG_MINI_TYPE_SINT16:
			*s = obj->as.s16;
			return true;
		case MSG_MINI_TYPE_UINT16:
			if (obj->as.u16 <= 32767) {
				*s = obj->as.u16;
				return true;
			}
		default:
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_object_as_int(msg_mini_object *obj, int32_t *i) 
	{
		switch (obj->type) {
		case MSG_MINI_TYPE_POSITIVE_FIXNUM:
		case MSG_MINI_TYPE_NEGATIVE_FIXNUM:
		case MSG_MINI_TYPE_SINT8:
			*i = obj->as.s8;
			return true;
		case MSG_MINI_TYPE_UINT8:
			*i = obj->as.u8;
			return true;
		case MSG_MINI_TYPE_SINT16:
			*i = obj->as.s16;
			return true;
		case MSG_MINI_TYPE_UINT16:
			*i = obj->as.u16;
			return true;
		case MSG_MINI_TYPE_SINT32:
			*i = obj->as.s32;
			return true;
		case MSG_MINI_TYPE_UINT32:
			if (obj->as.u32 <= 2147483647) {
				*i = obj->as.u32;
				return true;
			}
		default:
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_object_as_long(msg_mini_object *obj, int64_t *d) 
	{
		switch (obj->type) {
		case MSG_MINI_TYPE_POSITIVE_FIXNUM:
		case MSG_MINI_TYPE_NEGATIVE_FIXNUM:
		case MSG_MINI_TYPE_SINT8:
			*d = obj->as.s8;
			return true;
		case MSG_MINI_TYPE_UINT8:
			*d = obj->as.u8;
			return true;
		case MSG_MINI_TYPE_SINT16:
			*d = obj->as.s16;
			return true;
		case MSG_MINI_TYPE_UINT16:
			*d = obj->as.u16;
			return true;
		case MSG_MINI_TYPE_SINT32:
			*d = obj->as.s32;
			return true;
		case MSG_MINI_TYPE_UINT32:
			*d = obj->as.u32;
			return true;
		case MSG_MINI_TYPE_SINT64:
			*d = obj->as.s64;
			return true;
		case MSG_MINI_TYPE_UINT64:
			if (obj->as.u64 <= 9223372036854775807) {
				*d = obj->as.u64;
				return true;
			}
		default:
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_object_as_sinteger(msg_mini_object *obj, int64_t *d) 
	{
		return msg_mini_object_as_long(obj, d);
	}

	bool 
	msg_mini::msg_mini_object_as_float(msg_mini_object *obj, float *f) 
	{
		if (obj->type == MSG_MINI_TYPE_FLOAT) {
			*f = obj->as.flt;
			return true;
		}

		return false;
	}

	bool 
	msg_mini::msg_mini_object_as_double(msg_mini_object *obj, double *d) 
	{
		if (obj->type == MSG_MINI_TYPE_DOUBLE) {
			*d = obj->as.dbl;
			return true;
		}

		return false;
	}

	bool 
	msg_mini::msg_mini_object_as_bool(msg_mini_object *obj, bool *b) 
	{
		if (obj->type == MSG_MINI_TYPE_BOOLEAN) {
			if (obj->as.boolean)
				*b = true;
			else
				*b = false;

			return true;
		}

		return false;
	}

	bool 
	msg_mini::msg_mini_object_as_str(msg_mini_object *obj, uint32_t *size) 
	{
		switch (obj->type) {
		case MSG_MINI_TYPE_FIXSTR:
		case MSG_MINI_TYPE_STR8:
		case MSG_MINI_TYPE_STR16:
		case MSG_MINI_TYPE_STR32:
			*size = obj->as.str_size;
			return true;
		default:
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_object_as_array(msg_mini_object *obj, uint32_t *size) 
	{
		switch (obj->type) {
		case MSG_MINI_TYPE_FIXARRAY:
		case MSG_MINI_TYPE_ARRAY16:
		case MSG_MINI_TYPE_ARRAY32:
			*size = obj->as.array_size;
			return true;
		default:
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_object_as_map(msg_mini_object *obj, uint32_t *size) 
	{
		switch (obj->type) {
		case MSG_MINI_TYPE_FIXMAP:
		case MSG_MINI_TYPE_MAP16:
		case MSG_MINI_TYPE_MAP32:
			*size = obj->as.map_size;
			return true;
		default:
			return false;
		}
	}

	bool 
	msg_mini::msg_mini_object_to_str(msg_mini_object *obj, char *data, uint32_t buf_size) 
	{
		uint32_t str_size = 0;
		switch (obj->type) {
		case MSG_MINI_TYPE_FIXSTR:
		case MSG_MINI_TYPE_STR8:
		case MSG_MINI_TYPE_STR16:
		case MSG_MINI_TYPE_STR32:
			str_size = obj->as.str_size;
			if ((str_size + 1) > buf_size) {
				error = STR_DATA_LENGTH_TOO_LONG_ERROR;
				return false;
			}

			if (!read(data, str_size)) {
				error = DATA_READING_ERROR;
				return false;
			}

			data[str_size] = 0;
			return true;
		default:
			return false;
		}
	}
}