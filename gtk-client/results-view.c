//
// Created by sergios on 30/6/2023.
//
#include "results-view.h"

G_DEFINE_TYPE (ResultsView, results_view, GTK_TYPE_SCROLLED_WINDOW)

static void results_view_class_init(ResultsViewClass *klass) {
  // Add your class initialization code here
}

static void results_view_init(ResultsView *widget) {
  GtkWidget *frame = gtk_frame_new ("Results");
  gtk_widget_set_hexpand (frame, TRUE);
  gtk_widget_set_vexpand (frame, TRUE);
  gtk_widget_set_margin_top (frame, 5);
  gtk_widget_set_margin_bottom (frame, 5);
  gtk_widget_set_margin_start (frame, 5);
  gtk_widget_set_margin_end(frame, 5);
  gtk_container_add (GTK_CONTAINER (widget), frame);

  widget->view = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (widget->view), FALSE);
  gtk_container_add (GTK_CONTAINER (frame), widget->view);

  gtk_widget_show_all (GTK_WIDGET (widget));
}