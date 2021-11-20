#ifndef WINUWP_FILESYSTEM_FACTORY_H
#define WINUWP_FILESYSTEM_FACTORY_H

#include "common/singleton.h"
#include "backends/fs/fs-factory.h"

class WinUWPFilesystemFactory : public FilesystemFactory, public Common::Singleton<WinUWPFilesystemFactory> {
public:
	virtual AbstractFSNode *makeRootFileNode() const;
	virtual AbstractFSNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFSNode *makeFileNodePath(const Common::String &path) const;

private:
	friend class Common::Singleton<SingletonBaseType>;
};

#endif
