//
// Created by sergios on 30/6/2023.
//
#ifndef _QUERY_EDITOR_H_
#define _QUERY_EDITOR_H_

#include <gtk/gtk.h>

typedef struct _QueryEditor QueryEditor;
typedef struct _QueryEditorClass QueryEditorClass;

#define QUERY_EDITOR_TYPE (query_editor_get_type())
#define QUERY_EDITOR(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), QUERY_EDITOR_TYPE, QueryEditor))
#define QUERY_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), QUERY_EDITOR_TYPE, QueryEditorClass))
#define IS_QUERY_EDITOR(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), QUERY_EDITOR_TYPE))
#define IS_QUERY_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), QUERY_EDITOR_TYPE))
#define QUERY_EDITOR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), QUERY_EDITOR_TYPE, QueryEditorClass))

struct _QueryEditor {
    GtkScrolledWindow parent_instance;
    // Add your custom fields here

    GtkWidget *editor;
};

struct _QueryEditorClass {
    GtkScrolledWindowClass parent_class;
    // Add your custom class fields here
};

GType query_editor_get_type (void) G_GNUC_CONST;

#endif //_QUERY_EDITOR_H_
