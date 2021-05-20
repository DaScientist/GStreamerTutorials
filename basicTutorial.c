#include<stdio.h>
#include<gst/gst.h>

int main(int argc, char *argv[]){
	GstElement *pipeline;
	GstBus *bus;
	GstMessage *message;

//	printf("Ran the code till here");

	gst_init(&argc, &argv);
	pipeline = gst_parse_launch("playbin uri=https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm", NULL);

	gst_element_set_state(pipeline, GST_STATE_PLAYING);
	bus = gst_element_get_bus(pipeline);
	message = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS );

	/* Free Resources */
	if (message != NULL) {
		gst_message_unref (message);
	}
	gst_object_unref(bus);
	gst_element_set_state (pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);
	return 0;
}
