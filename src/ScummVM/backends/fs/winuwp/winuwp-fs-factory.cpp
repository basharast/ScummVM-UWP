#include "backends/fs/winuwp/winuwp-fs-factory.h"
#include "backends/fs/winuwp/winuwp-fs.h"

namespace Common {
	DECLARE_SINGLETON(WinUWPFilesystemFactory);
}

AbstractFSNode *WinUWPFilesystemFactory::makeRootFileNode() const {
	return new WinUWPFilesystemNode();
}

AbstractFSNode *WinUWPFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new WinUWPFilesystemNode();
}

AbstractFSNode *WinUWPFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	return new WinUWPFilesystemNode(path);
}