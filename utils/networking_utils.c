//
// Created by sergios on 7/7/2023.
//
#include <gtk/gtk.h>
#include "networking_utils.h"

gsize
write_to_connection_long (GSocketConnection *connection,
                          long               message)
{
  if (g_io_stream_is_closed (G_IO_STREAM (connection)))
    return -1;
  GOutputStream *ostream = g_io_stream_get_output_stream (G_IO_STREAM (connection));
  gsize bytes_written =  g_output_stream_write (ostream,
                                                &message, /* your message goes here */
                                                sizeof (message), /* length of your message */
                                                NULL,
                                                NULL);
  return bytes_written;
}

gsize
read_from_connection_long (GSocketConnection *connection,
                           long              *buffer)
{
  if (g_io_stream_is_closed (G_IO_STREAM (connection)))
    return -1;
  GInputStream *istream = g_io_stream_get_input_stream (G_IO_STREAM (connection));
  return g_input_stream_read (istream,
                              buffer,
                              sizeof (*buffer),
                              NULL,
                              NULL);
}

gsize
write_to_connection_str (GSocketConnection *connection,
                         gchar             *message)
{
  GString *s = g_string_new (message);
  g_string_append_c (s, '\0');
  if (g_io_stream_is_closed (G_IO_STREAM (connection)))
    return -1;
  GOutputStream *ostream = g_io_stream_get_output_stream (G_IO_STREAM (connection));
  gsize bytes_written =  g_output_stream_write (ostream,
                                                s->str, /* your message goes here */
                                                s->len, /* length of your message */
                                                NULL,
                                                NULL);
  g_string_free (s, FALSE);
  return bytes_written;
}

gsize
read_from_connection_str (GSocketConnection *connection,
                          gchar             *buffer)
{
  printf("Hello\n");
  if (g_io_stream_is_closed (G_IO_STREAM (connection)))
    return -1;
  printf("World\n");
  GInputStream *istream = g_io_stream_get_input_stream (G_IO_STREAM (connection));
  return g_input_stream_read (istream,
                              buffer,
                              255,
                              NULL,
                              NULL);
}

gboolean
ping (GSocketConnection *connection)
{
  if (write_to_connection_str (connection, "PING") == -1)
    return FALSE;

  gchar input[256];
  if (read_from_connection_str (connection, input) < 4)
    return FALSE;

//  printf("%s\n", input);

  if (strncmp (input, "PING", 4) != 0)
    return FALSE;

  return TRUE;
}