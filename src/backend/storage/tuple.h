/*-------------------------------------------------------------------------
 *
 * abstract_tuple.h
 * file description
 *
 * Copyright(c) 2015, CMU
 *
 * /n-store/src/storage/abstract_tuple.h
 *
 *-------------------------------------------------------------------------
 */

#pragma once

#include <memory>

#include "backend/catalog/schema.h"
#include "backend/common/value.h"
#include "backend/common/value_factory.h"
#include "backend/common/value_peeker.h"
#include "backend/common/types.h"
#include "backend/expression/tuple.h"

namespace nstore {
namespace storage {

//===--------------------------------------------------------------------===//
// Tuple class
//===--------------------------------------------------------------------===//

class Tuple : public expression::Tuple {
	friend class catalog::Schema;
	friend class ValuePeeker;
	friend class Tile;

public:

	// Default constructor (don't use this)
	inline Tuple() : tuple_schema(nullptr), tuple_data(nullptr), allocated(false) {
	}

	// Setup the tuple given a table
	inline Tuple(const Tuple &rhs) : tuple_schema(rhs.tuple_schema), tuple_data(rhs.tuple_data), allocated(false) {
	}

	// Setup the tuple given a schema
	inline Tuple(const catalog::Schema *schema) : tuple_schema(schema), tuple_data(nullptr), allocated(false) {
		assert(tuple_schema);
	}

	// Setup the tuple given a schema and location
	inline Tuple(const catalog::Schema *schema, char* data) : tuple_schema(schema), tuple_data(data), allocated(false) {
		assert(tuple_schema);
		assert(tuple_data);
	}

	// Setup the tuple given a schema and allocate space
	inline Tuple(const catalog::Schema *schema, bool allocate) : tuple_schema(schema), tuple_data(nullptr), allocated(allocate) {
		assert(tuple_schema);

		if(allocated) {
		  // initialize heap allocation
			tuple_data = new char[tuple_schema->GetLength()]();
		}

	}

	// Deletes tuple data
	// Does not delete either SCHEMA or UNINLINED data
	// Tile or larger entities must take care of this
	~Tuple() {
    // delete the tuple data
	  if(allocated)
	    delete[] tuple_data;
	}

	// Setup the tuple given the specified data location and schema
	Tuple(char *data, catalog::Schema *schema);

	// Assignment operator
	Tuple& operator=(const Tuple &rhs);

	void Copy(const void *source, Pool *pool = NULL);

	/**
	 * Set the tuple to point toward a given address in a table's
	 * backing store
	 */
	inline void Move(void *address) {
		tuple_data = reinterpret_cast<char*> (address);
	}

	bool operator==(const Tuple &other) const;
	bool operator!=(const Tuple &other) const;

	int Compare(const Tuple &other) const;

	//===--------------------------------------------------------------------===//
	// Getters and Setters
	//===--------------------------------------------------------------------===//

	// Get the value of a specified column (const)
	// (expensive) checks the schema to see how to return the Value.
	const Value GetValue(const oid_t column_id) const;

	// Set appropriate column in tuple
	void SetValue(const oid_t column_id, Value value);

  /**
   * Allocate space to copy strings that can't be inlined rather
   * than copying the pointer.
   *
   * Used when setting a NValue that will go into permanent storage in a persistent table.
   * It is also possible to provide NULL for stringPool in which case
   * the strings will be allocated on the heap.
   */
  void SetValueAllocate(const oid_t column_id, Value value, Pool *dataPool);

	inline int GetLength() const {
		return tuple_schema->GetLength();
	}

	// Is the column value null ?
	inline bool IsNull(const uint64_t column_id) const {
		return GetValue(column_id).IsNull();
	}

	// Is the tuple null ?
	inline bool IsNull() const {
		return tuple_data == NULL;
	}

	// Get the type of a particular column in the tuple
	inline ValueType GetType(int column_id) const {
		return tuple_schema->GetType(column_id);
	}

	inline const catalog::Schema *GetSchema() const {
		return tuple_schema;
	}

	// Get the address of this tuple in the table's backing store
	inline char* GetData() const {
		return tuple_data;
	}

	// Return the number of columns in this tuple
	inline oid_t GetColumnCount() const {
		return tuple_schema->GetColumnCount();
	}

	// Release to the heap any memory allocated for any uninlined columns.
	void FreeUninlinedData();

	bool EqualsNoSchemaCheck(const Tuple &other) const;

	// this does set NULL in addition to clear string count.
	void SetAllNulls();

	void SetNull() {
		tuple_data = NULL;
	}

	/**
	 * Determine the maximum number of bytes when serialized for Export.
	 * Excludes the bytes required by the row header (which includes
	 * the null bit indicators) and ignores the width of metadata columns.
	 */
	size_t ExportSerializationSize() const;

	// Return the amount of memory allocated for non-inlined objects
	size_t GetUninlinedMemorySize() const;

	//===--------------------------------------------------------------------===//
	// Serialization utilities
	//===--------------------------------------------------------------------===//

	void SerializeTo(SerializeOutput &output);
	void SerializeToExport(ExportSerializeOutput &output, int col_offset,
			uint8_t *null_array);
	void SerializeWithHeaderTo(SerializeOutput &output);

	void DeserializeFrom(SerializeInput &input, Pool *pool);
	int64_t DeserializeWithHeaderFrom(SerializeInput &input);

	size_t HashCode(size_t seed) const;
	size_t HashCode() const;

	// Get a string representation of this tuple
	friend std::ostream& operator<< (std::ostream& os, const Tuple& tuple);

	std::string GetInfo() const;

private:

	char* GetDataPtr(const oid_t column_id);

	const char* GetDataPtr(const oid_t column_id) const;

	//===--------------------------------------------------------------------===//
	// Data members
	//===--------------------------------------------------------------------===//

	// The types of the columns in the tuple
	const catalog::Schema *tuple_schema;

	// The tuple data, padded at the front by the TUPLE_HEADER
	char *tuple_data;

	// Allocated or not ?
	bool allocated;
};

//===--------------------------------------------------------------------===//
// Implementation
//===--------------------------------------------------------------------===//

// Setup the tuple given the specified data location and schema
inline Tuple::Tuple(char *data, catalog::Schema *schema) {
	assert(data);
	assert(schema);

	tuple_data = data;
	tuple_schema = schema;
}

inline Tuple& Tuple::operator=(const Tuple &rhs) {
	tuple_schema = rhs.tuple_schema;
	tuple_data = rhs.tuple_data;
	return *this;
}

} // End storage namespace
} // End nstore namespace
