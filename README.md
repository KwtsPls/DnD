# DnD

A distributed (peer-to-peer) database that supports a simple SQLite language.

## Architecture

The system consists of three main components. The relational database management system, the distributed system and the Client Application.

![image](https://github.com/KwtsPls/DnD/assets/21060365/9f832851-f7ca-4a6f-a2f7-a478fa8a67e1)

## Code Structure

- `gtk-client`: a GTK application for interacting with the system.
- `net`: peer code
- `rdbms`: relational database code
- `utils`: utility functions
