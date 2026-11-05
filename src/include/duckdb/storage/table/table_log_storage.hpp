#pragma once

#include "duckdb/common/common.hpp"
#include "duckdb/common/optional_ptr.hpp"

#include <memory>
#include <span>

namespace duckdb {

class ClientContext;
class DataChunk;

static constexpr idx_t WAL_FROZEN_BITMAP_ALIGNMENT = 32;

struct WALFrozenRowPositionGroup {
	uint64_t segment = 0;
	std::span<const data_t> docs;
};

struct WALFrozenRowPositionMap {
	uint64_t target = 0;
	uint32_t base = 0;
	uint32_t rank_begin = 0;
	uint32_t rank_end = 0;
	std::span<const data_t> mask;
};

class TableLogStorage {
public:
	virtual ~TableLogStorage() = default;

	virtual void ReplayInsert(ClientContext &context, DataChunk &chunk) = 0;
	virtual void ReplayTruncate() = 0;
	virtual void ReplayAdoptSegments(vector<string> segments) = 0;
	virtual void ReplayDeleteRowPositions(std::shared_ptr<const data_t> entry,
	                                      const vector<WALFrozenRowPositionGroup> &groups) = 0;
	virtual void ReplayRowPositionMaps(std::shared_ptr<const data_t> entry, uint64_t source,
	                                   const vector<WALFrozenRowPositionMap> &maps) = 0;
};

//! Supplies the consumers for a WAL read that is not driven by the boot-time table registry, which
//! only exists while a database is being loaded. A reader also tells the resolver where each batch
//! ends, so a consumer can stage a batch's entries and drop the ones whose transaction never
//! committed.
class TableLogStorageResolver {
public:
	virtual ~TableLogStorageResolver() = default;

	//! The consumer for this table's entries, or null to pass over them.
	virtual optional_ptr<TableLogStorage> Resolve(idx_t table_oid) = 0;
	//! True keeps what the batch just read produced, false drops it.
	virtual void EndBatch(bool committed) = 0;
};

} // namespace duckdb
