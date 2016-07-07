#include <stdio.h>
#include <cv.h>
#include <highgui.h>

#ifdef __DEBUG__
#define debugf(fmt, ...) 	\
	fprintf(stderr, "DBG %s(): " fmt "\n", __func__, ##__VA_ARGS__)
#else
#define debugf(fmt, ...)
#endif
#define warnf(fmt, ...) 	\
	fprintf(stderr, "WARN: %s(): " fmt "\n", __func__, ##__VA_ARGS__)
#define errorf(fmt, ...) 	\
	fprintf(stderr, "ERR: %s(): " fmt "\n", __func__, ##__VA_ARGS__)
#define perrorf(fmt, ...) 	\
	fprintf(stderr, "ERR: %s(): " fmt ": %s\n", __func__, ##__VA_ARGS__, strerror(errno))

#define CAMERA_TYPE 1

static char __filenamebuf [128];
static int __initialized = 0;
static CvFont __font;
static CvCapture *__capture = NULL;
static IplImage *__img_cap;

// Initialize the Camera Module
int rpicam_init (void) 
{
	if (!__initialized) {
		cvInitFont (&__font, CV_FONT_HERSHEY_PLAIN, 1.0, 1.0, 0, 1, 8);
		//__capture = cvCaptureFromCAM (CAMERA_TYPE);
		__capture = cvCreateCameraCapture(-1);
		cvSetCaptureProperty (__capture, CV_CAP_PROP_FRAME_WIDTH, 640);
		cvSetCaptureProperty (__capture, CV_CAP_PROP_FRAME_HEIGHT, 480);
		if (!__capture) {
			errorf("no video input source.  Did you remember to 'sudo modprobe bcm2835-v4l2'?\n");
			return (-1);
		}
		__img_cap = cvQueryFrame (__capture);
		__initialized = 1;
	}
	return (0);
}

// Capture an image
void rpicam_capture (void)
{
	if (!__initialized)
		rpicam_init();
	__img_cap = cvQueryFrame (__capture);
}

// Save the previously captured image to the specified filename with the specified index
// Resulting filename of the image will be '<fname>-<histid>.png'
void rpicam_save_image ( const char * fname, int histid )
{
	FILE * f;
	
	sprintf(__filenamebuf, "%s-%04d.png", fname, histid);
	f = fopen (__filenamebuf, "w");
	if (! f) {
		errorf ("error writing to %s", __filenamebuf);
		return;
	}
	cvPutText (__img_cap, __filenamebuf, cvPoint(6, __img_cap->height-9), &__font, cvScalar(0, 0, 0, 0));
	cvPutText (__img_cap, __filenamebuf, cvPoint(5, __img_cap->height-8), &__font, cvScalar(255, 255, 255, 0));
	cvSaveImage (__filenamebuf, __img_cap, NULL);
	fclose (f);
}

// Clean up the camera module
void rpicam_fini (void)
{
	if (__initialized) {
		cvReleaseImage (&__img_cap);
		cvReleaseCapture (&__capture);
		__initialized = 0;
	}
}

