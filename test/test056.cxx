#include <iostream>

#include <pqxx/pqxx>

#include "test_helpers.hxx"

// Don't try this at home: use some libpqxx-internal information about libpq
#include <pqxx/config-internal-libpq.h>

using namespace PGSTD;
using namespace pqxx;


// Simple test program for libpqxx.  Issue invalid query and handle error.
namespace
{
void test_056(connection_base &C, transaction_base &T)
{
  disable_noticer d(C);

  PQXX_CHECK_THROWS(
	T.exec("DELIBERATELY INVALID TEST QUERY...", "invalid_query"),
	sql_error,
	"SQL syntax error did not raise expected exception.");
}
} // namespace

PQXX_REGISTER_TEST(test_056)
