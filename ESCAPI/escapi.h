/* Extremely Simple Capture API */
#include <Windows.h>

#define MAXDEVICES 16

struct SimpleCapParams
{
	/* Target buffer. 
	 * Must be at least mWidth * mHeight * sizeof(int) of size! 
	 */
	int * mTargetBuf;
	/* Buffer width */
	int mWidth;
	/* Buffer height */
	int mHeight;
};

enum CAPTURE_PROPETIES
{
	CAPTURE_BRIGHTNESS,
	CAPTURE_CONTRAST,
	CAPTURE_HUE,
	CAPTURE_SATURATION,
	CAPTURE_SHARPNESS,
	CAPTURE_GAMMA,
	CAPTURE_COLORENABLE,
	CAPTURE_WHITEBALANCE,
	CAPTURE_BACKLIGHTCOMPENSATION,
	CAPTURE_GAIN,
	CAPTURE_PAN,
	CAPTURE_TILT,
	CAPTURE_ROLL,
	CAPTURE_ZOOM,
	CAPTURE_EXPOSURE,
	CAPTURE_IRIS,
	CAPTURE_FOCUS,
	CAPTURE_PROP_MAX
};

// Options accepted by above:
// Return raw data instead of converted rgb. Using this option assumes you know what you're doing.
#define CAPTURE_OPTION_RAWDATA 1 
// Mask to check for valid options - all options OR:ed together.
#define CAPTURE_OPTIONS_MASK (CAPTURE_OPTION_RAWDATA) 

extern HRESULT InitDevice(int device);
extern void CleanupDevice(int device);
extern int CountCaptureDevices();
extern void GetCaptureDeviceName(int deviceno, char* namebuffer, int bufferlength);
extern void CheckForFail(int device);
extern int GetErrorCode(int device);
extern int GetErrorLine(int device);
extern float GetProperty(int device, int prop);
extern int GetPropertyAuto(int device, int prop);
extern int SetProperty(int device, int prop, float value, int autoval);

extern void getCaptureDeviceName(unsigned int deviceno, char* namebuffer, int bufferlength);
extern int ESCAPIDLLVersion();
extern int ESCAPIVersion();
extern int countCaptureDevices();
extern void initCOM();
extern int initCapture(unsigned int deviceno, struct SimpleCapParams* aParams);
extern void deinitCapture(unsigned int deviceno);
extern void doCapture(unsigned int deviceno);
extern int isCaptureDone(unsigned int deviceno);
extern int getCaptureErrorLine(unsigned int deviceno);
extern int getCaptureErrorCode(unsigned int deviceno);
extern float getCapturePropertyValue(unsigned int deviceno, int prop);
extern int getCapturePropertyAuto(unsigned int deviceno, int prop);
extern int setCaptureProperty(unsigned int deviceno, int prop, float value, int autoval);
extern int initCaptureWithOptions(unsigned int deviceno, struct SimpleCapParams* aParams, unsigned int aOptions);
