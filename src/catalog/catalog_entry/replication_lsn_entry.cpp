#include "duckdb/catalog/catalog_entry/replication_lsn_entry.hpp"

namespace duckdb {

ReplicationLsnEntry::ReplicationLsnEntry(CatalogType type, Catalog &catalog, Identifier name, idx_t oid,
                                         shared_ptr<ReplicationLsnState> lsn_state_p)
    : InCatalogEntry(type, catalog, std::move(name), oid), lsn_state(std::move(lsn_state_p)) {
	D_ASSERT(lsn_state);
}

void ReplicationLsnEntry::RaiseRemoteLsn(uint64_t lsn) {
	auto current = lsn_state->remote_lsn.load();
	while (lsn > current && !lsn_state->remote_lsn.compare_exchange_weak(current, lsn)) {
	}
}

} // namespace duckdb
