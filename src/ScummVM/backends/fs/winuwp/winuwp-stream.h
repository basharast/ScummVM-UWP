#include "common/noncopyable.h"
#include "common/stream.h"

using namespace Windows::Storage;

class WinUWPStream : public Common::SeekableReadStream, public Common::WriteStream, public Common::NonCopyable {
protected:
	Streams::IBuffer^ _buffer;
	Streams::IRandomAccessStream^ _stream;
	bool _error;
	bool _eos;

public:

	static WinUWPStream *makeFromPath(const Common::String &path, bool writeMode);
	
	Common::String _path;

	WinUWPStream::WinUWPStream(Streams::IRandomAccessStream^ stream);
	virtual ~WinUWPStream();

	virtual bool err() const;
	virtual void clearErr();
	virtual bool eos() const;
	virtual bool eof();

	virtual uint32 write(const void *dataPtr, uint32 dataSize);
	virtual bool flush();

	virtual int32 pos() const;
	virtual int32 size() const;
	virtual bool seek(int32 offs, int whence = SEEK_SET);
	virtual uint32 read(void *ptr, uint32 len);

};