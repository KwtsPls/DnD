//
// Created by sergios on 30/6/2023.
//
#include "query-editor.h"

G_DEFINE_TYPE (QueryEditor, query_editor, GTK_TYPE_SCROLLED_WINDOW)

static void query_editor_class_init(QueryEditorClass *klass) {
  // Add your class initialization code here
}

static void query_editor_init(QueryEditor *widget) {
  GtkWidget *frame = gtk_frame_new ("Query Editor");
  gtk_widget_set_hexpand (frame, TRUE);
  gtk_widget_set_vexpand (frame, TRUE);
  gtk_widget_set_margin_top (frame, 5);
  gtk_widget_set_margin_bottom (frame, 5);
  gtk_widget_set_margin_start (frame, 5);
  gtk_widget_set_margin_end(frame, 5);
  gtk_container_add (GTK_CONTAINER (widget), frame);

  widget->editor = gtk_text_view_new ();
  gtk_container_add (GTK_CONTAINER (frame), widget->editor);

  gtk_widget_show_all (GTK_WIDGET (widget));
}