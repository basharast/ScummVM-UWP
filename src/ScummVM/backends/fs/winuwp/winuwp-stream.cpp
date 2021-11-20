#if !defined(DISABLE_STDIO_FILESTREAM)

// Disable symbol overrides so that we can use FILE, fopen etc.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "winuwp-stream.h"
#include <Windows.h>
#include <ppltasks.h>

using namespace concurrency;
using namespace Windows::UI::Core;
using namespace Windows::Storage::Streams;


WinUWPStream::WinUWPStream(Streams::IRandomAccessStream^ stream)
{
	_stream = stream;
	clearErr();
}

WinUWPStream::~WinUWPStream()
{
	delete _stream;
}

bool WinUWPStream::err() const
{
	return _error;
}

void WinUWPStream::clearErr()
{
	_error = false;
	_eos = false;
}

bool WinUWPStream::eos() const
{
	return _eos;
}

bool WinUWPStream::eof()
{
	return _stream->Position >= _stream->Size;
}

uint32 WinUWPStream::write(const void * dataPtr, uint32 dataSize)
{
	//We are not writing outside of the app's sandbox
	return 0;
}

bool WinUWPStream::flush()
{
	volatile bool done = false;
	bool result = false;
	try {
		auto t = create_task(_stream->FlushAsync());

		t.then([&done, &result](bool b) {
			result = b;
			});

		auto window = CoreWindow::GetForCurrentThread();
		while (false == done) {
			if (window != nullptr && window->Dispatcher->HasThreadAccess)
				window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneIfPresent);
		}
	}
	catch (...) {}
	return result;
}

int32 WinUWPStream::pos() const
{
	return (int32)_stream->Position;
}

int32 WinUWPStream::size() const
{
	return (int32)_stream->Size;
}

bool WinUWPStream::seek(int32 offs, int whence)
{
	//not sure if there is a proper way to check 
	//if Seek() was successful or not
	try {
		switch (whence)
		{
		case SEEK_END:
			_stream->Seek(_stream->Size);
			break;
		case SEEK_SET:
			_stream->Seek(0);
			break;
		}
		_stream->Seek(_stream->Position + offs);
		_eos = false;
	}
	catch (...) {
		return false;
	}
	return true;
}

uint32 WinUWPStream::read(void * ptr, uint32 len)
{
	volatile bool done = false;
	uint32 toReturn = 0;
	try {
		if (!eof()) {
			uint32 toRead = (_stream->Size - _stream->Position);
			if (len < toRead)
				toRead = len;

			auto t = create_task(_stream->ReadAsync(ref new Streams::Buffer(toRead), toRead, Streams::InputStreamOptions::Partial));

			t.then([this, &done, &toReturn, ptr](Streams::IBuffer^ buffer) {
				try {
					Streams::DataReader::FromBuffer(buffer)->ReadBytes(Platform::ArrayReference<byte>((byte*)ptr, buffer->Length));
					toReturn = buffer->Length;
					done = true;
				}
				catch (...) {
					toReturn = 0;
					_error = true;
					done = true;
				}
				});
		}
		else {
			_eos = true;
			done = true;
		}
		auto window = CoreWindow::GetForCurrentThread();
		while (false == done) {
			if (window != nullptr && window->Dispatcher->HasThreadAccess)
				window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneIfPresent);
		}
	}
	catch (...) {}
	return toReturn;
}


WinUWPStream * WinUWPStream::makeFromPath(const Common::String & path, bool writeMode)
{
	WinUWPStream* toReturn = nullptr;
	volatile bool done = false;
	try {
		static wchar_t unicodeString[MAX_PATH];
		const char* str = path.c_str();
		MultiByteToWideChar(CP_ACP, 0, str, strlen(str) + 1, unicodeString, sizeof(unicodeString) / sizeof(wchar_t));

		Platform::String^ myPath = ref new Platform::String(unicodeString);

		auto t = create_task(Windows::Storage::StorageFile::GetFileFromPathAsync(myPath));

		t.then([&writeMode](Windows::Storage::StorageFile^ file) -> Windows::Foundation::IAsyncOperation <Streams::IRandomAccessStream^>^ {
			return file->OpenAsync(FileAccessMode::Read);
			}).then([&toReturn, &done](Streams::IRandomAccessStream^ stream) {
				toReturn = new WinUWPStream(stream);
				done = true;
				});

			auto window = CoreWindow::GetForCurrentThread();
			while (false == done) {
				if (window != nullptr && window->Dispatcher->HasThreadAccess)
					window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneIfPresent);
			}

			toReturn->_path = path;
	}
	catch (...) {}
	return toReturn;
}

#endif