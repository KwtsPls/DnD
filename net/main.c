//
// Created by sergios on 28/6/2023.
//
#include <stdio.h>
#include <gtk/gtk.h>
#include "../utils/networking_utils.h"
#include "../rdbms/db_files/db_file.h"
#include "../rdbms/table/record.h"

static Database *db;

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

              // send query request to peers
              for (GList *lp = peers; lp != NULL; lp = lp->next)
                {
                  Peer *p = lp->data;
                  gsize bytes_written = write_to_connection_str (p->connection, input);

                  printf ("Bytes written %d\n", bytes_written);
                  if (bytes_written == -1)
                    {
                      g_warning ("Disconnected peer.");
                      // TODO: Call for the migration of the peer.
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
                  gchar input[256];
                  gsize bytes_read = read_from_connection_str (p->connection, input);

                  printf ("Bytes read %lu: %s\n", bytes_read, input);
                  if (bytes_read == -1)
                    {
                      g_warning ("Disconnected peer (read).");
                      // TODO: Call for the migration of the peer.
                    }

                  g_string_append (result, input);
                }

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
          printf ("Got %lu bytes: %s\n", bytes_read, input);
          if (strncmp (input, "PING", 4) == 0)
            {
              write_to_connection_str (connection, "PING");
            }
          else if (strncmp (input, "COORDINATOR", 11) == 0)
            {

            }
          else if (strncmp (input, "QUERY", 5) == 0)
            {
              printf("(Peer) Query to execute:\n\t%s", input);

              // execute query
              GList *results = database_query (db, input + 7);

              GString *result = records_list_to_string (results);
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
  Peer *p = data;

  // there is a valid connection, do nothing
  if (p->connection != NULL && ping (p->connection) == TRUE)
    return G_SOURCE_CONTINUE;

  p->connection = NULL;

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
      write_to_connection_str (p->connection, "ID_REQ");
      long buffer;
      gsize bytes_read = read_from_connection_long(p->connection, &buffer);
      if (bytes_read > 0)
        p->id = buffer;
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
      if (p->connection != NULL && p->id < self->id)
        return; // I shouldn't be the leader
    }

  // I am the oldest process in the network, I should be the leader
  for (GList *lp = peers; lp != NULL; lp = lp->next)
    {
      Peer *p = lp->data;
      if (p->connection != NULL)
        write_to_connection_str (p->connection, "COORDINATOR"); // TODO: The peers should check the ID.
    }

  printf ("Assuming leadership\n");
  self->is_leader = TRUE;
}

static gchar *servers_filepath = NULL;
static gchar *database_filepath = NULL;

static GOptionEntry entries[] =
    {
        { "addresses", 'a', 0, G_OPTION_ARG_STRING, &servers_filepath, "File containing the addresses of the servers used in the system.", NULL },
        { "database", 'd', 0, G_OPTION_ARG_STRING, &database_filepath, "Directory containing the database files.",                         NULL },
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

  if (database_filepath == NULL)
    g_error ("please provide a database directory.\n");

  db = database_open (database_filepath);

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
