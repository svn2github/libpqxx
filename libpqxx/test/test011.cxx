#include <cstdio>
#include <iostream>

#include <pqxx/connection>
#include <pqxx/transaction>
#include <pqxx/result>

using namespace PGSTD;
using namespace pqxx;


// Test program for libpqxx.  Query a table and report its metadata.
//
// Usage: test011 [connect-string] [table]
//
// Where table is the table to be queried; if none is given, pg_tables is
// queried by default.
//
// The connect-string is a set of connection options in Postgresql's
// PQconnectdb() format, eg. "dbname=template1" to select from a database
// called template1, or "host=foo.bar.net user=smith" to connect to a
// backend running on host foo.bar.net, logging in as user smith.
int main(int argc, char *argv[])
{
  try
  {
    const string Table = ((argc >= 3) ? argv[2] : "pg_tables");

    connection C(argv[1]);
    transaction<> T(C, "test11");

    result R( T.exec("SELECT * FROM " + Table) );

    // Print column names
    for (result::tuple::size_type c = 0; c < R.columns(); ++c)
    {
      string N= R.column_name(c);
      cout << c << ":\t" << N << endl;
      if (R.column_number(N) != c)
	throw logic_error("Expected column '" + N + 
			  "' to be no. " + ToString(c) + ", "
			  "but it was no. " + ToString(R.column_number(N)));
    }

    // If there are rows in R, compare their metadata to R's.
    if (!R.empty())
    {
      if (R[0].rownumber() != 0) 
	throw logic_error("Row 0 said it was row " + R[0].rownumber());

      if (R.size() < 2)
	cout << "(Only one row in table.)" << endl;
      else if (R[1].rownumber() != 1)
        throw logic_error("Row 1 said it was row " + R[1].rownumber());

      for (result::tuple::size_type c = 0; c < R[0].size(); ++c)
      {
	string N = R.column_name(c);

	if (string(R[0].at(c).c_str()) != R[0].at(N).c_str())
          throw logic_error("Field " + ToString(c) + " contains "
			    "'" + R[0].at(c).c_str() + "'; "
			    "field '" + N + "' "
			    "contains '" + R[0].at(N).c_str() + "'");
	if (string(R[0][c].c_str()) != R[0][N].c_str())
          throw logic_error("Field " + ToString(c) + " ('" + N + "'): "
			    "at() inconsistent with operator[]!");

	if (R[0][c].name() != N)
	  throw logic_error("Field " + ToString(c) + " "
			    "called '" + N + "' by result, "
			    "but '" + R[0][c].name() + "' by Field object");

	if (size_t(R[0][c].size()) != strlen(R[0][c].c_str()))
	  throw logic_error("Field '" + N + "' "
			    "says its length is " + ToString(R[0][c].size()) + 
			    ", "
			    "but its value is '" + R[0][c].c_str() + "' "
			    "(" + ToString(strlen(R[0][c].c_str())) + " "
			    "chars)");
      }
    }
    else
    {
      cout << "(Table is empty.)" << endl;
    }
  }
  catch (const sql_error &e)
  {
    // If we're interested in the text of a failed query, we can write separate
    // exception handling code for this type of exception
    cerr << "SQL error: " << e.what() << endl
         << "Query was: '" << e.query() << "'" << endl;
    return 1;
  }
  catch (const exception &e)
  {
    // All exceptions thrown by libpqxx are derived from std::exception
    cerr << "Exception: " << e.what() << endl;
    return 2;
  }
  catch (...)
  {
    // This is really unexpected (see above)
    cerr << "Unhandled exception" << endl;
    return 100;
  }

  return 0;
}

