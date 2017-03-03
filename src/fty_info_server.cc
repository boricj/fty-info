/*  =========================================================================
    fty_info_server - 42ity info server

    Copyright (C) 2014 - 2017 Eaton                                        
                                                                           
    This program is free software; you can redistribute it and/or modify   
    it under the terms of the GNU General Public License as published by   
    the Free Software Foundation; either version 2 of the License, or      
    (at your option) any later version.                                    
                                                                           
    This program is distributed in the hope that it will be useful,        
    but WITHOUT ANY WARRANTY; without even the implied warranty of         
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          
    GNU General Public License for more details.                           
                                                                           
    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.            
    =========================================================================
*/

/*
@header
    fty_info_server - 42ity info server
@discuss
@end
*/
#define TIMEOUT_MS 30000   //wait at least 30 seconds
static const char* RELEASE_DETAILS = "/etc/release-details.json";

#include <string>
#include <unistd.h>
#include <bits/local_lim.h>
#include <tntdb/connect.h>
#include <tntdb/result.h>
#include <tntdb/error.h>
#include <cxxtools/jsondeserializer.h>
#include <istream>
#include <fstream>

#include "fty_info_classes.h"

struct _fty_info_t {
	char *uuid;
	char *hostname;
	char *name;
	char *product_name;
	char *location;
	char *version;
	char *rest_root;
	int rest_port;
};

std::vector<std::string> rest_roots { "/api/v1/" };
std::vector<int> rest_ports { 8000, 80, 443 };

int
get_product_name
	(std::istream &f,
	 std::string &product_name)
{
	try {
		cxxtools::SerializationInfo si;
		std::string json_string (std::istreambuf_iterator<char>(f), {});
		std::stringstream s(json_string);
		cxxtools::JsonDeserializer json(s);
		json.deserialize (si);
		//if (si.memberCount == 0)
		//	throw std::runtime_error ("Document /etc/release-details.json is empty");
		si.getMember("release-details").getMember("hardware-catalog-number") >>= product_name;
		return 0;
	}
	catch (const std::exception& e) {
		zsys_error ("Error while parsing JSON: %s", e.what ());
		return -1;
	}
}

int
select_rack_controller_parent
	(tntdb::Connection &conn,
	 char *name,
	 std::string &parent_name)
{
	try {
		tntdb::Statement st = conn.prepareCached(
			" SELECT "
			"   t1.parent_name "
			" FROM "
			"   v_bios_asset_element t1"
			" WHERE "
			"   t1.name = :name "
		);
		tntdb::Result result = st.set ("name", name).select ();
		if (result.size () > 1) {
			zsys_error ("asset '%s' has more than one parent, DB is messed up");
			return -2;
		}
		for (auto &row: result) {
			row ["parent_name"].get (parent_name);
			printf ("Found parent '%s'\n", parent_name.c_str ());
		}
		printf ("Returning %s\n", parent_name.c_str ());
		return 0;
	}
	catch (const std::exception& e) {
		zsys_error ("Error: %s", e.what ());
		return -1;
	}
}

//TODO: change after we add display name
int
select_rack_controller_name
	(tntdb::Connection &conn,
	 std::string &name)
{
	try {
		tntdb::Statement st = conn.prepareCached(
			" SELECT "
			"   t1.name "
			" FROM "
			"   (v_bios_asset_element t1 INNER JOIN v_bios_asset_device_type t2 ON (t1.id_subtype = t2.id))"
			" WHERE "
			"   t2.name = 'rack controller'"
		);
		printf("statement prepared\n");
		tntdb::Result result = st.select ();
		if (result.size () > 1) {
			zsys_error ("fty-info found more than one RC, not sure what to do");
			return -2;
		}
		printf ("select succeeded\n");

		for (auto &row: result) {
			row ["name"].get (name);
			printf ("Found RC '%s'\n", name.c_str ());
		}
		printf ("Returning %s\n", name.c_str ());
		return 0;
	}
	catch (const std::exception& e) {
		zsys_error ("Error: %s", e.what ());
		return -1;
	}
}

fty_info_t*
fty_info_new (void)
{
	fty_info_t *self = (fty_info_t *) malloc (sizeof (fty_info_t));
    
	printf ("Initialized\n");	
	printf ("Connecting to the database...\n");
	tntdb::Connection conn;
	std::string url = std::string("mysql:db=box_utf8;user=") +
		((getenv("DB_USER")   == NULL) ? "root" : getenv("DB_USER")) +
		((getenv("DB_PASSWD") == NULL) ? ""     :   
		 std::string(";password=") + getenv("DB_PASSWD"));

	try {
		 conn = tntdb::connectCached (url);
	}
	catch ( const std::exception &e) {
		zsys_error ("DB: cannot connect, %s", e.what());
	}    
    
	printf ("Connected\n");
	// set uuid - generated by `uuid -v5 <name_space_based_on_uuid> <(Concatenation of UTF8 encoded: vendor + model + serial)>`
	// TODO: /etc/release-details.json doesn't contain vendor, and serial number is empty 
	self->uuid = strdup ("");
	
	// set hostname
	char *hostname = (char *) malloc (HOST_NAME_MAX+1);
	int rv = gethostname (hostname, HOST_NAME_MAX+1);
	if (rv == -1) {
		zsys_warning ("fty_info could not be fully initialized (error while getting the hostname)");
		self->hostname = strdup("");
	}
	else {
		printf ("hostname = '%s'\n", hostname);
		self->hostname = strdup (hostname);
	}
	zstr_free (&hostname);
	printf ("hostname = '%s'\n", self->hostname);
	
	//set name
	std::string name;
	rv = select_rack_controller_name (conn, name);
	if (rv < 0) {
		zsys_warning ("fty_info could not be fully initialized (error while getting the name)");
		self->name = strdup ("");
	}
	else {
		printf ("Returned '%s'\n", name.c_str ());
		self->name = strdup (name.c_str ());
	}
	printf ("name set to the value '%s'\n", self-> name);
	
	// set product name - "hardware-catalog-number" from /etc/release-details.json (first part?)
	std::ifstream f(RELEASE_DETAILS);
	std::string product_name;
	rv = get_product_name (f, product_name);
	if (rv < 0) {
		zsys_warning ("fty_info could not be fully initialized (error while getting the product name)");
		self->product_name = strdup ("");
	}
	else {
		printf ("Returned '%s'\n", product_name.c_str ());
		self->product_name = strdup (product_name.c_str ());	
	}
	printf ("Product name set to the value '%s'\n", self->product_name);
	
	// set location (parent)
	std::string parent;
	rv = select_rack_controller_parent (conn, self->name, parent);
	if (rv < 0) {
		zsys_warning ("fty_info could not be fully initialized (error while getting the location)");
		self->location = strdup ("");
	}
	else {
		printf ("Returned '%s'\n", parent.c_str ());
		self->location = strdup (parent.c_str ());
	}
	printf ("location set to the value '%s'\n", self->location);
	
	// TODO: set version
	self->version = strdup ("");
	// set rest_root - what if we can find more than one?
	self->rest_root = strdup ("");
	// set rest_port - what if we can find more than one?
	self->rest_port = 0;

	return self;
}

void
fty_info_destroy (fty_info_t ** self_ptr)
{
	if (!self_ptr)
		return;
	if (*self_ptr) {
		fty_info_t *self = *self_ptr;
		// Free class properties here
		zstr_free (&self->uuid);
		zstr_free (&self->hostname);
		zstr_free (&self->name);
		zstr_free (&self->product_name);
		zstr_free (&self->location);
		zstr_free (&self->version);
		zstr_free (&self->rest_root);
		// Free object itself
		free (self);
		*self_ptr = NULL;
	}

}

static char*
s_readall (const char* filename) {
    FILE *fp = fopen(filename, "rt");
    if (!fp)
        return NULL;

    size_t fsize = 0; 
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *ret = (char*) malloc (fsize * sizeof (char) + 1);
    if (!ret) {
        fclose (fp);
        return NULL;
    }    
    memset ((void*) ret, '\0', fsize * sizeof (char) + 1);

    size_t r = fread((void*) ret, 1, fsize, fp); 
    fclose (fp);
    if (r == fsize)
        return ret; 

    free (ret);
    return NULL;
}

//  --------------------------------------------------------------------------
//  Create a new fty_info_server

void
fty_info_server (zsock_t *pipe, void *args)
{
    char *name = (char *)args;
    if (!name) {
        zsys_error ("Adress for fty-info actor is NULL");
        return;
    }
    bool verbose = false;
    mlm_client_t *client = mlm_client_new ();
    zpoller_t *poller = zpoller_new (pipe, mlm_client_msgpipe (client), NULL);
    assert (poller);

    zsock_signal (pipe, 0); 
    zsys_info ("fty-info: Started");

    while (!zsys_interrupted)
    {
         void *which = zpoller_wait (poller, TIMEOUT_MS);
         if (which == NULL) {
             if (zpoller_terminated (poller) || zsys_interrupted) {
                 break;
             }
         }
         if (which == pipe) {
             if (verbose)
                 zsys_debug ("which == pipe");
             zmsg_t *message = zmsg_recv (pipe);
             if (!message)
                 break;
             
             char *command = zmsg_popstr (message);
             if (!command) {
                 zmsg_destroy (&message);
                 zsys_warning ("Empty command.");
                 continue;
             }
             if (streq(command, "$TERM")) {
                 zsys_info ("Got $TERM");
                 zmsg_destroy (&message);
                 zstr_free (&command);
                 break;
             }
             else
                 if (streq(command, "CONNECT"))
                 {
                     char *endpoint = zmsg_popstr (message);

                     if (endpoint) {
                         zsys_debug ("fty-info: CONNECT: %s/%s", endpoint, name);
                         int rv = mlm_client_connect (client, endpoint, 1000, name);
                         if (rv == -1)
                             zsys_error("mlm_client_connect failed\n");
                     }
                     zstr_free (&endpoint);
                 }
                 else
                     if (streq (command, "VERBOSE"))
                     {
                         verbose = true;
                         zsys_debug ("fty-info: VERBOSE=true");
                     }
                     else {
                         zsys_error ("fty-info: Unknown actor command: %s.\n", command);
                     }
             zstr_free (&command);
             zmsg_destroy (&message);
         }
         else
             if (which == mlm_client_msgpipe (client)) {
                 //TODO: implement actor interface
                 zmsg_t *message = mlm_client_recv (client);
                 if (!message)
                    break;

                 char *command = zmsg_popstr (message);
                 if (!command) {
                     zmsg_destroy (&message);
                     zsys_warning ("Empty command.");
                     continue;
                 }
                 if (streq(command, "VERSION")) {
                    zmsg_t *reply = zmsg_new ();
                    char *version = s_readall ("/etc/release-details.json");
                    if (version == NULL) {
                        zmsg_addstrf (reply, "%s", "ERROR");
                        zmsg_addstrf (reply, "%s", "Version info could not be found");
                        mlm_client_sendto (client, mlm_client_sender (client), "fty-info", NULL, 1000, &reply);
                    }
                    else {
                        zmsg_addstrf (reply, "%s", "VERSION");
                        zmsg_addstrf (reply, "%s", version);
                        mlm_client_sendto (client, mlm_client_sender (client), "fty-info", NULL, 1000, &reply);
                    }
                 }
                 else {
                     zsys_error ("fty-info: Unknown actor command: %s.\n", command);
                 }
                 zstr_free (&command);
                 zmsg_destroy (&message);
             }
    }
    zpoller_destroy (&poller);
    mlm_client_destroy (&client);
}

//  --------------------------------------------------------------------------
//  Self test of this class

void
fty_info_server_test (bool verbose)
{
    printf (" * fty_info_server: ");

    //  @selftest

   static const char* endpoint = "inproc://fty-info-test";

   zactor_t *server = zactor_new (mlm_server, (void*) "Malamute");
   zstr_sendx (server, "BIND", endpoint, NULL);
   if (verbose)
       zstr_send (server, "VERBOSE");

   mlm_client_t *ui = mlm_client_new ();
   mlm_client_connect (ui, endpoint, 1000, "UI");
   /*
   zactor_t *info_server = zactor_new (fty_info_server, (void*) "fty-info-test");
   if (verbose)
       zstr_send (info_server, "VERBOSE");
   zstr_sendx (info_server, "CONNECT", endpoint, NULL);
   zclock_sleep (1000);

    // Test #1: command VERSION
    {
        zmsg_t *command = zmsg_new ();
        zmsg_addstrf (command, "%s", "VERSION");
        mlm_client_sendto (ui, "fty-info-test", "fty-info", NULL, 1000, &command);

        zmsg_t *recv = mlm_client_recv (ui);

        assert (zmsg_size (recv) == 2);
        char * foo = zmsg_popstr (recv);
        if (streq (foo, "VERSION")) {
            zstr_free (&foo);
            foo = zmsg_popstr (recv);
            zsys_debug ("Received version: \n%s", foo);
        }
        else if (streq (foo, "ERROR")) {
            zstr_free (&foo);
            foo = zmsg_popstr (recv);
            zsys_debug ("Received error: \n%s", foo);
        }
        else {
            zsys_warning ("Unexpected message: commmand = '%s', sender = '%s', subject = '%s'",
                    mlm_client_command (ui),
                    mlm_client_sender (ui),
                    mlm_client_subject (ui));
        }
        zstr_free (&foo);
        zmsg_destroy (&recv);
    }
	// Test #2: create/destroy test for fty_info_t
	{
		fty_info_t *self = fty_info_new ();
		if (self) {
			printf ("fty_info object created successfully\n");
			fty_info_destroy (&self);
		}
	}
   */
   
    //  @end
    printf ("OK\n");
    //   zactor_destroy (&info_server);
    mlm_client_destroy (&ui);
    zactor_destroy (&server);
}
