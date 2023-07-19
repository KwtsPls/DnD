//
// Created by sergios on 28/6/2023.
//
#include <stdio.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include "../utils/networking_utils.h"
#include "../rdbms/db_files/db_file.h"

static Database *db;

void load_database (void);

void load_database_from_fragments (const gchar *input);
typedef struct {
  gchar *address;
  time_t id;
  GSocketConnection *connection;
  gboolean is_leader;
} Peer;

Peer*
peer_new (gchar *address)
{
  Peer *p = malloc (sizeof (Peer));
  p->address = g_strdup (address);
  p->id = 0;
  p->connection = NULL;
  p->is_leader = FALSE;
  return p;
}

void
peer_delete (Peer *p)
{
  free (p->address);
  free (p);
}

GList *peers = NULL;
Peer *self = NULL;
gboolean db_loading = FALSE;

static gchar *servers_filepath = NULL;
static gchar *database_folderpath = NULL;
const int FRAGMENT_COUNT = 4;

GString*
filepath_of_fragment(char* frag)
{
  GString *filepath = g_string_new (database_folderpath);
  g_string_append (filepath, "data");
  g_string_append (filepath, frag);
  g_string_append (filepath, "/");
  return filepath;
}

GString*
records_list_to_string (GList *records)
{
  GString *s = g_string_new ("");
  for (GList *lp = records; lp != NULL; lp = lp->next)
    {
      Record *r = lp->data;
      GString *rs = record_to_string (r);
      g_string_append (s, rs->str);
      g_string_append_c(s, '\n');
      g_string_free (rs, FALSE);
    }
  return s;
}

gpointer
client_main (gpointer data)
{
  GSocketConnection *connection = data;
  gchar input[256];

  while (TRUE)
    {
      gsize bytes_read = read_from_connection_str (connection, input);
      if (bytes_read > 0)
        {
          while(db_loading == TRUE);

          printf ("Got %lu bytes: %s\n", bytes_read, input);
          if (strncmp (input, "PING", 4) == 0)
            {
              write_to_connection_str (connection, "PING");
            }
          else if (strncmp (input, "QUERY", 5) == 0)
            {
              if (self->is_leader == FALSE)
                g_error ("Got a QUERY command without being the leader.");
              printf("(Client) Query to execute:\n\t%s\n", input);

              clock_t start, end;
              double cpu_time_used;

              start = clock();

              // send query request to peers
              for (GList *lp = peers; lp != NULL; lp = lp->next)
                {
                  Peer *p = lp->data;
                  if (p->connection == NULL)
                    continue;

                  gsize bytes_written = write_to_connection_str (p->connection, input);

                  printf ("Bytes written %d\n", bytes_written);
                  if (bytes_written == -1)
                    {
                      printf ("Disconnected peer.\n");
                      p->connection = NULL;
                      load_database();
                      write_to_connection_str (connection, "Try again!");
                      return NULL;
                      // TODO: Rerun query.
                    }
                }

              // execute query
              GList *results = database_query (db, input + 7);

              // combine results and respond
              GString *result = records_list_to_string (results);

              // get query results from peers
              for (GList *lp = peers; lp != NULL; lp = lp->next)
                {
                  Peer *p = lp->data;
                  if (p->connection == NULL)
                    continue;

                  gchar input[256];
                  gssize bytes_read = read_from_connection_str (p->connection, input);

                  printf ("Bytes read %ld: %s\n", bytes_read, input);
                  if (bytes_read == -1)
                    {
                      printf ("Disconnected peer (read).\n");
                      p->connection = NULL;
                      load_database();
                      write_to_connection_str (connection, "Try again!");
                      return NULL;
                      // TODO: Rerun query.
                    }
                  else
                    g_string_append (result, input);
                }

              end = clock();
              cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
              printf("Loading time: %lf\n", cpu_time_used);

              write_to_connection_str (connection, result->str);
              g_string_free (result, FALSE);
            }
        }
      else if (ping (connection) == FALSE)
        break;
      usleep (100000);
    }

  g_object_unref (connection);

  return NULL;
}

gboolean
client_connection (GSocketService     *service,
                   GSocketConnection  *connection,
                   GObject            *source_object,
                   gpointer            user_data)
{
  GSocketAddress *sockaddr = g_socket_connection_get_remote_address (connection, NULL);
  GInetAddress   *addr = g_inet_socket_address_get_address (G_INET_SOCKET_ADDRESS (sockaddr));
  guint16         port = g_inet_socket_address_get_port (G_INET_SOCKET_ADDRESS (sockaddr));

  printf ("New Connection from %s:%d\n", g_inet_address_to_string (addr), port);

  g_object_ref (connection);

  gchar input[256];
  gsize bytes_read = read_from_connection_str (connection, input);

  printf("Got %lu bytes: %s\n", bytes_read, input);

  if ((strncmp (input, "LEADER_SYN", 10) == 0) && self->is_leader == TRUE)
    {
      printf ("Sending LEADER_ACK\n");
      write_to_connection_str (connection, "LEADER_ACK");
      g_thread_new (NULL, client_main, connection);
    }
  else
    g_object_unref (connection);

  return TRUE;
}

gpointer
peer_main (gpointer data)
{
  GSocketConnection *connection = data;
  gchar input[256];

  while (TRUE)
    {
      gsize bytes_read = read_from_connection_str (connection, input);
      if (bytes_read > 0)
        {
          while(db_loading == TRUE);

//          printf ("Got %lu bytes: %s\n", bytes_read, input);
          if (strncmp (input, "PING", 4) == 0)
            {
              write_to_connection_str (connection, "PING");
            }
          else if (strncmp (input, "COORDINATOR", 11) == 0)
            {

            }
          else if (strncmp (input, "LOAD", 4) == 0)
            {
              db_loading = TRUE;
              load_database_from_fragments (input);
              db_loading = FALSE;
              // FIXME: Should send a response that confirms the operation.
            }
          else if (strncmp (input, "QUERY", 5) == 0)
            {
              printf("(Peer) Query to execute:\n\t%s\n", input);

              // execute query
              GList *results = database_query (db, input + 7);

              printf("(Peer) Query executed\n");

              GString *result = records_list_to_string (results);

              printf("(Peer) Result: %s\n", result->str);

              write_to_connection_str (connection, result->str);
              g_string_free (result, FALSE);
            }
        }
      else if (ping (connection) == FALSE)
        break;
      usleep (100000);
    }

  printf ("Thread exiting\n");
  g_object_unref (connection);

  return NULL;
}

void load_database_from_fragments (const gchar *input)
{
  if (db != NULL)
    database_close (db);
  gchar** fragments = g_regex_split_simple(",", input + 6, 0, 0);
  db = database_open (filepath_of_fragment (fragments[0])->str);
  for (int i = 1; i < g_strv_length (fragments) - 1; i++)
    {
      database_open_existing(db, filepath_of_fragment (fragments[i])->str);
    }
  g_strfreev (fragments);
}

gboolean
peer_connection (GSocketService     *service,
                 GSocketConnection  *connection,
                 GObject            *source_object,
                 gpointer            user_data)
{
  GSocketAddress *sockaddr = g_socket_connection_get_remote_address (connection, NULL);
  GInetAddress   *addr = g_inet_socket_address_get_address (G_INET_SOCKET_ADDRESS (sockaddr));
  guint16         port = g_inet_socket_address_get_port (G_INET_SOCKET_ADDRESS (sockaddr));

  printf ("New Connection from %s:%d\n", g_inet_address_to_string (addr), port);

  g_object_ref (connection);

  gchar input[256];
  gsize bytes_read = read_from_connection_str (connection, input);

  printf("Got %lu bytes: %s\n", bytes_read, input);

  if (strncmp (input, "ID_REQ", 6) == 0)
    {
      printf ("Sending ID\n");
      write_to_connection_long (connection, self->id);
    }

  g_thread_new (NULL, peer_main, connection);

  return TRUE;
}

gboolean
connect_with_peer (gpointer data)
{
  if (db_loading == TRUE)
    return G_SOURCE_CONTINUE;

  Peer *p = data;

//  printf("Connect with peer: %s\n", p->address);

  // there is a valid connection, do nothing
  if (p->connection != NULL && ping (p->connection) == TRUE)
    return G_SOURCE_CONTINUE;

  gboolean reload_db = FALSE;
  if (p->connection != NULL)
    {
      p->connection = NULL;
      reload_db = TRUE;
    }

  printf("Attempting to connect to: %s\n", p->address);

  GSocketConnection *connection = NULL;
  GSocketClient *client = g_socket_client_new ();
  connection = g_socket_client_connect_to_host (client,
                                                p->address,
                                                1313, /* your port goes here */
                                                NULL,
                                                NULL);

  if (connection != NULL)
    {
      p->connection = connection;
      printf("Sending ID_REQ to: %s\n", p->address);
      write_to_connection_str (p->connection, "ID_REQ");
      long buffer;
      gsize bytes_read = read_from_connection_long(p->connection, &buffer);
      if (bytes_read > 0)
        {
          p->id = buffer;
          printf("Got ID: %ld\n", p->id);
        }
      if (self->is_leader)
        load_database();
    }
  else if (reload_db && self->is_leader)
    {
      load_database();
    }

  return G_SOURCE_CONTINUE;
}

void
seniority_succession_algorithm (void)
{
  if (self->is_leader == TRUE)
    return; // I am the leader

  for (GList *lp = peers; lp != NULL; lp = lp->next)
    {
      Peer *p = lp->data;
//      printf("Connection %p: %ld ?? %ld\n", p->connection, p->id, self->id);
      if (p->connection != NULL && p->id < self->id)
        return; // I shouldn't be the leader
    }

  // I am the oldest process in the network, I should be the leader
  for (GList *lp = peers; lp != NULL; lp = lp->next)
    {
      Peer *p = lp->data;
      if (p->connection != NULL)
        write_to_connection_str (p->connection, "COORDINATOR");
    }

  printf ("Assuming leadership\n");
  self->is_leader = TRUE;
  load_database ();
}

void
load_database (void)
{
  if (self->is_leader == FALSE)
    g_error("Non-leader trying to load the database of the system.");

  printf("DATABASE LOADING STARTED!\n");

  db_loading = TRUE;

  guint peer_count = 1;
  for (GList *lp = peers; lp != NULL; lp = lp->next)
    {
      Peer *p = lp->data;
      if (p->connection != NULL && ping (p->connection) == TRUE)
        peer_count++;
    }

  GString* fragments_per_peer[peer_count];
  for (int i = 0; i < peer_count; i++)
    fragments_per_peer[i] = g_string_new ("LOAD: ");

  for (int i = 0; i < FRAGMENT_COUNT; i++)
    {
      char str[4];
      sprintf(str, "%d,", i);
      g_string_append (fragments_per_peer[i % peer_count], str);
    }

  for (int i = 0; i < peer_count; i++)
    printf("Peer %d: %s\n", i, fragments_per_peer[i]->str);

  int i = 1;
  for (GList *lp = peers; lp != NULL; lp = lp->next)
    {
      Peer *p = lp->data;
      if (p->connection != NULL && ping (p->connection) == TRUE)
        {
          write_to_connection_str (p->connection, fragments_per_peer[i++]->str);
        }
    }

  load_database_from_fragments (fragments_per_peer[0]->str);

  sleep (1); // workaround for the FIXME that follows

  // FIXME: Should wait for a response that confirms the operation on each peer.

  db_loading = FALSE;

  printf("DATABASE LOADING FINISHED!\n");
}

static GOptionEntry entries[] =
    {
        { "addresses", 'a', 0, G_OPTION_ARG_STRING, &servers_filepath, "File containing the addresses of the servers used in the system.", NULL },
        { "database", 'd', 0, G_OPTION_ARG_STRING, &database_folderpath, "Directory containing the database files.",                       NULL },
        { NULL }
    };

int
main (int    argc,
      char **argv)
{
  self = peer_new ("127.0.0.1");
  self->id = time (NULL);

  GError *error = NULL;
  GOptionContext *context;

  context = g_option_context_new ("- node of the DnD distributed database");
  g_option_context_add_main_entries (context, entries, NULL);
  if (!g_option_context_parse (context, &argc, &argv, &error))
    g_error ("option parsing failed: %s\n", error->message);

  if (servers_filepath == NULL)
    g_error ("please provide a server file.\n");

  if (database_folderpath == NULL)
    g_error ("please provide a database directory.\n");

  FILE * fp;
  char * line = NULL;
  size_t len = 0;

  fp = fopen(servers_filepath, "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);

  while (getline(&line, &len, fp) != -1)
    {
      g_strchomp (line);
      printf("Read line: %s\n", line);
      Peer *p = peer_new (line);
      peers = g_list_prepend (peers, p);
      connect_with_peer (p);
      g_timeout_add (200, connect_with_peer, p);
    }

  fclose(fp);

  g_timeout_add (400, (GSourceFunc) seniority_succession_algorithm, NULL);

  GSocketService *client_listener = g_socket_service_new ();
  g_socket_listener_add_inet_port (G_SOCKET_LISTENER (client_listener), 1312, NULL, NULL);
  g_signal_connect (client_listener, "incoming", G_CALLBACK (client_connection), NULL);
  g_socket_service_start (client_listener);

  GSocketService *net_listener = g_socket_service_new ();
  g_socket_listener_add_inet_port (G_SOCKET_LISTENER (net_listener), 1313, NULL, NULL);
  g_signal_connect (net_listener, "incoming", G_CALLBACK (peer_connection), NULL);
  g_socket_service_start (net_listener);

  GMainLoop *loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);

  return 0;
}
