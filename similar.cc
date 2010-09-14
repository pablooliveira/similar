// similar: find groups of similar text files in a directory.
// Copyright Pablo de Oliveira Castro, 2010
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <xapian.h>

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/strong_components.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>

/*The name of the Xapian temporary database, used for indexing the directory.*/
#define DB_NAME ".tmp-similar-db"

using namespace std;
using namespace boost;
typedef adjacency_list<> similarity_graph;
typedef graph_traits<similarity_graph>::vertex_descriptor similarity_vertex;

void usage()
{
    cerr
    << "\nusage:   similar [path] [threshold]" << endl
    << endl
    << "similar uses Xapian to index the files in [path]." << endl
    << "It then computes the relevance between the terms of every" << endl
    << "pair of files. A directed similarity graph is built with" << endl
    << "a vertex for each indexed file. An edge A->B exists iff" << endl
    << "relevance of B for the terms in A is higher than " << endl
    << "[threshold]. similar returns the strong connected " << endl
    << "components of the similarity graph." << endl
    << endl
    << "\t [path] : the directory that contains the files to compare." << endl
    << "\t [threshold] : cut-off percentage that decides if two files" << endl
    << "\t   are similar (at 100 %, files must be really close to " << endl
    << "\t   be considered similar)" << endl
    << endl;
    exit(-1);
}

void index_file(Xapian::WritableDatabase &db, Xapian::TermGenerator indexer,
		filesystem::path p)
{
    filesystem::ifstream myfile (p);
    if (!myfile.is_open()) {
	cerr << "\nUnable to open file: " << p.file_string() << endl;
    }

    try {
	Xapian::Document doc;
	doc.set_data(p.file_string());
	indexer.set_document(doc);

	string line;
	while (!myfile.eof()) {
	    getline (myfile,line);
	    indexer.index_text(line);
	}
	db.add_document(doc);
    } catch (const Xapian::Error &e) {
	cout << e.get_description() << endl;
	exit(1);
    }
    myfile.close();
}

/* Index the files in the given directory
   (limitations: does not recurse into subdirectories,
                 stemmer is hardcoded to english)
*/
void index_directory(Xapian::WritableDatabase &db, filesystem::path directory)
{
    Xapian::TermGenerator indexer;
    Xapian::Stem stemmer("english");
    indexer.set_stemmer(stemmer);

    filesystem::directory_iterator end_iter;
    int count = 0;
    for ( filesystem::directory_iterator dir_itr(directory);
	  dir_itr != end_iter;
	  ++dir_itr ) {
	try {
	    if (filesystem::is_regular_file( dir_itr->status())) {
		index_file(db, indexer, dir_itr->path());
		cout << "indexing files (" << ++count << " done)";
		cout.flush();
		cout << "\r";
	    }
	}
	catch ( const std::exception & ex ) {
	    cerr << dir_itr->path().filename() << " " << ex.what() << endl;
	}
    }
    cout << endl << "indexing done." << endl;
    db.commit();
}

/* Populate the similarity graph */
void populate_similarity_graph(Xapian::Database &db,
			       int threshold, similarity_graph &graph)
{
    /* Connect every document, to the other similar documents */
    for (int id = 1; id < db.get_lastdocid(); id++) {
	Xapian::Enquire enquire(db);
	Xapian::Document doc = db.get_document(id);
	Xapian::RSet rset;
	rset.add_document(id);

	/* Consider 40 terms for the query (is this a good value ?
	   taken from http://trac.xapian.org/wiki/FAQ/FindSimilar) */
	Xapian::ESet eset = enquire.get_eset(40, rset);
	Xapian::Query query(Xapian::Query::OP_OR, eset.begin(), eset.end());
	enquire.set_cutoff(threshold);
	enquire.set_query(query);
	Xapian::MSet matches = enquire.get_mset(0, db.get_doccount());
	for (Xapian::MSetIterator i = matches.begin(); i != matches.end(); ++i)
	    /* For every match, add an edge in the similarity graph */
	    /* There is no need to connect a vertex to itself */
	    if (*i != id) add_edge(id,*i,graph);
    }
}

/* Find and prints the strong connected components in the similarity graph */
void find_strong_components(Xapian::Database &db, similarity_graph &G)
{
    vector<int> component(num_vertices(G)), discover_time(num_vertices(G));
    vector<default_color_type> color(num_vertices(G));
    vector<similarity_vertex> root(num_vertices(G));

    /* Use Boost's Tarjan algorithm. */
    int num = strong_components(G, &component[0],
				root_map(&root[0]).
				color_map(&color[0]).
				discover_time_map(&discover_time[0]));

    cout << "Non trivial strong connected components of the similarity graph:" << endl;
    for (int i = 1; i < num; i++) {
	ostringstream component_members;
	int component_card = 0;
	for (int j = 1; j < num_vertices(G); j++)
	    if (component[j] == i) {
		component_members << "\t" << db.get_document(j).get_data() <<  endl;
		component_card++;
	    }
	if (component_card > 1) /* Only consider non trivial components */
	    cout << "{" << endl << component_members.str() << "}" << endl;
    }
}

int main( int argc, char* argv[] )
{
    filesystem::path path;
    int threshold;

    /* Parse arguments */
    if (argc != 3) usage();

    path = filesystem::path(argv[1]);
    threshold = atoi(argv[2]);
    if (threshold > 100 || threshold < 0) {
	cerr << "\nthreshold must be between 0 and 100." << endl;
	exit(-1);
    }

    /* Check that the given path is an existing directory */
    if ( !filesystem::exists(path) ||  !filesystem::is_directory(path)) {
	cerr << "\nNot a directory: " << path.file_string() << endl;
	exit(-1);
    }

    /* Create a new database. */
    filesystem::path db_path =  path / DB_NAME;
    if (filesystem::exists(db_path)) {
	cerr << "\nTemporary database already exists (" << db_path.file_string()
	     << ")" << endl;
	exit(-1);
    }
    Xapian::WritableDatabase * db = new Xapian::WritableDatabase(db_path.string(), Xapian::DB_CREATE_OR_OPEN);

    /* Index the files in the directory */
    index_directory(*db, path);

    /* Create a similarity graph, and find its strong connected components */
    similarity_graph graph(db->get_lastdocid()+1);
    populate_similarity_graph(*db,threshold,graph);
    find_strong_components(*db, graph);

    delete db;

    /* Clean up Xapian db directory*/
    try {
	remove_all(db_path);
    }
    catch ( const std::exception & ex ) {
	cerr << "\ncould not remove tmp database: " << db_path.string() << endl;
	exit(-1);
    }
    return 0;
}
