#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mfobjects.h>

#include <cassert>
#include <string>
#include <vector>
#include <iostream>

#pragma comment (lib, "ole32.lib")
#pragma comment (lib, "mf.lib")
#pragma comment (lib, "mfplat.lib")
#pragma comment (lib, "mfuuid.lib")
#pragma comment (lib, "mfreadwrite.lib")

#define check(x) assert(SUCCEEDED(x))

namespace tools {

	struct Device {
		size_t index;
		std::wstring name, symlink;
	};

	class WebCam {
	public:
		WebCam() = default;

		inline WebCam(const Device& device) {
			IMFMediaSource* imfDevice;

			HRESULT hr;
			{
				IMFAttributes* attr;

				hr = MFCreateAttributes(&attr, 1);
				check(hr);

				hr = attr->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
				check(hr);

				hr = attr->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, device.symlink.c_str());
				check(hr);

				hr = MFCreateDeviceSource(attr, &imfDevice);
				check(hr);

				attr->Release();
			}

			{
				IMFAttributes* attr;

				hr = MFCreateAttributes(&attr, 1);
				check(hr);

				hr = MFCreateSourceReaderFromMediaSource(imfDevice, attr, &m_reader);
				check(hr);
			}

			imfDevice->Release();

			// TODO: Media Type Enumeration
			{
				IMFMediaType* type;

				hr = MFCreateMediaType(&type);
				check(hr);

				hr = type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
				check(hr);

				hr = type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB24);
				check(hr);

				hr = m_reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, type);
				check(hr);

				type->Release();
			}

			UINT32 width;
			UINT32 height;

			// get width/height
			{
				IMFMediaType* type;

				hr = m_reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &type);
				check(hr);

				UINT64 tmp;
				hr = type->GetUINT64(MF_MT_FRAME_SIZE, &tmp);
				check(hr);

				width = (UINT32)(tmp >> 32);
				height = (UINT32)(tmp);

				type->Release();
			}

			std::cout << "Size = " << width << "x" << height << "\n";
		}

		static std::vector<Device> enumerate() {
			initCOM();

			HRESULT hr;

			std::vector<Device> retDevices;

			UINT count;
			IMFActivate** devices;

			{
				IMFAttributes* attr;

				hr = MFCreateAttributes(&attr, 1);
				check(hr);

				hr = attr->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
				check(hr);

				hr = MFEnumDeviceSources(attr, &devices, &count);
				check(hr);

				attr->Release();
			}

			std::cout << "Devices found: " << count << "\n";
			for (UINT i = 0; i < count; i++) {
				auto act = devices[i];

				Device dev{};
				dev.index = i;

				LPWSTR name, link;
				UINT32 size;
				hr = act->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &link, &size);
				check(hr);

				dev.symlink = std::wstring(link, size);

				hr = act->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &name, &size);
				check(hr);

				dev.name = std::wstring(name, size);

				std::wcout << i << "): " << dev.name << "\n";

				CoTaskMemFree(name);
				CoTaskMemFree(link);

				act->Release();

				retDevices.push_back(dev);
			}

			CoTaskMemFree(devices);

			return retDevices;
		}

	private:
		std::vector<Device> m_devices;

		IMFSourceReader* m_reader;

		static void initCOM() {
			HRESULT hr;
			hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
			check(hr);

			hr = MFStartup(MF_VERSION);
			check(hr);
		}

		void readSample() {
			IMFSample* sample;

			DWORD stream;
			DWORD flags;
			LONGLONG timestamp;
			HRESULT hr;

			for (;;) {
				// this is reading in syncronous blocking mode, MF supports also async calls
				hr = m_reader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &stream, &flags, &timestamp, &sample);
				
				check(hr);

				if (flags & MF_SOURCE_READERF_STREAMTICK) {
					continue;
				}

				break;
			}
		}

	};

}
