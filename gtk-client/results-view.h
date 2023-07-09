//
// Created by sergios on 30/6/2023.
//
#ifndef _RESULTS_VIEW_H_
#define _RESULTS_VIEW_H_

#include <gtk/gtk.h>

typedef struct _ResultsView ResultsView;
typedef struct _ResultsViewClass ResultsViewClass;

#define RESULTS_VIEW_TYPE (results_view_get_type())
#define RESULTS_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), RESULTS_VIEW_TYPE, ResultsView))
#define RESULTS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), RESULTS_VIEW_TYPE, ResultsViewClass))
#define IS_RESULTS_VIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), RESULTS_VIEW_TYPE))
#define IS_RESULTS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), RESULTS_VIEW_TYPE))
#define RESULTS_VIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), RESULTS_VIEW_TYPE, ResultsViewClass))

struct _ResultsView {
    GtkScrolledWindow parent_instance;
    // Add your custom fields here

    GtkWidget *view;
};

struct _ResultsViewClass {
    GtkScrolledWindowClass parent_class;
    // Add your custom class fields here
};

GType results_view_get_type (void) G_GNUC_CONST;

#endif //_RESULTS_VIEW_H_
