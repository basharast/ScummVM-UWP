#ifndef WINUWP_FILESYSTEM_H
#define WINUWP_FILESYSTEM_H

#include "backends/fs/abstract-fs.h"

#define MAX_PATH 260

using namespace Windows::Storage;
using namespace Windows::Foundation::Collections;

class WinUWPFilesystemNode : public AbstractFSNode {

private:
	void setFlags();

	void addStorageItem(AbstractFSList & myList, IStorageItem ^ item) const;
	void addStorageItems(AbstractFSList & myList, IVectorView<IStorageItem^>^ items) const;

	Platform::String ^ getAccessListToken(const Common::String & path) const;
	void getAccessList(AbstractFSList & myList) const;
	void getPathFromFolderPicker(Common::String & path) const;
	IStorageItem ^ getItemFromAccessList(Common::String & path) const;
	IStorageItem ^ getItemFromAccessList(Platform::String ^ token) const;


protected:
	Common::String _displayName;
	Common::String _path;
	WinUWPFilesystemNode* _parent;
	bool _isPseudoRoot;
	bool _isDirectory;
	bool _isValid;
	bool _isReadonly;

public:
	WinUWPFilesystemNode();
	~WinUWPFilesystemNode();
	WinUWPFilesystemNode(const Common::String &path);

	virtual bool exists() const;
	virtual Common::String getDisplayName() const { return _displayName; }
	virtual Common::String getName() const { return _displayName; }
	virtual Common::String getPath() const;
	virtual bool isDirectory() const { return _isDirectory; }
	virtual bool isReadable() const;
	virtual bool isWritable() const;

	virtual AbstractFSNode *getChild(const Common::String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	
	virtual AbstractFSNode *getParent() const;

	virtual Common::SeekableReadStream *createReadStream();
	virtual Common::WriteStream *createWriteStream();

	static void createDir(StorageFolder^ dir, Platform::String ^subDir);
	
		
	/**
	* Converts a Unicode string to Ascii format.
	*
	* @param str Common::String to convert from Unicode to Ascii.
	* @return str in Ascii format.
	*/
	static char *toAscii(const wchar_t *str);

	/**
	* Converts an Ascii string to Unicode format.
	*
	* @param str Common::String to convert from Ascii to Unicode.
	* @return str in Unicode format.
	*/
	static const wchar_t* toUnicode(const char *str);

};

#endif