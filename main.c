#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <ccv.h>
#include <picam.h>

int main(int argc, char** argv){
	int x, y;
	char *file = argv[3];

	x = atoi(argv[1]);
	y = atoi(argv[2]);

	// enable default cache
	ccv_enable_default_cache();

	// initialize picam variables
	PicamParams prms;
	prms.exposure = 1;
	prms.meterMode = 1;
	prms.imageFX = 0;
	prms.awbMode = 1;
	prms.ISO = 0;
	prms.sharpness = 10;
	prms.contrast = 20;
	prms.brightness = 70;
	prms.saturation = 0;
	prms.videoStabilisation = 1;
	prms.exposureCompensation = 1;
	prms.rotation = 0;
	prms.hflip = 0;
	prms.vflip = 0;
	prms.shutter_speed = 0;
	prms.videoProfile = 0;
	prms.videoBitrate = 17000000;
	prms.videoFramerate = 30;
	prms.quantisationParameter = 0;
	prms.inlineHeaders = 0;

	ccv_dense_matrix_t *firstFrame = 0;
	// read the first image into our dense matrix
	ccv_read(file, &firstFrame, CCV_IO_ANY_FILE | CCV_IO_GRAY);

	// initialize the tracking box
	ccv_rect_t trackingBox = ccv_rect(x, y, 400, 400);

	// tune parameters?
	/*
	ccv_tld_param_t params = {
		.win_size = {
			9,
			9
		},
		.level = 5,
		.min_forward_backward_error = 100,
		.min_eigen = 0.025,
		.interval = 3,
		.shift = 0.02,
		.min_win = 20,
		.include_overlap = 0.7,
		.exclude_overlap = 0.2,
		.structs = 25,
		.features = 18,
		.validate_set = 0.5,
		.nnc_same = 0.95,
		.nnc_thres = 0.55,
		.nnc_verify = 0.67,
		.nnc_beyond = 0.85,
		.nnc_collect = 0.50,
		.bad_patches = 100,
		.new_deform = 15,
		.new_deform_angle = 10,
		.new_deform_scale = 0.05,
		.new_deform_shift = 0.05,
		.track_deform = 10,
		.track_deform_angle = 10,
		.track_deform_scale = 0.02,
		.track_deform_shift = 0.02,
		.rotation = 1,
		.top_n = 100
	};
	*/

	// setup the tracker
	ccv_tld_t* tracker = ccv_tld_new(firstFrame, trackingBox, ccv_tld_default_params); 

	ccv_dense_matrix_t *lastFrame = firstFrame;
	ccv_dense_matrix_t *currentFrame = 0;

	for(;;){
		// take a picture with the picam and set it as the current frame
		long size = 0L;
		uint8_t* currentFrameData = takePhotoWithDetails(400, 400, 85, &prms, &size);
		ccv_read((const void*)currentFrameData, &currentFrame, CCV_IO_ANY_STREAM | CCV_IO_GRAY, size);

		// struct with information about the new tracked frame
		ccv_tld_info_t info;

		// returns a tracking box
		ccv_comp_t box = ccv_tld_track_object(tracker, lastFrame, currentFrame,  &info);

		//display the tracked data if we have a positive match
		if(tracker->found){
			printf("(%d, %d), confidence: %f\n", (box.rect.x + box.rect.width)/2, (box.rect.y + box.rect.height)/2, box.confidence);
			ccv_dense_matrix_t *borderFrame = 0;
			ccv_margin_t margin = ccv_margin(box.rect.x, box.rect.y, 400 - (box.rect.width + box.rect.x), 400 - (box.rect.height + box.rect.y));
			// write the current frame to a file
			char fn[100];
			sprintf(fn, "./frame-%04d.jpeg", tracker->count);
			ccv_border(currentFrame, (ccv_matrix_t**)&borderFrame, 0, margin);
			ccv_write(borderFrame, fn, 0, CCV_IO_JPEG_FILE, 0);
			
		}else{
			printf("Not found!\n");
		}

		free(currentFrameData);		

		//setup the next loop
		lastFrame = currentFrame;
		currentFrame = 0;

		//delay of 1 sec
		//sleep(1);
	}
	
	ccv_tld_free(tracker);
	ccv_disable_cache();

	return 0;
}
