#pragma once

namespace duckdb {

class WriteAheadLog;

class TransactionLogWriter {
public:
	virtual ~TransactionLogWriter() = default;

	virtual void Append() = 0;
	virtual void WriteToWAL(WriteAheadLog &wal) = 0;
};

} // namespace duckdb
