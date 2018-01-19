#pragma once

#include <vector>
#include <array>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>
#include <functional>

// BareBones MessagePack Reader, only handles ints, floats, arrays and strings

namespace mpMini {
	enum {
		MSG_MINI_GENERIC_INT_TYPE,
		MSG_MINI_GENERIC_FLOAT_TYPE,
		MSG_MINI_GENERIC_STRING_TYPE,
		MSG_MINI_GENERIC_ARRAY_INT_TYPE,
		MSG_MINI_GENERIC_ARRAY_FLOAT_TYPE,
		MSG_MINI_GENERIC_ARRAY_STRING_TYPE,
	};

	class msg_mini_generic_data {
	public:
		msg_mini_generic_data(int32_t type_in)
		{
			type = type_in;
			int_val = 0;
			float_val = 0;
		}

		int32_t type;
		int32_t int_val;
		float float_val;
		std::string string_val;
		std::vector<int32_t> int_array_val;
		std::vector<float> float_array_val;
		std::vector<std::string> str_array_val;
	};

	union msg_mini_object_data {
		bool      boolean;
		uint8_t   u8;
		uint16_t  u16;
		uint32_t  u32;
		uint64_t  u64;
		int8_t    s8;
		int16_t   s16;
		int32_t   s32;
		int64_t   s64;
		float     flt;
		double    dbl;
		uint32_t  array_size;
		uint32_t  map_size;
		uint32_t  str_size;
	};

	class msg_mini_object {
	public:
		uint8_t type;
		union msg_mini_object_data as;
	};

	class msg_mini {
	protected:
		uint8_t error;
		std::vector<uint8_t> buf;
		uint32_t read_idx, store_read_idx;
		std::vector<msg_mini_generic_data> generic_data;

		bool read_one_byte(uint8_t *x);

		bool read_marker_type(uint8_t *marker);

		void store_read_pos()
		{
			store_read_idx = read_idx;
		}

		void restore_read_pos()
		{
			read_idx = store_read_idx;
		}

	public:
		msg_mini(const std::vector<uint8_t>& buf_in)
		{
			error = 0;
			buf = buf_in;
			read_idx = 0;
			store_read_idx = 0;
			msg_mini_build_generic_objects();
		}

		virtual ~msg_mini() {}

		bool read(void *data, size_t limit) 
		{
			if (is_at_end())
			{
				// end of buffer
				return false;
			}

			uint8_t * base_ptr = &buf[read_idx];
			//memcpy_s(data, limit, base_ptr, limit);
			std::memcpy(data, base_ptr, limit);
			read_idx += (uint32_t)limit;

			return true;
		}

		bool readBytesChunk(size_t limit, std::function<void(int, uint8_t)> readCB)
		{
			if (is_at_end())
			{
				// end of buffer
				return false;
			}

			uint8_t * base_ptr = &buf[read_idx];
			for (size_t i = 0; i < limit; i++)
			{
				readCB(i, base_ptr[i]);
				read_idx++;
			}

			return true;
		}

		bool is_at_end() const {
			return read_idx >= buf.size();
		}

		void reset_read()
		{
			read_idx = 0;
		}

		/*
		* Actual Calls
		*/

		const char* get_strerror() const;

		bool
		msg_mini_build_generic_objects();

		const std::vector<msg_mini_generic_data>&
		msg_mini_get_generic_objects() const;

		bool msg_mini_read_int(int32_t *i);

		bool msg_mini_read_real(float *d);

		bool msg_mini_read_nil();

		bool msg_mini_read_bool(bool *b);

		bool msg_mini_read_str_size(uint32_t *size);

		bool msg_mini_read_str(std::string& data);

		bool msg_mini_read_bin(uint32_t *size);

		bool msg_mini_read_array(uint32_t *size);

		bool msg_mini_read_object(msg_mini_object *obj);

		/* Data calls
		*/

		bool msg_mini_read_pfix(uint8_t *c);
		bool msg_mini_read_nfix(int8_t *c);

		bool msg_mini_read_sfix(int8_t *c);
		bool msg_mini_read_s8(int8_t *c);
		bool msg_mini_read_s16(int16_t *s);
		bool msg_mini_read_s32(int32_t *i);
		bool msg_mini_read_s64(int64_t *l);

		bool msg_mini_read_ufix(uint8_t *c);
		bool msg_mini_read_u8(uint8_t *c);
		bool msg_mini_read_u16(uint16_t *s);
		bool msg_mini_read_u32(uint32_t *i);
		bool msg_mini_read_u64(uint64_t *l);

		bool msg_mini_read_float(float *f);
		bool msg_mini_read_double(double *d);

		/* Object calls
		*/

		bool msg_mini_object_is_char(msg_mini_object *obj);
		bool msg_mini_object_is_short(msg_mini_object *obj);
		bool msg_mini_object_is_int(msg_mini_object *obj);
		bool msg_mini_object_is_long(msg_mini_object *obj);
		bool msg_mini_object_is_sinteger(msg_mini_object *obj);
		bool msg_mini_object_is_uchar(msg_mini_object *obj);
		bool msg_mini_object_is_ushort(msg_mini_object *obj);
		bool msg_mini_object_is_uint(msg_mini_object *obj);
		bool msg_mini_object_is_ulong(msg_mini_object *obj);
		bool msg_mini_object_is_uinteger(msg_mini_object *obj);
		bool msg_mini_object_is_float(msg_mini_object *obj);
		bool msg_mini_object_is_double(msg_mini_object *obj);
		bool msg_mini_object_is_nil(msg_mini_object *obj);
		bool msg_mini_object_is_bool(msg_mini_object *obj);
		bool msg_mini_object_is_str(msg_mini_object *obj);
		bool msg_mini_object_is_bin(msg_mini_object *obj);
		bool msg_mini_object_is_array(msg_mini_object *obj);
		bool msg_mini_object_is_map(msg_mini_object *obj);

		bool msg_mini_object_as_char(msg_mini_object *obj, int8_t *c);
		bool msg_mini_object_as_short(msg_mini_object *obj, int16_t *s);
		bool msg_mini_object_as_int(msg_mini_object *obj, int32_t *i);
		bool msg_mini_object_as_long(msg_mini_object *obj, int64_t *d);
		bool msg_mini_object_as_sinteger(msg_mini_object *obj, int64_t *d);
		bool msg_mini_object_as_float(msg_mini_object *obj, float *f);
		bool msg_mini_object_as_double(msg_mini_object *obj, double *d);
		bool msg_mini_object_as_bool(msg_mini_object *obj, bool *b);
		bool msg_mini_object_as_str(msg_mini_object *obj, uint32_t *size);
		bool msg_mini_object_as_array(msg_mini_object *obj, uint32_t *size);
		bool msg_mini_object_as_map(msg_mini_object *obj, uint32_t *size);

		bool msg_mini_object_to_str(msg_mini_object *obj, char *data, uint32_t buf_size);

	};



}