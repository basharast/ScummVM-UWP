#include <Windows.h>
#include <ppltasks.h>

#include "winuwp-stream.h"

#include "backends/fs/winuwp/winuwp-fs.h"
#include "backends/fs/stdiostream.h"
#include "common/memstream.h"
#include "common/bufferedstream.h"

using namespace concurrency;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::Storage;


/* Dear Microsoft
	  * I really appreciate all the effort you took to not provide any
	  * synchronous file APIs and block all attempts to synchronously
	  * wait for the results of async tasks for "smooth user experience",
	  * but I'm not going to run and rewrite all RetroArch cores for
	  * async I/O. I hope you like this hack I made instead.
	  */
template<typename T>
T RunAsync(std::function<concurrency::task<T>()> func)
{
	volatile bool finished = false;
	Platform::Exception^ exception = nullptr;
	T result;

	func().then([&finished, &exception, &result](concurrency::task<T> t) {
		try
		{
			result = t.get();
		}
		catch (Platform::Exception^ exception_)
		{
			exception = exception_;
		}
		finished = true;
		});

	/* Don't stall the UI thread - prevents a deadlock */
	//Windows::UI::Core::CoreWindow^ corewindow = Windows::UI::Core::CoreWindow::GetForCurrentThread();
	/*
	while (!finished)
	{
		if (corewindow) {
			corewindow->Dispatcher->ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent);
		}
	}
	*/

	//if (exception != nullptr)
	//	throw exception;
	return result;
}

template<typename T>
T RunAsyncAndCatchErrors(std::function<concurrency::task<T>()> func, T valueOnError)
{
	try
	{
		return RunAsync<T>(func);
	}
	catch (Platform::Exception^ e)
	{
		return valueOnError;
	}
}

WinUWPFilesystemNode::WinUWPFilesystemNode() {
	_path = "";
	_isValid = false;
	_isReadonly = true;
	_isDirectory = true;
	_isPseudoRoot = true;
	_parent = 0;
}

WinUWPFilesystemNode::~WinUWPFilesystemNode()
{
	_parent = 0;
}

WinUWPFilesystemNode::WinUWPFilesystemNode(const Common::String& path) {
	try {
		_parent = 0;
		_path = path;
		setFlags();
	}
	catch (...) {

	}
}
char* WinUWPFilesystemNode::toAscii(const wchar_t* str) {
	static char asciiString[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, str, wcslen(str) + 1, asciiString, sizeof(asciiString), NULL, NULL);
	return asciiString;
}

const wchar_t* WinUWPFilesystemNode::toUnicode(const char* str) {
	static wchar_t unicodeString[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, str, strlen(str) + 1, unicodeString, sizeof(unicodeString) / sizeof(wchar_t));
	return unicodeString;
}

void WinUWPFilesystemNode::createDir(StorageFolder^ dir, Platform::String^ subDir) {
	auto t = create_task(dir->TryGetItemAsync(subDir));
	t.then([dir, subDir](IStorageItem^ result) {
		if (result == nullptr) {
			create_task(dir->CreateFolderAsync(subDir)).then([](task<StorageFolder^> t) {
				try { t.get(); }
				catch (...) {}
				});
		}
		});
}

Platform::String^ WinUWPFilesystemNode::getAccessListToken(const Common::String& path) const {

	Common::String s = path;

	if (s.lastChar() == '\\')
		s.deleteLastChar();

	std::string str = std::string(s.c_str());
	std::replace(str.begin(), str.end(), '\\', '_');
	std::replace(str.begin(), str.end(), ':', '_');
	return ref new Platform::String(toUnicode(str.c_str()));
}

IStorageItem^ WinUWPFilesystemNode::getItemFromAccessList(Platform::String^ token) const {
	IStorageItem^ toReturn = nullptr;
	volatile bool done = false;
	if (token != nullptr && token->Length() > 0 && AccessCache::StorageApplicationPermissions::FutureAccessList->ContainsItem(token)) {
		try
		{
			//toReturn = AccessCache::StorageApplicationPermissions::FutureAccessList->GetItemAsync(token)->GetResults();
			auto t = create_task(AccessCache::StorageApplicationPermissions::FutureAccessList->GetItemAsync(token));
			t.then([&done, &toReturn](IStorageItem^ item) {
				return item;
				}).then([&done, &toReturn, token](task<IStorageItem^> t)
					{
						try
						{
							toReturn = t.get();
							done = true;
							// .get() didn't throw, so we succeeded.
						}
						catch (Platform::COMException^ e)
						{
							//Example output: The system cannot find the specified file.
							AccessCache::StorageApplicationPermissions::FutureAccessList->Remove(token);
							done = true;
						}
					});
		}
		catch (Platform::COMException^ e) {
			done = true;
		}
	}
	else {
		done = true;
	}

	auto window = CoreWindow::GetForCurrentThread();

	while (!done) {
		if (window != nullptr && window->Dispatcher->HasThreadAccess)
			window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneIfPresent);
	}
	return toReturn;
}

IStorageItem^ WinUWPFilesystemNode::getItemFromAccessList(Common::String& path) const {
	return getItemFromAccessList(getAccessListToken(path));
}

void WinUWPFilesystemNode::addStorageItem(AbstractFSList& myList, IStorageItem^ item) const {
	try {
		WinUWPFilesystemNode node;
		node._path = toAscii(item->Path->Data());
		node._displayName = toAscii(item->Name->Data());
		node._isValid = true;
		node._isDirectory = (item->Attributes & FileAttributes::Directory) == FileAttributes::Directory;
		node._isReadonly = (item->Attributes & FileAttributes::ReadOnly) == FileAttributes::ReadOnly;
		node._isPseudoRoot = false;
		node._parent = new WinUWPFilesystemNode(*this);

		myList.push_back(new WinUWPFilesystemNode(node));
	}
	catch (...) {}
}

void WinUWPFilesystemNode::addStorageItems(AbstractFSList& myList, IVectorView<IStorageItem^>^ items) const {
	for (auto it = items->First(); it->HasCurrent; it->MoveNext()) {
		try {
			addStorageItem(myList, it->Current);
		}
		catch (...) {}
	}
}

void WinUWPFilesystemNode::getAccessList(AbstractFSList& myList) const {
	try {
		auto entries = AccessCache::StorageApplicationPermissions::FutureAccessList->Entries;
		for (auto it = entries->First(); it->HasCurrent; it->MoveNext()) {
			try {
				auto item = getItemFromAccessList(it->Current.Token);
				if (item != nullptr) {
					addStorageItem(myList, item);
				}
			}
			catch (...) {}
		}
		addStorageItem(myList, ApplicationData::Current->LocalFolder);
	}
	catch (...) {}
}

void WinUWPFilesystemNode::getPathFromFolderPicker(Common::String& path) const {
	try {
		volatile bool done = false;
		try {
			Pickers::FolderPicker^ picker = ref new Pickers::FolderPicker();

			// Oddly enough, even a Folderpicker needs at least one FileTypeFilter.
			picker->FileTypeFilter->Append(".scummvm");
			picker->ViewMode = Pickers::PickerViewMode::Thumbnail;
			picker->SuggestedStartLocation = Pickers::PickerLocationId::Downloads;

			auto t = create_task(picker->PickSingleFolderAsync());
			t.then([this, &path, &done](StorageFolder^ folder) {
				if (folder == nullptr) {
					done = true;
					cancel_current_task();
					path = "";
				}
				path = Common::String(toAscii(folder->Path->Data()));
				AccessCache::StorageApplicationPermissions::FutureAccessList->AddOrReplace(getAccessListToken(path), folder);
				done = true;
				});
		}
		catch (...) {}
		auto window = CoreWindow::GetForCurrentThread();
		while (!done) {
			if (window != nullptr && window->Dispatcher->HasThreadAccess)
				window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneIfPresent);
		}
	}
	catch (...) {}
}

void WinUWPFilesystemNode::setFlags() {

	_isValid = false;
	_isDirectory = false;
	_isReadonly = false;
	_isPseudoRoot = false;

	if (_path.empty())
		return;

	volatile bool done = false;
	const char* start = _path.c_str();

	//We are first trying to get the item from FutureAccessList
	IStorageItem^ item = getItemFromAccessList(_path);

	//if that fails, we are trying to fetch the item directly
	if (item == nullptr) {
		if (_path.lastChar() == '\\') {
			auto t = create_task(StorageFolder::GetFolderFromPathAsync(ref new Platform::String(toUnicode(start))));
			t.then([this, &item, &done](task<StorageFolder^> t1) {
				try
				{
					item = t1.get();
					done = true;
				}
				catch (...)
				{
					_path = "";
					_isValid = false;
					_isReadonly = true;
					_isDirectory = true;
					_isPseudoRoot = true;
					done = true;
				}
				});
		}
		else {
			auto t = create_task(StorageFile::GetFileFromPathAsync(ref new Platform::String(toUnicode(start))));
			t.then([&item, &done](task<StorageFile^> t1) {
				try
				{
					item = t1.get();
					done = true;
				}
				catch (...)
				{
					done = true;
				}
				});
		}

		auto window = CoreWindow::GetForCurrentThread();
		while (!done) {
			if (window != nullptr && window->Dispatcher->HasThreadAccess)
				window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneIfPresent);
		}
	}

	if (item != nullptr) {
		_isValid = true;
		_isDirectory = (item->Attributes & FileAttributes::Directory) == FileAttributes::Directory;
		_isReadonly = (item->Attributes & FileAttributes::ReadOnly) == FileAttributes::ReadOnly;
		_displayName = toAscii(item->Name->Data());
		_isPseudoRoot = false;
		return;
	}
}

Common::String WinUWPFilesystemNode::getPath() const
{
	if (_isDirectory && !_path.empty() && _path.lastChar() != '\\')
		return _path + '\\';
	return _path;
}

bool WinUWPFilesystemNode::isReadable() const {
	return _isValid;
}

bool WinUWPFilesystemNode::isWritable() const {
	return !_isReadonly;
}

bool WinUWPFilesystemNode::exists() const {
	return _isValid;
}

AbstractFSNode* WinUWPFilesystemNode::getChild(const Common::String& n) const {
	assert(_isDirectory);

	// Make sure the string contains no slashes
	assert(!n.contains('/'));

	Common::String newPath(_path);
	if (_path.lastChar() != '\\')
		newPath += '\\';
	newPath += n;

	return new WinUWPFilesystemNode(newPath);
}

bool WinUWPFilesystemNode::getChildren(AbstractFSList& myList, ListMode mode, bool hidden) const {

	volatile bool done = false;
	try {
		assert(_isDirectory);

		Platform::String^ path = ref new Platform::String(toUnicode(_path.c_str()));

		if (_isPseudoRoot)
			getAccessList(myList);
		else
		{
			auto t = create_task(StorageFolder::GetFolderFromPathAsync(path));
			t.then([](StorageFolder^ folder) -> task < IVectorView<IStorageItem^>^ > {
				return create_task(folder->GetItemsAsync());
				}).then([this, &myList, &done](task<IVectorView<IStorageItem^>^> t) {
					try {
						addStorageItems(myList, t.get());
						done = true;
					}
					catch (...) {
						done = true;
					}
					});

				auto window = CoreWindow::GetForCurrentThread();
				while (!done) {
					if (window != nullptr && window->Dispatcher->HasThreadAccess)
						window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneIfPresent);
				}
		}
	}
	catch (...) {}
	return true;
}

AbstractFSNode* WinUWPFilesystemNode::getParent() const {
	try {
		if (_parent) {
			return new WinUWPFilesystemNode(*_parent);
		}

		if (_isPseudoRoot) {
			Common::String path;
			getPathFromFolderPicker(path);
			if (!path.empty()) {
				return new WinUWPFilesystemNode(path);
			}
		}
		else if (!_path.empty()) {
			const char* start = _path.c_str();
			const char* end = lastPathComponent(_path, '\\');
			Common::String path = Common::String(start, end - start);
			return new WinUWPFilesystemNode(path);
		}
	}
	catch (...) {}
	return new WinUWPFilesystemNode();
}

Common::SeekableReadStream* WinUWPFilesystemNode::createReadStream() {
	Common::SeekableReadStream* stream = nullptr;
	try {
		stream = StdioStream::makeFromPath(getPath(), false);
	}
	catch (...) {}
	//if the stream isn't correctly initialized we are outside the app's sandbox, 
	// thus we have to use the FileStorage API
	if (stream == nullptr)
		// wrapping the stream into a BufferedSeekableReadStream (thanks @wjp for the tip!)
		stream = Common::wrapBufferedSeekableReadStream(WinUWPStream::makeFromPath(getPath(), false), 1500, DisposeAfterUse::YES);
	return stream;
}

Common::WriteStream* WinUWPFilesystemNode::createWriteStream() {
	return StdioStream::makeFromPath(getPath(), true);
}
