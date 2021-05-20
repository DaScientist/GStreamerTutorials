/*
 * =====================================================================================
 *
 *       Filename:  complexTutorial.c
 *
 *    Description:  Gstreamer Tutorials Tutorial 2
 *
 *        Version:  1.0
 *        Created:  Thursday 20 May 2021 08:32:17  IST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mustanseer Sakerwala
 *        Company:  SVIT, Vasad 
 *
 * =====================================================================================
 */

#include<gst/gst.h>
int main ( int argc, char *argv[] )
{
	GstElement *pipeline, *sink, *source;
	GstBus *bus;
	GstMessage *msg;
	GstStateChangeReturn ret;
	
	
	/*-----------------------------------------------------------------------------
	 *  Initialize GST
	 *-----------------------------------------------------------------------------*/
	gst_init(&argc, &argv);
	
	
	/*-----------------------------------------------------------------------------
	 *  Create the elements
	 *-----------------------------------------------------------------------------*/
	source = gst_element_factory_make("videotestsrc", "source");
	sink = gst_element_factory_make("autovideosink", "sink");

	/*-----------------------------------------------------------------------------
	 *  Create an empty pipeline
	 *-----------------------------------------------------------------------------*/
	pipeline = gst_pipeline_new("test-pipeline");

	if(!pipeline || !source || !sink){
		g_printerr("Not all elements could be created.\n");
		return -1;
	}


	/*-----------------------------------------------------------------------------
	 *  Build the pipeline
	 *-----------------------------------------------------------------------------*/
	gst_bin_add_many (GST_BIN(pipeline), source, sink, NULL);
	if (gst_element_link (source, sink) !=	TRUE) {
		g_printerr ("Elements could not be linked.\n");
		gst_object_unref(pipeline);
		return -1;
	}

	
	/*-----------------------------------------------------------------------------
	 *  Modify the source's properties
	 *-----------------------------------------------------------------------------*/
	g_object_set(source, "pattern", 24, NULL);



	/*-----------------------------------------------------------------------------
	 *  Start Playing
	 *-----------------------------------------------------------------------------*/
	ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE){
		g_printerr("Unable to change the state");
		gst_object_unref(pipeline);
		return -1;
	}



	/*-----------------------------------------------------------------------------
	 *  Wait until error or EOS
	 *-----------------------------------------------------------------------------*/
	bus = gst_element_get_bus (pipeline);
	msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);


	/*-----------------------------------------------------------------------------
	 *  Parse message
	 *-----------------------------------------------------------------------------*/
	if(msg != NULL){
		GError *err;
		gchar *debug_info;
		

		switch(GST_MESSAGE_TYPE(msg)){
			case GST_MESSAGE_ERROR:
				gst_message_parse_error (msg, &err, &debug_info);
				g_printerr("Error recieved from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
				g_printerr("Debugging info: %s\n", debug_info ? debug_info : "None");
				g_clear_error(&err);
				g_free(debug_info);
				break;
			case GST_MESSAGE_EOS:
				g_print("End-Of-Stream reached");
				break;
			default:

				/*-----------------------------------------------------------------------------
				 *  We should not reach here
				 *-----------------------------------------------------------------------------*/
				g_printerr("Unexpected error recieved!\n");
				break;
		}
		gst_message_unref(msg);
	}



	/*-----------------------------------------------------------------------------
	 *  Free Resources
	 *-----------------------------------------------------------------------------*/
	gst_object_unref(bus);
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);
	
	return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */



