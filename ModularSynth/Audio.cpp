#include "Audio.h"

#include <cstdio>

static void dataCallback(ma_device* dev, void* output, const void* input, ma_uint32 frames) {
	ma_waveform* sineWave;
	sineWave = (ma_waveform*)dev->pUserData;
	ma_waveform_read_pcm_frames(sineWave, output, frames, nullptr);
}

void demo() {
	ma_waveform sineWave;
	ma_device_config deviceConfig;
	ma_device device;
	ma_waveform_config sineWaveConfig;

	sineWaveConfig = ma_waveform_config_init(ma_format_f32, 2, 44100, ma_waveform_type_sine, 0.3, 220.0);
	ma_waveform_init(&sineWaveConfig, &sineWave);

	deviceConfig = ma_device_config_init(ma_device_type_playback);
	deviceConfig.playback.format = ma_format_f32;
	deviceConfig.playback.channels = 2;
	deviceConfig.sampleRate = 44100;
	deviceConfig.dataCallback = dataCallback;
	deviceConfig.pUserData = &sineWave;

	if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
		printf("Failed to open playback device.\n");
		return;
	}

	printf("Device Name: %s\n", device.playback.name);

	if (ma_device_start(&device) != MA_SUCCESS) {
		printf("Failed to start playback device.\n");
		ma_device_uninit(&device);
		return;
	}

	printf("Press Enter to quit...\n");
	getchar();

	ma_device_uninit(&device);
}
