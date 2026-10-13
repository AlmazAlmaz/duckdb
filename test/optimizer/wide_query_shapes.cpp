TEST_CASE("Filters derived from a wide disjunction keep the plan shallow", "[optimizer]") {
	DuckDB db(nullptr);
	Connection con(db);
	REQUIRE_NO_FAIL(con.Query(
	    "CREATE TABLE t AS SELECT i AS a, i % 7 AS b, DATE '2000-01-01' + i::INTEGER AS d FROM range(100000) r(i)"));

	vector<string> terms;
	for (idx_t i = 0; i < 50000; i++) {
		terms.push_back(StringUtil::Format("(a = %d AND b = %d)", i, i % 7));
	}
	auto result = con.Query("SELECT count(*) FROM t WHERE " + StringUtil::Join(terms, " OR "));
	REQUIRE(CHECK_COLUMN(result, 0, {50000}));

	vector<string> years;
	for (idx_t year = 1; year <= 20000; year++) {
		years.push_back(to_string(year));
	}
	result = con.Query("SELECT count(*) FROM t WHERE year(d) IN (" + StringUtil::Join(years, ", ") + ")");
	REQUIRE(CHECK_COLUMN(result, 0, {100000}));
}

TEST_CASE("Order dependencies of a wide UNION ALL stay linear", "[optimizer]") {
	DuckDB db(nullptr);
	Connection con(db);

	vector<string> branches;
	for (idx_t i = 0; i < 20000; i++) {
		branches.push_back(StringUtil::Format("SELECT %d AS v", i));
	}
	auto result = con.Query(StringUtil::Join(branches, " UNION ALL "));
	REQUIRE_NO_FAIL(*result);
	REQUIRE(result->RowCount() == 20000);
	idx_t row = 0;
	for (auto &chunk : result->Collection().Chunks()) {
		for (idx_t i = 0; i < chunk.size(); i++) {
			REQUIRE(chunk.GetValue(0, i) == Value::INTEGER(NumericCast<int32_t>(row++)));
		}
	}
	REQUIRE(row == 20000);
}

TEST_CASE("Alternating AND and OR levels plan without re-hashing their subtrees", "[optimizer]") {
	DuckDB db(nullptr);
	Connection con(db);
	REQUIRE_NO_FAIL(con.Query("CREATE TABLE t AS SELECT i AS a, i % 7 AS b FROM range(1000) r(i)"));

	string predicate = "a > 0";
	for (idx_t level = 200; level > 0; level--) {
		auto op = level % 2 ? "AND" : "OR";
		predicate = StringUtil::Format("(a > %d %s (%s) %s b < %d)", level, op, predicate, op, level % 7);
	}
	auto query = "SELECT count(*) FROM t WHERE " + predicate;
	auto optimized = con.Query(query);
	REQUIRE_NO_FAIL(*optimized);
	REQUIRE_NO_FAIL(con.Query("SET disabled_optimizers = 'expression_rewriter'"));
	auto unoptimized = con.Query(query);
	REQUIRE_NO_FAIL(*unoptimized);
	REQUIRE(optimized->Collection().GetValue(0, 0) == unoptimized->Collection().GetValue(0, 0));
}
