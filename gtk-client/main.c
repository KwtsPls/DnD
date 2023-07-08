#include <gtk/gtk.h>
#include "query-editor.h"
#include "results-view.h"
#include "../utils/networking_utils.h"

GList *server_addresses = NULL;
GtkWidget *query_editor;
GtkWidget *results_view;
GtkWidget *send_button;
GSocketConnection *leader_connection = NULL;

gboolean
has_valid_leader ()
{
  if (leader_connection == NULL)
    {
      g_warning ("No leader.");
      return FALSE;
    }

  if (ping (leader_connection) == FALSE)
    {
      g_warning ("Disconnected leader (ping).");
      leader_connection = NULL;
      return FALSE;
    }

  return TRUE;
}

static GSocketConnection*
poll_server (gchar *address)
{
  printf("Connecting to: %s\n", address);

  GSocketConnection *connection = NULL;
  GSocketClient *client = g_socket_client_new();
  connection = g_socket_client_connect_to_host (client,
                                                address,
                                                1312, /* your port goes here */
                                                NULL,
                                                NULL);
  if (connection == NULL)
    return NULL;

  write_to_connection_str (connection, "LEADER_SYN");

  return connection;
}

static GSocketConnection*
connect_to_leader (void)
{
  GList *connections = NULL;
  for (GList *lp = server_addresses; lp != NULL; lp = lp->next)
    {
      GSocketConnection *connection = poll_server (((GString*)lp->data)->str);
      if (connection != NULL)
        connections = g_list_prepend (connections, connection);
    }

  const gint MAX_TRIES = 10;
  for (int i = 0; i < MAX_TRIES; i++)
    {
      for (GList *lp = connections; lp != NULL; lp = lp->next)
        {
          GSocketConnection *connection = lp->data;
          gchar input[256];
          gsize bytes_read = read_from_connection_str (connection, input);

          if (bytes_read > 0 && strncmp (input, "LEADER_ACK", 10) == 0)
            {
              connections = g_list_remove_link (connections, lp);
              g_list_free_full (connections, (GDestroyNotify) g_io_stream_close);
              return connection;
            }
        }
      g_usleep (10000); // sleep for 1/100th of a second
    }

  return NULL;
}

static void
connect_button_clicked (GtkButton *button,
                        gpointer  *data)
{
  if (has_valid_leader ())
    return;

  GSocketConnection *connection = connect_to_leader ();
  if (connection != NULL)
    {
      printf ("Found leader!\n");
      leader_connection = connection;
    }
  else
    {
      printf ("Didn't find a leader!\n");
      connection = NULL;
    }
}

static void
send_button_clicked (GtkButton *button,
                     gpointer  *data)
{
  if (has_valid_leader () == FALSE)
    return;

  GtkTextIter start, end;
  GtkTextBuffer *buffer = gtk_text_view_get_buffer((GtkTextView *) QUERY_EDITOR (query_editor)->editor);
  GString *query;

  gtk_text_buffer_get_bounds (buffer, &start, &end);
  query = g_string_new (gtk_text_buffer_get_text (buffer, &start, &end, FALSE));
  g_string_prepend (query, "QUERY: ");

  gsize bytes_written = write_to_connection_str (leader_connection, query);

  g_string_free (query, FALSE);

  printf ("Bytes written %d\n", bytes_written);
  if (bytes_written == -1)
    {
      g_warning ("Disconnected leader.");
      leader_connection = NULL;
      return;
    }

  gchar input[256];
  gsize bytes_read = read_from_connection_str (leader_connection, input);

  printf ("Bytes read %lu: %s\n", bytes_read, input);
  if (bytes_read == -1)
    {
      g_warning ("Disconnected leader (read).");
      leader_connection = NULL;
      return;
    }

  buffer = gtk_text_view_get_buffer((GtkTextView *) RESULTS_VIEW (results_view)->view);
  gtk_text_buffer_set_text (buffer, input, bytes_read - 1);
}

static void
activate (GtkApplication* app,
          gpointer        user_data)
{
  GtkWidget *window;
  GtkWidget *grid;
  GtkWidget *button;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Distributed Database");
  gtk_window_set_default_size (GTK_WINDOW (window), 900, 600);
  gtk_widget_show_all (window);

  grid = gtk_grid_new ();
  gtk_grid_set_column_spacing (GTK_GRID (grid), 15);
  gtk_grid_set_row_spacing (GTK_GRID (grid), 5);
  gtk_widget_set_halign (grid, GTK_ALIGN_FILL);
  gtk_widget_set_valign (grid, GTK_ALIGN_FILL);
  gtk_container_add (GTK_CONTAINER (window), grid);

  button = gtk_button_new_with_label ("Connect");
  gtk_widget_set_hexpand (button, FALSE);
  gtk_widget_set_vexpand (button, FALSE);
  gtk_widget_set_halign (button, GTK_ALIGN_CENTER);
  gtk_grid_attach (GTK_GRID (grid), button, 1, 1, 1, 1);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (connect_button_clicked), NULL);

  query_editor = g_object_new (QUERY_EDITOR_TYPE, NULL);
  gtk_widget_set_hexpand (query_editor, TRUE);
  gtk_widget_set_vexpand (query_editor, TRUE);
  gtk_grid_attach (GTK_GRID (grid), query_editor, 0, 0, 2, 1);

  send_button = gtk_button_new_with_label ("Send Query");
  gtk_widget_set_hexpand (send_button, FALSE);
  gtk_widget_set_vexpand (send_button, FALSE);
  gtk_widget_set_halign (send_button, GTK_ALIGN_CENTER);
  gtk_grid_attach (GTK_GRID (grid), send_button, 0, 1, 1, 1);

  g_signal_connect (G_OBJECT (send_button), "clicked", G_CALLBACK (send_button_clicked), NULL);

  results_view = g_object_new (RESULTS_VIEW_TYPE, NULL);
  gtk_widget_set_hexpand (results_view, TRUE);
  gtk_widget_set_vexpand (results_view, TRUE);
  gtk_grid_attach (GTK_GRID (grid), results_view, 0, 2, 2, 1);

  gtk_widget_show_all (grid);
}

static gchar *servers_filepath = NULL;

static GOptionEntry entries[] =
    {
        { "addresses", 'a', 0, G_OPTION_ARG_STRING, &servers_filepath, "Filepath containing the addresses of the servers used in the system", NULL },
        { NULL }
    };

int
main (int    argc,
      char **argv)
{
  GtkApplication *app;
  int status;

  GError *error = NULL;
  GOptionContext *context;

  context = g_option_context_new ("- graphical user interface for DnD");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_add_group (context, gtk_get_option_group (TRUE));
  if (!g_option_context_parse (context, &argc, &argv, &error))
    g_error ("option parsing failed: %s\n", error->message);

  if (servers_filepath == NULL)
    g_error ("please provide a server file.\n");

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
      server_addresses = g_list_prepend (server_addresses, g_string_new (line));
    }

  fclose(fp);

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}