/*
 * =====================================================================================
 *
 *       Filename:  DynamicTutorial.c
 *
 *    Description:  Tutorial 3: Dynamic Pipeline
 *
 *        Version:  1.0
 *        Created:  Friday 21 May 2021 03:56:48  IST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mustanseer Sakerwala (), mustanseer.170410107097@gmail.com
 *        Company:  SVIT, vasad
 *
 * =====================================================================================
 */


#include<gst/gst.h>

typedef struct CustomData {
	
	GstElement *pipeline;
	GstElement *source;
	GstElement *convert;
 	GstElement *resample;
 	GstElement *sink;

} CustomData;
static void pad_added_handler ( GstElement *src, GstPad *pad, CustomData *data );



int main ( int argc, char *argv[] )
{
	CustomData data;
	GstBus *bus;
	GstMessage *msg;
	GstStateChangeReturn ret;
	gboolean terminate = FALSE;

	/*-----------------------------------------------------------------------------
	 *  Initialize GST
	 *-----------------------------------------------------------------------------*/
	gst_init(&argc, &argv);
	
	/*-----------------------------------------------------------------------------
	 *  Initialize variables
	 *-----------------------------------------------------------------------------*/
	data.source = gst_element_factory_make("uridecodebin", "source");
	data.convert = gst_element_factory_make("audioconvert", "convert");
	data.resample = gst_element_factory_make("audioresample", "resample");
	data.sink = gst_element_factory_make("autoaudiosink", "sink");



	/*-----------------------------------------------------------------------------
	 *  Create empty pipeline
	 *-----------------------------------------------------------------------------*/
	data.pipeline = gst_pipeline_new("test-pipeline");


	if( !data.pipeline || !data.source || !data.convert || !data.resample || !data.sink ) {
		g_printerr( "Not all elements could be initialized.\n" );
		return -1;
	}



	/*-----------------------------------------------------------------------------
	 *  Build pipeline but donot link source right now.
	 *-----------------------------------------------------------------------------*/
	gst_bin_add_many ( GST_BIN(data.pipeline), data.source, data.convert, data.resample, data.sink, NULL );
	if ( !gst_element_link_many ( data.convert, data.resample, data.sink, NULL ) ) {
		g_printerr("Not all elements could be linked.\n");
		gst_object_unref (data.pipeline);
		return -1;
	}

	
	/*-----------------------------------------------------------------------------
	 *  Set URI to play
	 *-----------------------------------------------------------------------------*/
	g_object_set( data.source, "uri", "https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm", NULL );
	


	/*-----------------------------------------------------------------------------
	 *  Connect the pad-added Signal
	 *-----------------------------------------------------------------------------*/
	g_signal_connect( data.source, "pad-added", G_CALLBACK(pad_added_handler), &data );


	
	/*-----------------------------------------------------------------------------
	 *  Start playing
	 *-----------------------------------------------------------------------------*/
	ret = gst_element_set_state ( data.pipeline, GST_STATE_PLAYING );	
	if ( ret == GST_STATE_CHANGE_FAILURE ) {
		g_printerr("Unable to play the pipeline.\n");
		gst_object_unref (data.pipeline);
		return -1;
	}

	bus = gst_element_get_bus (data.pipeline);
	do {
		msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS );

		if(msg != NULL) {
			GError *err;
			gchar *debug_info;

			switch(GST_MESSAGE_TYPE (msg)){
				
				case GST_MESSAGE_ERROR:
					gst_message_parse_error (msg, &err, &debug_info);
			          	g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
					g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
					g_clear_error (&err);
					g_free (debug_info);
					terminate = TRUE;
					break;
				case GST_MESSAGE_EOS:
				        g_print ("End-Of-Stream reached.\n");
				        terminate = TRUE;
				        break;
				case GST_MESSAGE_STATE_CHANGED:
				        /* We are only interested in state-changed messages from the pipeline */
				        if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data.pipeline)) {
						GstState old_state, new_state, pending_state;
				        	gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
				            	g_print ("Pipeline state changed from %s to %s:\n",
				                gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
				        }
				        break;
				default:
				        /* We should not reach here */
				        g_printerr ("Unexpected message received.\n");
				        break;
			}
			gst_message_unref(msg);

		} 
	} while (!terminate);
	
	
	/* Free resources */
	gst_object_unref (bus);
	gst_element_set_state (data.pipeline, GST_STATE_NULL);
	gst_object_unref (data.pipeline);


	return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  pad_added_handler
 *  Description:  Handler for when a pad is added to the DEMUX
 * =====================================================================================
 */
static void pad_added_handler ( GstElement *src, GstPad *pad, CustomData *data )
{
	GstPad *sink_pad = gst_element_get_static_pad(data->convert, "sink");
	GstPadLinkReturn ret;
	GstCaps *new_pad_caps = NULL;
	GstStructure *new_pad_structure = NULL;
	const gchar *new_pad_type = NULL;
	
	g_print( "Recieved new pad '%s' from '%s':\n", GST_PAD_NAME(pad), GST_ELEMENT_NAME(src) );

	if( gst_pad_is_linked(sink_pad) ) {
		g_print("We are already linked, Ignoring.\n");
		goto exit;
	}


	/*-----------------------------------------------------------------------------
	 *  Check new pad's type
	 *-----------------------------------------------------------------------------*/
	new_pad_caps = gst_pad_get_current_caps ( pad );
	new_pad_structure = gst_caps_get_structure ( new_pad_caps, 0 );
	new_pad_type = gst_structure_get_name( new_pad_structure );
	if ( !g_str_has_prefix ( new_pad_type, "audio/x-raw" ) ) {
		g_print( "It has %s audio, which is not raw. hence ignoring.\n", new_pad_type );
		goto exit;
	}



	/*-----------------------------------------------------------------------------
	 *  Attempt the link
	 *-----------------------------------------------------------------------------*/
	ret = gst_pad_link ( pad, sink_pad );
	if ( GST_PAD_LINK_FAILED (ret) ) {
		g_print("Type is: '%s' but link failed.\n", new_pad_type );
	} else {
		g_print("Type is: '%s' and link succeded.\n", new_pad_type );
	}

exit:
	if(new_pad_caps != NULL) {
		gst_caps_unref( new_pad_caps );
	}
	gst_object_unref ( sink_pad );

}		/* -----  end of function pad_added_handler  ----- */
