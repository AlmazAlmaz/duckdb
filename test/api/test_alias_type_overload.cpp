TEST_CASE("A SQL function declared to return an aliased type returns that type", "[api][scalar_function]") {
	DuckDB db(nullptr);
	ExtensionLoader loader(*db.instance, "alias_type_result");
	loader.RegisterType("cents", LogicalType::DECIMAL(10, 2).WithAlias("cents"));
	Connection con(db);

	REQUIRE_NO_FAIL(con.Query("CREATE FUNCTION in_cents() RETURNS cents LANGUAGE sql AS 'SELECT 1.5'"));
	auto result = con.Query("SELECT in_cents()::VARCHAR, typeof(in_cents())");
	REQUIRE(CHECK_COLUMN(result, 0, {"1.50"}));
	REQUIRE(CHECK_COLUMN(result, 1, {"cents"}));
}
