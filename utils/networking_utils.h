//
// Created by sergios on 7/7/2023.
//

#ifndef _NETWORKING_UTILS_H_
#define _NETWORKING_UTILS_H_

gsize write_to_connection_long (GSocketConnection *connection,
                                long               message);

gsize read_from_connection_long (GSocketConnection *connection,
                                 long              *buffer);

gsize write_to_connection_str (GSocketConnection *connection,
                               gchar             *message);

gssize read_from_connection_str (GSocketConnection *connection,
                                gchar             *buffer);

gboolean ping (GSocketConnection *connection);

#endif //_NETWORKING_UTILS_H_
